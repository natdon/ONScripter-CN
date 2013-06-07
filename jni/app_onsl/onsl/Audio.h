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

#ifndef NONS_AUDIO_H
#define NONS_AUDIO_H

#include "SDLhead.h"
#include "Common.h"
#include "ErrorCodes.h"
#include "ThreadManager.h"
#include "IOFunctions.h"
#include "OpenAL.h"
#include <map>
#include <list>
#include <string>

struct channel_listing{
	struct channel{
		std::wstring filename;
		long loop;
		int volume;
	};
	channel music;
	std::map<int,channel> sounds;
};

struct interpreter_stored_state;

class NONS_Audio{
	static const int initial_channel_counter;

	audio_device *dev;
public:
	typedef std::map<int,audio_stream *> chan_t;
private:
	chan_t channels;
	bool uninitialized,
		notmute;
	int channel_counter;
	float mvol,
		svol;

	NONS_Thread thread;
	void update_thread();
	NONS_Mutex mutex;
	bool stop_thread;

	audio_stream *get_channel(int channel);
	int set_volume(float &,int);
	TiXmlElement *save_channels();
	void load_channel(TiXmlElement *);
	void load_channels(TiXmlElement *);
public:
	static const long max_valid_channel;
	static const int music_channel;
	std::wstring music_dir,
		music_format;
	NONS_Audio(const std::wstring &musicDir);
	~NONS_Audio();
	ErrorCode play_music(const std::wstring &filename,long times=-1);
	ErrorCode stop_music();
	ErrorCode pause_music();
	ErrorCode resume_music();
	ErrorCode play_sound_once(const std::wstring &filename){
		return this->play_sound(filename,-1,0,1);
	}
	ErrorCode play_sound(const std::wstring &filename,int channel,long times,bool automatic_cleanup);
	ErrorCode stop_sound(int channel);
	ErrorCode stop_all_sound();
	ErrorCode load_sound_on_a_channel(const std::wstring &filename,int &channel,bool use_channel_as_input=0);
	ErrorCode unload_sound_from_channel(int channel);
	ErrorCode play(int channel,long times,bool automatic_cleanup);
	void wait_for_channel(int channel);
	int music_volume(int vol){
		return this->set_volume(this->mvol,vol);
	}
	int sound_volume(int vol){
		return this->set_volume(this->svol,vol);
	}
	int channel_volume(int channel,int vol);
	bool toggle_mute();
	bool is_playing(int channel);
	bool is_initialized(){
		return !this->uninitialized;
	}
	void get_channel_listing(channel_listing &cl);
	asynchronous_audio_stream *new_video_stream();
	void delete_video_stream(asynchronous_audio_stream *stream){
		NONS_MutexLocker ml(this->mutex);
		if (stream)
			this->dev->remove(stream);
	}
	TiXmlElement *save();
	void load(TiXmlElement *);
};

class NONS_ScopedAudioStream{
	int channel;
	NONS_Audio *audio;
	bool good;
public:
	NONS_ScopedAudioStream():audio(0),good(0){}
	NONS_ScopedAudioStream(NONS_Audio *audio,const std::wstring &filename){
		this->init(audio,filename);
	}
	~NONS_ScopedAudioStream(){
		if (this->good)
			this->audio->unload_sound_from_channel(this->channel);
	}
	void init(NONS_Audio *audio,const std::wstring &filename){
		this->audio=audio;
		this->good=this->audio && this->audio->load_sound_on_a_channel(filename,this->channel)==NONS_NO_ERROR;
	}
	void play(bool loop){
		if (this->good)
			this->audio->play(this->channel,loop?-1:0,0);
	}
	void stop(){
		if (this->good)
			this->audio->stop_sound(this->channel);
	}
};
#endif
