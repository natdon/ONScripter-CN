/*
 * Copyright (c) 2004-2006  Kazunori "jagarl" Ueno
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*  music.cc	SDL_mixer を用いた音楽再生ルーチン */


#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#ifdef ENABLE_PATCH
#else
#include <signal.h>
#endif

#include"system/system_config.h"
#include"system/file.h"
#include "music.h"
#include<SDL.h>
#include<SDL_mixer.h>
#include"wavfile.h"

using namespace std;

int pcm_enable   = 0;
Mix_Chunk *play_chunk[MIX_PCM_SIZE];

MuSys::MuSys(AyuSysConfig& _config) : config(_config), movie_id(-1), music_enable(1) {
	int i;
	for (i=0; i<MIX_PCM_SIZE; i++)
		play_chunk[i] = 0;
	cdrom_track[0] = 0;
	effec_track[0] = 0;
}


// #define delete fprintf(stderr,"smus.cc: %d.",__LINE__), delete

void bgm_start(const char* path, int loop_pt);
void effec_start(int chn, const char* path, int loop, int fadein_time);
void bgm_fadeout(int time);

void MuSys::PlayCDROM(char* name, int play_count) {
	char wave[128]; wave[127] = '\0'; wave[0] = '\0';

	strcpy(cdrom_track, name);

	StopCDROM(0);
	strcpy(cdrom_track, name);

	/* name -> track */
	int track =config.track_name.CDTrack(name);
	if (track == -1) track = atoi(name);
	if (config.track_name.WaveTrack(name) != 0) strncpy(wave, config.track_name.WaveTrack(name), 127);
	if (wave[0] == 0 && track != 0) { /* DSTRACK が見つからない場合、CDTRACKを使用する */
		sprintf(wave, "audio_%02d",track);
	}
	if (wave == 0) return;
	// BGM 再生
	if (!pcm_enable) return;
	if (play_count == 0)
		bgm_start(wave, -1);
	else
		bgm_start(wave, config.track_name.TrackStart(name));
	return;
}

void MuSys::StopCDROM(int time)
{
	cdrom_track[0] = '\0';
	if (!pcm_enable) return;
	bgm_fadeout(time);
}

void MuSys::PlaySE(const char* se, int loop_flag, int channel) {
	if (! pcm_enable) return;
	if (loop_flag)
		effec_start(MIX_PCM_EFFEC, se, 10000, 0);
	else
		effec_start(MIX_PCM_EFFEC, se, 0, 0);
	return;
}
void MuSys::PlaySE(int number) {
	if (! pcm_enable) return;
	const char* se_name = config.track_name.SETrack(number);
	if (se_name == 0) return;
	effec_start(MIX_PCM_EFFEC, se_name, 0, 0);
	return;
}
void MuSys::StopSE(int time) {
	if (! pcm_enable) return;
	if (time == 0) 
		Mix_HaltChannel(MIX_PCM_EFFEC);
	else
		Mix_FadeOutChannel(MIX_PCM_EFFEC, time);
}
bool MuSys::IsStopSE(void) {
	if (! pcm_enable) return true;
	if (Mix_Playing(MIX_PCM_EFFEC) != 0) return false;
	return true;
}

void MuSys::StopKoe(int time) {
	if (! pcm_enable) return;
	if (time == 0) Mix_HaltChannel(MIX_PCM_KOE);
	else Mix_FadeOutChannel(MIX_PCM_KOE, time);
}

void MuSys::InitMusic(void)
{
	if (music_enable != 1) return;
	cdrom_track[0] = '\0';
	if ( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, DEFAULT_AUDIOBUF ) < 0 ){
//	if ( Mix_OpenAudio( 48000, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, DEFAULT_AUDIOBUF ) < 0 ){
		return;
	}
	int freq, channels; Uint16 format;
	if ( Mix_QuerySpec(&freq, &format, &channels) ) {
		WAVFILE::freq = freq;
		WAVFILE::format = format;
		WAVFILE::channels = channels;
	}
	pcm_enable = 1;
	Mix_AllocateChannels( MIX_PCM_SIZE);
	music_enable = 2;
	return;
}
void MuSys::FinalizeMusic(void)
{
	if (music_enable != 2) return;
	int i;
	for (i=0; i<MIX_PCM_SIZE; i++) {
		Mix_HaltChannel(i);
		if (play_chunk[i]) {
			Mix_FreeChunk(play_chunk[i]);
		}
		play_chunk[i] = 0;
	}
	Mix_HaltMusic();
	Mix_HookMusic(0,0);
	Mix_CloseAudio();
	pcm_enable = 0;
	music_enable = 1;
}

