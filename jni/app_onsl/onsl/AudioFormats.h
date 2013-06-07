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

#ifndef NONS_AUDIOFORMATS_H
#define NONS_AUDIOFORMATS_H
#include "OpenAL.h"
#include "IOFunctions.h"
#include "ThreadManager.h"
#include <vorbis/vorbisfile.h>
#include <FLAC++/decoder.h>
#include <mpg123.h>
#include <mikmod.h>
#include "libtimidity/timidity.h"

class ogg_decoder:public decoder{
	OggVorbis_File file;
	int bitstream;
public:
	ogg_decoder(NONS_DataStream *stream);
	~ogg_decoder();
	audio_buffer *get_buffer(bool &error);
	void loop();
};

class flac_decoder:public decoder,public FLAC::Decoder::Stream{
	audio_buffer *buffer;
	FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame,const FLAC__int32 * const *buffer);
	FLAC__StreamDecoderReadStatus read_callback(FLAC__byte *buffer,size_t *bytes);
	FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset);
	bool eof_callback();
	FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64 *absolute_byte_offset);
	FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length);
	void error_callback(::FLAC__StreamDecoderErrorStatus status){
		o_stderr <<"Got error callback: "<<FLAC__StreamDecoderErrorStatusString[status]<<"\n";
	}
public:
	flac_decoder(NONS_DataStream *stream);
	~flac_decoder();
	operator bool(){
		return decoder::operator bool() && FLAC::Decoder::Stream::operator bool();
	}
	audio_buffer *get_buffer(bool &error);
	void loop();
};

class static_initializer{
protected:
	bool initialized;
	NONS_Mutex mutex;
	virtual bool perform_initialization()=0;
public:
	static_initializer():initialized(0){}
	virtual ~static_initializer(){}
	bool init();
};

class mp3_static_data:public static_initializer{
	bool perform_initialization();
public:
	~mp3_static_data();
};

class mp3_decoder:public decoder{
	static mp3_static_data static_data;
	mpg123_handle *handle;
	bool has_played;
public:
	mp3_decoder(NONS_DataStream *stream);
	~mp3_decoder();
	audio_buffer *get_buffer(bool &error);
	void loop();
};

class mod_static_data:public static_initializer{
	bool perform_initialization();
public:
	~mod_static_data();
};

class mod_decoder:public decoder{
	static mod_static_data static_data;
	MODULE *module;
public:
	mod_decoder(NONS_DataStream *stream);
	~mod_decoder();
	audio_buffer *get_buffer(bool &error);
	void loop();
};

class midi_static_data:public static_initializer{
	bool perform_initialization();
public:
	typedef std::map<std::wstring,NONS_DataStream *> map_t;

	~midi_static_data();
	void cache_file(const char *path,NONS_DataStream *stream);
	NONS_DataStream *get_file(const char *path);
private:
	map_t cached_files;
};

class midi_decoder:public decoder{
	MidSong *song;
public:
	static midi_static_data static_data;
	midi_decoder(NONS_DataStream *stream);
	~midi_decoder();
	audio_buffer *get_buffer(bool &error);
	void loop();
};
#endif
