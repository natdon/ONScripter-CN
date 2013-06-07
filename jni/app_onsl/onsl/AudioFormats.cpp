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

#include "AudioFormats.h"
#include <iostream>
#include <cerrno>

mp3_static_data mp3_decoder::static_data;
mod_static_data mod_decoder::static_data;
midi_static_data midi_decoder::static_data;

size_t ogg_read(void *buffer,size_t size,size_t nmemb,void *s){
	NONS_DataStream *stream=(NONS_DataStream *)s;
	if (!stream->read(buffer,nmemb,size*nmemb)){
		errno=1;
		return 0;
	}
	errno=0;
	return nmemb/size;
}

int ogg_seek(void *s,ogg_int64_t offset,int whence){
	NONS_DataStream *stream=(NONS_DataStream *)s;
	stream->stdio_seek(offset,whence);
	return 0;
}

long ogg_tell(void *s){
	return (long)((NONS_DataStream *)s)->seek(0,0);
}

std::string ogg_code_to_string(int e){
	switch (e){
		case 0:
			return "no error";
		case OV_EREAD:
			return "a read from media returned an error";
		case OV_ENOTVORBIS:
			return "bitstream does not contain any Vorbis data";
		case OV_EVERSION:
			return "vorbis version mismatch";
		case OV_EBADHEADER:
			return "invalid Vorbis bitstream header";
		case OV_EFAULT:
			return "internal logic fault; indicates a bug or heap/stack corruption";
		default:
			return "unknown error";
	}
}

ogg_decoder::ogg_decoder(NONS_DataStream *stream):decoder(stream){
	if (!*this)
		return;
	this->bitstream=0;
	ov_callbacks cb;
	cb.read_func=ogg_read;
	cb.seek_func=ogg_seek;
	cb.tell_func=ogg_tell;
	cb.close_func=0;
	int error=ov_open_callbacks(stream,&this->file,0,0,cb);
	if (error<0){
		o_stderr <<ogg_code_to_string(error);
		this->good=0;
		return;
	}
}

ogg_decoder::~ogg_decoder(){
	ov_clear(&this->file);
}

audio_buffer *ogg_decoder::get_buffer(bool &error){
	static char nativeEndianness=checkNativeEndianness();
	const size_t n=1<<12;
	char *temp=(char *)audio_buffer::allocate(n/4,2,2);
	size_t size=0;
	while (size<n){
		int r=ov_read(&this->file,temp+size,n-size,nativeEndianness==NONS_BIG_ENDIAN,2,1,&this->bitstream);
		if (r<0){
			error=1;
			audio_buffer::deallocate(temp);
			return 0;
		}
		error=0;
		if (!r){
			if (!size){
				audio_buffer::deallocate(temp);
				return 0;
			}
			break;
		}
		size+=r;
	}
	vorbis_info *i=ov_info(&this->file,this->bitstream);
	return new audio_buffer(temp,size/(i->channels*2),i->rate,i->channels,16,1);
}

void ogg_decoder::loop(){
	ov_pcm_seek(&this->file,0);
}

inline int16_t fix_24bit_sample(FLAC__int32 v){
	return (v+0x80)>>8;
}

inline int16_t fix_32bit_sample(FLAC__int32 v){
	const FLAC__int32 max_i32=0xFFFFFFFF;
	return ((max_i32-0x80<v)?max_i32:v+0x80)>>8;
}

flac_decoder::flac_decoder(NONS_DataStream *stream):decoder(stream),buffer(0){
	if (!*this)
		return;
	this->set_md5_checking(1);
	this->good=0;
	if (this->init()!=FLAC__STREAM_DECODER_INIT_STATUS_OK)
		return;
	this->good=1;
}

flac_decoder::~flac_decoder(){
	delete this->buffer;
}

