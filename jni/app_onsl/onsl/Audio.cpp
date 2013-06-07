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

#include "Audio.h"
#include "IOFunctions.h"
#include "Options.h"
#include "Archive.h"
#include "ScriptInterpreter.h"
#include <iostream>

#define NONS_Audio_FOREACH() for (chan_t::iterator i=this->channels.begin(),e=this->channels.end();i!=e;++i)

const int NONS_Audio::initial_channel_counter=1<<20;
const long NONS_Audio::max_valid_channel=(long)initial_channel_counter-1;
const int NONS_Audio::music_channel=-1;

NONS_Audio::NONS_Audio(const std::wstring &musicDir){
	this->notmute=0;
	this->uninitialized=1;
	if (CLOptions.no_sound){
		this->dev=0;
		return;
	}
	this->dev=new audio_device;
	if (!*this->dev){
		delete this->dev;
		this->dev=0;
		return;
	}
	this->uninitialized=0;
	this->notmute=(!settings.mute.set)?1:!settings.mute.data;
	if (!musicDir.size())
		this->music_dir=L"./CD";
	else
		this->music_dir=musicDir;
	this->music_format=CLOptions.musicFormat;
	this->channel_counter=initial_channel_counter;
	this->mvol=this->svol=1.f;
	this->stop_thread=0;
	this->thread.call(member_bind(&NONS_Audio::update_thread,this),1);
}

NONS_Audio::~NONS_Audio(){
	if (this->uninitialized)
		return;
	this->stop_thread=1;
	this->thread.join();
	delete this->dev;
	settings.mute.data=!this->notmute;
	settings.mute.set=1;
}

void NONS_Audio::update_thread(){
	while (!this->stop_thread){
		{
			NONS_MutexLocker ml(this->mutex);
			std::vector<audio_stream *> removed_streams;
			this->dev->update(removed_streams);
			for (size_t a=0;a<removed_streams.size();a++){
				NONS_Audio_FOREACH(){
					if (i->second==removed_streams[a]){
						this->channels.erase(i);
						break;
					}
				}
			}
		}
		SDL_Delay(10);
	}
}

audio_stream *NONS_Audio::get_channel(int channel){
	NONS_MutexLocker ml(this->mutex);
	chan_t::iterator i=this->channels.find(channel);
	return (i==this->channels.end())?0:i->second;
}

std::wstring wav2flac(const std::wstring &str){
	std::wstring r=str;
	if (ends_with(tolowerCopy(r),(std::wstring)L".wav")){
		size_t n=r.size()-3;
		r.resize(n+4);
		static const wchar_t *flac=L"flac";
		std::copy(flac,flac+4,&r[n]);
	}
	return r;
}

extern const wchar_t *sound_formats[];