/*************************************************************************
**
** ファイル読み込み / 外部コマンド呼び出し
*/

struct WavChunk {
	WAVFILE* wav;
	int loop_pt;
	static void callback(void* userdata, Uint8* stream, int len);
};
WavChunk wav_playing;
static int fadetime_total;
static int fadecount;

void WavChunk::callback(void *userdata, Uint8 *stream, int len)
{
	WavChunk* chunk = (WavChunk*)userdata;
	int count;
	if (chunk->loop_pt == -2) { // 再生終了後
		memset(stream, 0, len);
		return;
	}
	count = chunk->wav->Read( (char*)stream, 4, len/4);

	if (count != len/4) {
		memset(stream+count*4, 0, len-count*4);
		// 最後まで再生した
		if (chunk->loop_pt == -1) { // 終了
			chunk->loop_pt = -2;
		} else {
			chunk->wav->Seek(chunk->loop_pt);
			chunk->wav->Read( (char*)(stream+count*4), 4, len/4-count);
		}
	}
	if (fadetime_total) {
		// 音楽を停止中 (fade out)
		int count_total = fadetime_total*(WAVFILE::freq/1000);
		if (fadecount > count_total || fadetime_total == 1) { // 音楽停止
			chunk->loop_pt = -2;
			memset(stream, 0, len);
			return;
		}
		// int cur_vol = 256*(count_total-fadecount)/count_total;
		int cur_vol = SDL_MIX_MAXVOLUME*(count_total-fadecount)/count_total;
		char* stream_dup = new char[len];
		memcpy(stream_dup, stream, len);
		memset(stream, 0, len);
		SDL_MixAudio(stream, (Uint8*)stream_dup, len, cur_vol);
		fadecount += len/4;
	}
	return;
}
void bgm_fadeout(int time) {
	fadecount = 0;
	if (time <= 0) time = 1;
	fadetime_total = time;
}

static SDL_RWops* OpenSDLRW(const char* path);
static WAVFILE* OpenWaveFile(const char* path);
void bgm_start(const char* path, int loop_pt) {
	if (! pcm_enable) return;
fprintf(stderr,"bgm start %s\n",path);
	WAVFILE* wav = OpenWaveFile(path);
	if (wav == 0) return;
	Mix_PauseMusic();
	Mix_HaltMusic();
	Mix_HookMusic(0,0);
	/* 前に再生していたのを終了 */
	if (wav_playing.wav) {
		delete wav_playing.wav;
		wav_playing.wav = 0;
	}
	wav_playing.wav = wav;
	wav_playing.loop_pt = loop_pt;
	fadetime_total = 0;
	fadecount = 0;
	Mix_HookMusic( &(WavChunk::callback), (void*)&wav_playing);
	Mix_VolumeMusic(128);
	return;
}

void effec_start(int chn, const char* path, int loop, int fadein_time) {
	if (! pcm_enable) return;
	SDL_RWops* op = OpenSDLRW(path);
	if (op == 0) { // ファイルが見付からない
		return;
	}
	Mix_Pause(chn);

	if (play_chunk[chn]) {
		Mix_FreeChunk(play_chunk[chn]);
	}
	play_chunk[chn] = Mix_LoadWAV_RW(op, 1);
	if (fadein_time <= 0) {
		Mix_Volume(chn, 128);
		Mix_PlayChannel(chn, play_chunk[chn],loop);
	} else {
		Mix_Volume(chn, 128);
		Mix_FadeInChannel(chn, play_chunk[chn],loop,fadein_time);
	}
	return;
}