FLAC__StreamDecoderWriteStatus flac_decoder::write_callback(const FLAC__Frame *frame,const FLAC__int32 * const *buffer){
	void *push_me=0;
	bool stereo=frame->header.channels!=1;
	ulong channels=stereo?2:1,
		bits=0;
	switch (frame->header.bits_per_sample){
		case 8:
			bits=8;
			{
				push_me=audio_buffer::allocate(frame->header.blocksize,bits/8,channels);
				int8_t *temp_buffer=(int8_t *)push_me;
				for (size_t i=0;i<frame->header.blocksize;i++){
					temp_buffer[i*channels]=buffer[0][i];
					if (stereo)
						temp_buffer[i*channels+1]=buffer[1][i];
				}
			}
			break;
		case 16:
			bits=16;
			{
				push_me=audio_buffer::allocate(frame->header.blocksize,bits/8,channels);
				int16_t *temp_buffer=(int16_t *)push_me;
				for (size_t i=0;i<frame->header.blocksize;i++){
					temp_buffer[i*channels]=buffer[0][i];
					if (stereo)
						temp_buffer[i*channels+1]=buffer[1][i];
				}
			}
			break;
		case 24:
			bits=16;
			{
				push_me=audio_buffer::allocate(frame->header.blocksize,bits/8,channels);
				int16_t *temp_buffer=(int16_t *)push_me;
				for (size_t i=0;i<frame->header.blocksize;i++){
					temp_buffer[i*channels]=fix_24bit_sample(buffer[0][i]);
					if (stereo)
						temp_buffer[i*channels+1]=fix_24bit_sample(buffer[1][i]);
				}
			}
			break;
		case 32:
			bits=16;
			{
				push_me=audio_buffer::allocate(frame->header.blocksize,bits/8,channels);
				int16_t *temp_buffer=(int16_t *)push_me;
				for (size_t i=0;i<frame->header.blocksize;i++){
					temp_buffer[i*channels]=fix_32bit_sample(buffer[0][i]);
					if (stereo)
						temp_buffer[i*channels+1]=fix_32bit_sample(buffer[1][i]);
				}
			}
			break;
	}
	assert(!this->buffer);
	this->buffer=new audio_buffer(push_me,frame->header.blocksize,frame->header.sample_rate,channels,bits,1);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus flac_decoder::read_callback(FLAC__byte *buffer,size_t *bytes){
	return
		(!this->stream || !this->stream->read(buffer,*bytes,*bytes))
		?FLAC__STREAM_DECODER_READ_STATUS_ABORT
		:FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus flac_decoder::seek_callback(FLAC__uint64 absolute_byte_offset){
	if (!this->stream)
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	this->stream->seek(absolute_byte_offset,1);
	return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

bool flac_decoder::eof_callback(){
	return (!this->stream)?1:this->stream->get_offset()>=this->stream->get_size();
}

FLAC__StreamDecoderTellStatus flac_decoder::tell_callback(FLAC__uint64 *absolute_byte_offset){
	if (!this->stream)
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	*absolute_byte_offset=this->stream->get_offset();
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus flac_decoder::length_callback(FLAC__uint64 *stream_length){
	if (!this->stream)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	*stream_length=this->stream->get_size();
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

audio_buffer *flac_decoder::get_buffer(bool &error){
	bool ok=1;
	while (!this->buffer && (ok=this->process_single()) && this->get_state()!=FLAC__STREAM_DECODER_END_OF_STREAM);
	error=!ok;
	audio_buffer *ret=this->buffer;
	this->buffer=0;
	return ret;
}

void flac_decoder::loop(){
	this->seek_absolute(0);
}

struct fd_tracker{
	NONS_Mutex mutex;
	int counter;
	typedef std::map<int,NONS_DataStream *> map_t;
	map_t map;
	fd_tracker():counter(0){}
	int add(NONS_DataStream *);
	NONS_DataStream *get(int);
} tracker;

int fd_tracker::add(NONS_DataStream *s){
	NONS_MutexLocker ml(this->mutex);
	this->map[this->counter]=s;
	return this->counter++;
}

NONS_DataStream *fd_tracker::get(int fd){
	NONS_MutexLocker ml(this->mutex);
	return this->map[fd];
}

ssize_t mp3_read(int fd,void *dst,size_t n){
	NONS_DataStream *stream=tracker.get(fd);
	stream->read(dst,n,n);
	return n;
}

off_t mp3_seek(int fd,off_t offset,int whence){
	NONS_DataStream *stream=tracker.get(fd);
	return (off_t)stream->stdio_seek(offset,whence);
}

#define HANDLE_MPG123_ERRORS(call,r) {                 \
	error=call;                                        \
	if (error!=MPG123_OK){                             \
		o_stderr <<mpg123_plain_strerror(error)<<"\n"; \
		return r;                                      \
	}                                                  \
}

bool static_initializer::init(){
	NONS_MutexLocker ml(this->mutex);
	if (this->initialized)
		return 1;
	if (!this->perform_initialization())
		return 0;
	this->initialized=1;
	return 1;
}

bool mp3_static_data::perform_initialization(){
	int error;
	HANDLE_MPG123_ERRORS(mpg123_init(),0);
	return 1;
}

mp3_static_data::~mp3_static_data(){
	if (this->initialized)
		mpg123_exit();
}

mp3_decoder::mp3_decoder(NONS_DataStream *stream):decoder(stream){
	if (!*this)
		return;
	this->good=0;
	this->has_played=0;
	int error;
	if (!mp3_decoder::static_data.init())
		return;
	this->handle=mpg123_new(0,&error);
	if (!this->handle){
		o_stderr <<mpg123_plain_strerror(error)<<"\n";
		return;
	}
	HANDLE_MPG123_ERRORS(mpg123_replace_reader(this->handle,mp3_read,mp3_seek),);
	HANDLE_MPG123_ERRORS(mpg123_open_fd(this->handle,tracker.add(stream)),);
	HANDLE_MPG123_ERRORS(mpg123_format_none(this->handle),);
	HANDLE_MPG123_ERRORS(mpg123_format(this->handle,44100,MPG123_STEREO,MPG123_ENC_SIGNED_16),);
	this->good=1;
}

mp3_decoder::~mp3_decoder(){
	mpg123_close(this->handle);
	mpg123_delete(this->handle);
}

audio_buffer *mp3_decoder::get_buffer(bool &there_was_an_error){
	there_was_an_error=1;
	int error;
	HANDLE_MPG123_ERRORS(mpg123_format(this->handle,44100,MPG123_STEREO,MPG123_ENC_SIGNED_16),0);
	uchar *buffer;
	off_t offset;
	size_t size;
	error=mpg123_decode_frame(this->handle,&offset,&buffer,&size);
	if (error==MPG123_DONE){
		there_was_an_error=0;
		return 0;
	}
	if (error!=MPG123_OK && error!=MPG123_NEW_FORMAT){
		o_stderr <<mpg123_plain_strerror(error)<<"\n";
		return 0;
	}
	ulong channels=2;
	there_was_an_error=0;
	this->has_played=1;
	return new audio_buffer(buffer,size/(channels*2),44100,channels,16);
}

void mp3_decoder::loop(){
	if (this->has_played)
		mpg123_seek(this->handle,0,SEEK_SET);
}

static const size_t BUFFERSIZE=32768;
static std::vector<SBYTE> audiobuffer;

void openal_Exit(){
	VC_Exit();
	audiobuffer.clear();
}

void openal_Update(){
	audiobuffer.resize(BUFFERSIZE);
	size_t n=VC_WriteBytes(&audiobuffer[0],BUFFERSIZE);
	audiobuffer.resize(n);
}

BOOL openal_Reset(){
	VC_Exit();
	return VC_Init();
}

BOOL openal_IsPresent(){
	return 1;
}

MDRIVER drv_openal={
	NULL,
	"openal",
	"OpenAL",
	0,255,
	"openal",

	0,
	openal_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	VC_Init,
	openal_Exit,
	openal_Reset,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	openal_Update,
	0,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

bool mod_static_data::perform_initialization(){
	MikMod_RegisterDriver(&drv_openal);
	md_mode|=DMODE_SOFT_MUSIC|DMODE_16BITS|DMODE_STEREO;
	md_mixfreq=44100;
	md_volume=100;
	MikMod_RegisterAllLoaders();
	//Shut GCC up:
	char s[]="";
	if (MikMod_Init(s))
		return 0;
	return 1;
}

mod_static_data::~mod_static_data(){
	if (this->initialized)
		MikMod_Exit();
}

struct NONS_MREADER{
	MREADER base;
	NONS_DataStream *stream;
};

BOOL mod_seek(MREADER *stream,long offset,int whence){
	((NONS_MREADER *)stream)->stream->stdio_seek(offset,whence);
	return 1;
}

long mod_tell(MREADER *stream){
	return (long)((NONS_MREADER *)stream)->stream->get_offset();
}

BOOL mod_read(MREADER *stream,void *dst,size_t n){
	return ((NONS_MREADER *)stream)->stream->read(dst,n,n);
}

int mod_get(MREADER *stream){
	uchar byte;
	size_t n=1;
	if (!((NONS_MREADER *)stream)->stream->read(&byte,n,n) || !n)
		return -1;
	return byte;
}

BOOL mod_eof(MREADER *stream){
	return ((NONS_MREADER *)stream)->stream->get_offset()>=((NONS_MREADER *)stream)->stream->get_size();
}

mod_decoder::mod_decoder(NONS_DataStream *stream):decoder(stream){
	if (!*this)
		return;
	this->good=0;
	if (!mod_decoder::static_data.init())
		return;
	NONS_MREADER reader;
	reader.base.Eof=mod_eof;
	reader.base.Get=mod_get;
	reader.base.Read=mod_read;
	reader.base.Seek=mod_seek;
	reader.base.Tell=mod_tell;
	reader.stream=stream;
	this->module=Player_LoadGeneric((MREADER *)&reader,64,0);
	this->good=this->module;
	if (!this->good){
		o_stderr <<MikMod_strerror(MikMod_errno)<<"\n";
		return;
	}
	Player_Start(this->module);
}

mod_decoder::~mod_decoder(){
	if (this->good)
		Player_Free(this->module);
}

audio_buffer *mod_decoder::get_buffer(bool &error){
	error=0;
	if (!Player_Active())
		return 0;
	MikMod_Update();
	return new audio_buffer(&audiobuffer[0],audiobuffer.size()/4,44100,2,16);
}

void mod_decoder::loop(){
	Player_SetPosition(0);
	Player_Start(this->module);
}

custom_FILE NONS_fopen(const char *filename){
	return midi_decoder::static_data.get_file(filename);
}

int NONS_fclose(custom_FILE file){
	return 0;
}

size_t NONS_fread(void *ptr,size_t size,size_t nmemb,custom_FILE file){
	((NONS_DataStream *)file)->read(ptr,nmemb,size*nmemb);
	return nmemb/size;
}

int NONS_fseek(custom_FILE stream,long int offset,int origin){
	return (int)((NONS_DataStream *)stream)->stdio_seek(offset,origin);
}

custom_stdio init_stdio(){
	custom_stdio stdio;
	stdio.fopen=NONS_fopen;
	stdio.fclose=NONS_fclose;
	stdio.fread=NONS_fread;
	stdio.fseek=NONS_fseek;
	return stdio;
}

bool midi_static_data::perform_initialization(){
	custom_stdio stdio=init_stdio();
	if (mid_init(0,&stdio))
		return 0;
	if (!general_archive.addArchive(L"timidity.zip"))
		general_archive.addArchive(L"timidity.oaf");
	return 1;
}

midi_static_data::~midi_static_data(){
	if (this->initialized)
		mid_exit();
}

void midi_static_data::cache_file(const char *path,NONS_DataStream *stream){
	std::wstring temp=UniFromUTF8(std::string(path));
	tolower(temp);
	toforwardslash(temp);
	NONS_MutexLocker ml(this->mutex);
	this->cached_files[temp]=stream;
}

NONS_DataStream *midi_static_data::get_file(const char *path){
	std::wstring temp=UniFromUTF8(std::string(path));
	tolower(temp);
	toforwardslash(temp);
	NONS_MutexLocker ml(this->mutex);
	midi_static_data::map_t::iterator i=this->cached_files.find(temp);
	if (i!=this->cached_files.end()){
		i->second->reset();
		return i->second;
	}
	NONS_DataStream *stream=general_archive.open(temp,KEEP_IN_MEMORY);
	if (!stream)
		return 0;
	return this->cached_files[temp]=stream;
}

size_t midi_read(void *ctx,void *ptr,size_t size,size_t nmemb){
	((NONS_DataStream *)ctx)->read(ptr,nmemb,size*nmemb);
	return nmemb/size;
}

int midi_close(void *ctx){
	return 0;
}

midi_decoder::midi_decoder(NONS_DataStream *stream):decoder(stream){
	if (!*this)
		return;
	this->good=0;
	if (!midi_decoder::static_data.init())
		return;
	MidIStream *file=mid_istream_open_callbacks(midi_read,midi_close,stream);
	if (!file)
		return;
	MidSongOptions options;
	options.buffer_size=1024;
	options.channels=2;
	options.format=MID_AUDIO_S16LSB;
	options.rate=44100;
	custom_stdio stdio=init_stdio();
	this->song=mid_song_load(file,&options,&stdio);
	mid_istream_close(file);
	if (!this->song)
		return;
	this->good=1;
	mid_song_start(this->song);
}

midi_decoder::~midi_decoder(){
	if (*this)
		mid_song_free(this->song);
}

audio_buffer *midi_decoder::get_buffer(bool &error){
	error=0;
	audio_buffer *buffer=(audio_buffer *)audio_buffer::allocate(1024,2,2);
	size_t bytes_read=mid_song_read_wave(this->song,buffer,1024*4);
	if (!bytes_read){
		audio_buffer::deallocate(buffer);
		return 0;
	}
	return new audio_buffer(buffer,bytes_read/4,44100,2,16,1);
}

void midi_decoder::loop(){
	mid_song_seek(this->song,0);
	mid_song_start(this->song);
}
