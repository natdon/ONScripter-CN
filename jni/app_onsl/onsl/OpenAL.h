/*
* Copyright (c) 2008-2011, Helios (helios.vmg@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the author may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*     * Products derived from this software may not be called "ONSlaught" nor
*       may "ONSlaught" appear in their names without specific prior written
*       permission from the author.
*
* THIS SOFTWARE IS PROVIDED BY HELIOS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL HELIOS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NONS_OPENAL_H
#define NONS_OPENAL_H
#include "Common.h"
#include "Archive.h"
#include <vector>
#include <list>
#include <cstdlib>
#include <cassert>
#include <AL/al.h>
#include <AL/alc.h>

class audio_sink{
	ALuint source;
	static const size_t n=16;
public:
	double time_offset;
	audio_sink();
	~audio_sink();
	operator bool(){ return !!this->source; }
	void push(const void *buffer,size_t length,ulong freq,ulong channels,ulong bit_depth);
	bool needs_more_data();
	void pause(){ alSourcePause(this->source); }
	void unpause(){ alSourcePlay(this->source); }
	void set_volume(float vol){ alSourcef(this->source,AL_GAIN,vol); }
	ALint get_state(){
		ALint state;
		alGetSourcei(this->source,AL_SOURCE_STATE,&state);
		return state;
	}
};

struct audio_buffer{
	void *buffer;
	size_t length;
	ulong frequency;
	ulong channels;
	ulong bit_depth;
	audio_buffer(const void *buffer,size_t length,ulong freq,ulong channels,ulong bit_depth,bool take_ownership=0);
	~audio_buffer(){
		free(this->buffer);
	}
	void push(audio_sink &sink){
		sink.push(this->buffer,this->length*this->channels,this->frequency,this->channels,this->bit_depth);
	}
	static void *allocate(ulong samples,ulong bytes_per_channel,ulong channels){
		return malloc(samples*bytes_per_channel*channels);
	}
	static void deallocate(void *buffer){
		free(buffer);
	}
};

class decoder{
protected:
	NONS_DataStream *stream;
	bool good;
public:
	decoder(NONS_DataStream *stream);
	virtual ~decoder();
	virtual operator bool(){ return this->good; }
	virtual audio_buffer *get_buffer(bool &error)=0;
	virtual void loop()=0;
};

class audio_stream;

class audio_device{
	friend class NONS_AudioDeviceManager;
public:
	typedef std::list<audio_stream *> list_t;
private:
	ALCdevice *device;
	ALCcontext *context;
	list_t streams;
	bool good;
public:
	audio_device();
	~audio_device();
	operator bool(){ return this->good; }
	void update(std::vector<audio_stream *> &removed_streams);
	void add(audio_stream *);
	void remove(audio_stream *);
};

class audio_stream{
protected:
	audio_sink *sink;
	decoder *dec;
	bool good,
		playing,
		paused;
	float volume,
		*general_volume;
	bool muted;
	float get_compound_volume(bool including_mute=1){
		if (including_mute && this->muted)
			return 0;
		if (!this->general_volume)
			return this->volume;
		return this->volume* *this->general_volume;
	}
	void set_internal_volume(){
		CHECK_POINTER_AND_CALL(this->sink,set_volume(this->get_compound_volume()));
	}
	std::vector<NONS_Event *> notify;
public:
	std::wstring filename;
	int loop;
	bool cleanup;
	audio_device::list_t::iterator iterator;
	audio_stream(const std::wstring &filename,bool prioritize_filesystem=0);
	virtual ~audio_stream();
	operator bool(){ return this->good; }
	void start();
	void stop();
	void pause(int mode=-1);
	virtual bool update();
	bool is_playing() const{ return this->playing; }
	bool is_sink_playing() const;
	bool is_paused() const{ return this->paused; }
	void set_volume(float);
	void set_general_volume(float &);
	void mute(int mode=-1);
	float get_volume() const{ return this->volume; }
	void notify_on_stop(NONS_Event *event){ this->notify.push_back(event); }
	int needs_update();
};

class asynchronous_audio_stream:public audio_stream{
	NONS_Mutex mutex;
public:
	asynchronous_audio_stream();
	~asynchronous_audio_stream(){}
	bool update();
	bool asynchronous_buffer_push(audio_buffer *buffer);
	double get_time_offset();
};
#endif