ErrorCode NONS_Audio::play_music(const std::wstring &filename,long times){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	const int channel=NONS_Audio::music_channel;
	std::wstring found_path,
		converted_filename=wav2flac(filename);
	bool found=0;
	if (!this->music_format.size()){
		for (ulong a=0;!found && sound_formats[a];a++){
			found_path=this->music_dir+L"/"+converted_filename+L"."+sound_formats[a];
			if (filesystem.exists(found_path))
				found=1;
		}
		for (ulong a=0;!found && sound_formats[a];a++){
			found_path=this->music_dir+L"/"+converted_filename+L"."+sound_formats[a];
			if (general_archive.exists(found_path))
				found=1;
		}
	}else
		found_path=this->music_dir+L"/"+converted_filename+L"."+this->music_format;
	if (!found){
		found_path=converted_filename;
		if (!filesystem.exists(found_path) && !general_archive.exists(found_path=converted_filename))
			return NONS_FILE_NOT_FOUND;
	}

	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(channel);
	if (stream)
		this->dev->remove(stream);
	stream=new audio_stream(found_path,1);
	if (!*stream){
		delete stream;
		return NONS_UNDEFINED_ERROR;
	}
	stream->loop=times;
	stream->cleanup=0;
	stream->set_general_volume(this->mvol);
	stream->mute(!this->notmute);
	stream->start();
	this->dev->add(stream);
	this->channels[NONS_Audio::music_channel]=stream;
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::stop_music(){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(NONS_Audio::music_channel);
	if (!stream)
		return NONS_NO_MUSIC_LOADED;
	stream->stop();
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::pause_music(){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	audio_stream *stream=this->get_channel(NONS_Audio::music_channel);
	if (!stream)
		return NONS_NO_MUSIC_LOADED;
	stream->pause();
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::resume_music(){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	audio_stream *stream=this->get_channel(NONS_Audio::music_channel);
	if (!stream)
		return NONS_NO_MUSIC_LOADED;
	stream->start();
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::play_sound(const std::wstring &filename,int channel,long times,bool automatic_cleanup){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	ErrorCode e;
	e=this->load_sound_on_a_channel(filename,channel,channel>=0);
	if (!CHECK_FLAG(e,NONS_NO_ERROR_FLAG))
		return e;
	e=this->play(channel,times,automatic_cleanup);
	return (!CHECK_FLAG(e,NONS_NO_ERROR_FLAG))?e:NONS_NO_ERROR;
}

ErrorCode NONS_Audio::load_sound_on_a_channel(const std::wstring &filename,int &channel,bool use_channel_as_input){
	if (!use_channel_as_input)
		channel=INT_MAX;
	if (this->uninitialized)
		return NONS_NO_ERROR;
	std::wstring converter_filename=wav2flac(filename);
	if (!general_archive.exists(converter_filename))
		return NONS_FILE_NOT_FOUND;
	NONS_MutexLocker ml(this->mutex);
	if (!use_channel_as_input)
		channel=this->channel_counter++;
	audio_stream *stream=this->get_channel(channel);
	if (stream)
		this->dev->remove(stream);
	stream=new audio_stream(converter_filename);
	if (!*stream){
		delete stream;
		return NONS_UNDEFINED_ERROR;
	}
	this->dev->add(stream);
	this->channels[channel]=stream;
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::unload_sound_from_channel(int channel){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(channel);
	if (!stream)
		return NONS_NO_SOUND_EFFECT_LOADED;
	this->dev->remove(stream);
	this->channels.erase(channel);
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::play(int channel,long times,bool automatic_cleanup){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(channel);
	if (!stream)
		return NONS_NO_SOUND_EFFECT_LOADED;
	stream->loop=times;
	stream->cleanup=automatic_cleanup;
	stream->set_general_volume(this->svol);
	stream->mute(!this->notmute);
	stream->start();
	return NONS_NO_ERROR;
}

void NONS_Audio::wait_for_channel(int channel){
	NONS_Event event;
	event.init();
	{
		NONS_MutexLocker ml(this->mutex);
		audio_stream *stream=this->get_channel(channel);
		stream->notify_on_stop(&event);
	}
	event.wait();
}

ErrorCode NONS_Audio::stop_sound(int channel){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(channel);
	if (!stream)
		return NONS_NO_SOUND_EFFECT_LOADED;
	stream->stop();
	return NONS_NO_ERROR;
}

ErrorCode NONS_Audio::stop_all_sound(){
	if (this->uninitialized)
		return NONS_NO_ERROR;
	NONS_MutexLocker ml(this->mutex);
	NONS_Audio_FOREACH()
		i->second->stop();
	return NONS_NO_ERROR;
}

int NONS_Audio::set_volume(float &p,int vol){
	if (this->uninitialized)
		return 0;
	if (vol<0){
		NONS_MutexLocker ml(this->mutex);
		return int(p*100.f);
	}
	saturate_value(vol,0,100);
	NONS_MutexLocker ml(this->mutex);
	p=vol/100.f;
	return vol;
}

int NONS_Audio::channel_volume(int channel,int vol){
	if (this->uninitialized)
		return 0;
	if (vol<0){
		NONS_MutexLocker ml(this->mutex);
		audio_stream *stream=this->get_channel(channel);
		if (!stream)
			return 0;
		return int(stream->get_volume()*100.f);
	}
	saturate_value(vol,0,100);
	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(channel);
	if (!stream)
		return 0;
	stream->set_volume(vol/100.f);
	return vol;
}

bool NONS_Audio::toggle_mute(){
	if (this->uninitialized)
		return 0;
	NONS_MutexLocker ml(this->mutex);
	this->notmute=!this->notmute;
	NONS_Audio_FOREACH()
		i->second->mute(!this->notmute);
	return this->notmute;
}

bool NONS_Audio::is_playing(int channel){
	if (this->uninitialized)
		return 0;
	NONS_MutexLocker ml(this->mutex);
	audio_stream *stream=this->get_channel(channel);
	if (!stream)
		return 0;
	return stream->is_sink_playing();
}

void read_channel(channel_listing::channel &c,const audio_stream &stream){
	if (!stream.is_playing()){
		c.filename.clear();
		c.loop=0;
		c.volume=100;
	}else{
		c.filename=stream.filename;
		c.loop=(stream.loop>0)?-1:0;
		c.volume=int(stream.get_volume()*100.f);
	}
}

void NONS_Audio::get_channel_listing(channel_listing &cl){
	if (this->uninitialized)
		return;
	NONS_MutexLocker ml(this->mutex);
	NONS_Audio_FOREACH(){
		if (i->first==-1)
			read_channel(cl.music,*i->second);
		else{
			channel_listing::channel c;
			read_channel(c,*i->second);
			cl.sounds[i->first]=c;
		}
	}
}

asynchronous_audio_stream *NONS_Audio::new_video_stream(){
	if (this->uninitialized)
		return 0;
	NONS_MutexLocker ml(this->mutex);
	asynchronous_audio_stream *stream=new asynchronous_audio_stream();
	stream->start();
	stream->mute(!this->notmute);
	this->dev->add(stream);
	this->channels[this->channel_counter++]=stream;
	return stream;
}

TiXmlElement *save_channel(int no,const audio_stream &stream){
	bool is_playing=stream.is_playing(),
		loop=!!stream.loop;
	if (!loop && is_playing)
		return 0;
	TiXmlElement *channel=new TiXmlElement("channel");
	channel->SetAttribute("no",no);
	channel->SetAttribute("path",stream.filename);
	channel->SetAttribute("loop",loop);
	channel->SetAttribute("volume",itoac(stream.get_volume()));
	channel->SetAttribute("playing",is_playing);
	return channel;
}

void NONS_Audio::load_channel(TiXmlElement *element){
	int index=element->QueryIntAttribute("no");
	audio_stream *stream=new audio_stream(element->QueryWStringAttribute("path"),index==NONS_Audio::music_channel);
	if (!*stream){
		delete stream;
		return;
	}
	this->channels[index]=stream;
	this->dev->add(stream);
	stream->loop=(element->QueryIntAttribute("loop"))?-1:0;
	stream->set_volume(element->QueryFloatAttribute("volume"));
	stream->set_general_volume((index==this->music_channel)?this->mvol:this->svol);
	stream->mute(!this->notmute);
	if (element->QueryIntAttribute("playing"))
		stream->start();
}

TiXmlElement *NONS_Audio::save_channels(){
	TiXmlElement *channels=new TiXmlElement("channels");
	NONS_MutexLocker ml(this->mutex);
	NONS_Audio_FOREACH(){
		TiXmlElement *el=save_channel(i->first,*i->second);
		if (!el)
			continue;
		channels->LinkEndChild(el);
	}
	return channels;
}

void NONS_Audio::load_channels(TiXmlElement *parent){
	TiXmlElement *channels=parent->FirstChildElement("channels");
	for (TiXmlElement *i=channels->FirstChildElement();i;i=i->NextSiblingElement())
		this->load_channel(i);
}

TiXmlElement *NONS_Audio::save(){
	TiXmlElement *audio=new TiXmlElement("audio");
	if (this->uninitialized)
		return audio;
	audio->SetAttribute("music_volume",this->music_volume(-1));
	audio->SetAttribute("sfx_volume",this->sound_volume(-1));
	audio->LinkEndChild(this->save_channels());
	return audio;
}

void NONS_Audio::load(TiXmlElement *parent){
	if (this->uninitialized)
		return;
	TiXmlElement *audio=parent->FirstChildElement("audio");
	this->music_volume(audio->QueryIntAttribute("music_volume"));
	this->sound_volume(audio->QueryIntAttribute("sfx_volume"));
	NONS_MutexLocker ml(this->mutex);
	NONS_Audio_FOREACH(){
		this->dev->remove(i->second);
	}
	this->channels.clear();
	this->load_channels(audio);
}