void MuSys::PlayKoe(const char* path) {
	if (! pcm_enable) return;
	static char* playing_koedata = 0;
	int len = 0;
	AvgKoeInfo koeinfo;
	int chn = MIX_PCM_KOE;

	Mix_Pause(chn);
	Mix_HaltChannel(chn); // これで RWop が解放されるはず…
	if (play_chunk[chn]) {
		Mix_FreeChunk(play_chunk[chn]);
		play_chunk[chn] = 0;
	}

	if (playing_koedata) {
		delete[] playing_koedata;
		playing_koedata = 0;
	}

	koeinfo = OpenKoeFile(path);

	if (koeinfo.stream == 0) return;
	playing_koedata = decode_koe(koeinfo, &len);
	fclose(koeinfo.stream);
	if (playing_koedata == 0) {
		return;
	}
	Mix_Volume(chn, 128);
	play_chunk[chn] = Mix_LoadWAV_RW(SDL_RWFromMem(playing_koedata, len+0x2c), 1);
	Mix_PlayChannel(chn, play_chunk[chn],0);
	return;
}
AvgKoeInfo OpenKoeFile(const char* path) {
	int radix = 10000;
	/* if (global_system.Version() >= 2) */ radix *= 10;
	AvgKoeInfo info;
	info.stream = 0; info.length = 0; info.offset = 0;
	if (isdigit(path[0]) && strchr(path,'.') == 0) { // 数値 (拡張子等なし)
		/* avg32 形式の音声アーカイブのキャッシュを検索 */
		int pointer = atoi(path);
		int file_no = pointer / radix;
		int index = pointer % radix;
		info = FindKoe(file_no, index);
	} else { // ファイル
		int length;
		ARCINFO* arcinfo = file_searcher.Find(FILESEARCH::KOE,path,".WPD");
		if (arcinfo == 0) return info;
		info.stream = arcinfo->OpenFile(&length);
		info.rate = 22050;
		info.length = length;
		info.offset = ftell(info.stream);
		info.type = koe_unknown;
		delete arcinfo;
	}
	return info;
}

static SDL_RWops* OpenSDLRW(const char* path) {
	char cmdline_buf[1024];
	/* まず wav ファイルを探す */
	ARCINFO* info = file_searcher.Find(FILESEARCH::WAV,path,".wav");
	if (info == 0) {
		info = file_searcher.Find(FILESEARCH::WAV,path,".nwa");
		if (info == 0) info = file_searcher.Find(FILESEARCH::BGM,path,"nwa");
		if (info) { // read NWA file

			static char* nwa_buffer = 0;
			int dummy;
			FILE* f = info->OpenFile(&dummy);
			static char* d = 0;
			int sz;
			if (d != 0) delete[] d;
			d = NWAFILE::ReadAll(f, sz);
			return SDL_RWFromMem(d, sz);
		}
	}
	if (info == 0) info = file_searcher.Find(FILESEARCH::BGM,path,"wav");
	if (info == 0) info = file_searcher.Find(FILESEARCH::WAV,path,".ogg");
	if (info) {
		int dummy;
		FILE* f = info->OpenFile(&dummy);
		delete info;
		if (f == 0) return 0;
		SDL_RWops* op = SDL_RWFromFP(f, 1);
		return op;
	}
	return 0;
}

static WAVFILE* OpenWaveFile(const char* path) {
	char cmdline_buf[1024];
	/* まず wav ファイルを探す */
	ARCINFO* info = file_searcher.Find(FILESEARCH::WAV,path,".wav");
	if (info == 0) info = file_searcher.Find(FILESEARCH::BGM,path,"wav");
	if (info) {
		int size;
		FILE* f = info->OpenFile(&size);
		delete info;
		if (f == 0) return 0;
		WAVFILE* w = WAVFILE::MakeConverter(new WAVFILE_Stream(f, size));
		return w;
	}
	/* 次に nwa ファイル */
	info = file_searcher.Find(FILESEARCH::WAV,path,".nwa");
	if (info == 0) info = file_searcher.Find(FILESEARCH::BGM,path,"nwa");
	if (info) {
		int size;
		FILE* f = info->OpenFile(&size);
		delete info;
		if (f == 0) return 0;
		WAVFILE* w = WAVFILE::MakeConverter(new NWAFILE(f));
		return w;
	}

	/* 次に mp3 ファイル */
	info = file_searcher.Find(FILESEARCH::WAV,path,".mp3");
	if (info == 0) info = file_searcher.Find(FILESEARCH::BGM,path,"mp3");
	if (info) {
		int size;
		FILE* f = info->OpenFile(&size);
		delete info;
		if (f == 0) return 0;
		MP3FILE* w = new MP3FILE(f, size);
		if (w->pimpl) {
			return WAVFILE::MakeConverter(w);
		}
		delete w;
	}

	/* 次に ogg ファイル */
	info = file_searcher.Find(FILESEARCH::WAV,path,".ogg");
	if (info == 0) info = file_searcher.Find(FILESEARCH::BGM,path,"ogg");
	if (info) {
		int size;
		FILE* f = info->OpenFile(&size);
		delete info;
		if (f == 0) return 0;
		OggFILE* w = new OggFILE(f, size);
		if (w->pimpl) {
			return WAVFILE::MakeConverter(w);
		}
		delete w;
	}
	return 0;
}

