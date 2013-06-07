/*
 * music_client.c  music client 部
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
/* $Id: music_client.c,v 1.18 2004/10/31 04:18:06 chikama Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <glib.h>

#include "portab.h"
#include "music.h"
#include "music_client.h"
#include "counter.h"
#include "pcmlib.h"
#include "nact.h"
#include "ald_manager.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <android/log.h>

/* それぞれの sub system が使用可能かどうか */
static boolean cdrom_available;
static boolean midi_available;
static boolean audio_available;

static boolean mixer_available = FALSE;
static Mix_Music *mixer_music = NULL;
static int mixer_music_cdrom_track = -1;

static int mixer_music_stop() {
	
	if(mixer_music){
		Mix_HaltMusic();
		Mix_FreeMusic(mixer_music);
		mixer_music = NULL;
	}
	mixer_music_cdrom_track = -1;
	
	return OK;
}

int musclient_init() {
	void *data;
	int i;
	
	cdrom_available = FALSE;
	midi_available  = FALSE;
	audio_available = FALSE;

	if( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 ){
		__android_log_print(ANDROID_LOG_DEBUG, "cdrom", "failed : SDL_InitSubSystem(SDL_INIT_AUDIO)");
		return NG;		
	}

    if(Mix_OpenAudio(44100/*22050*/, AUDIO_S16, 2, 1024/*4096*/) < 0){
		__android_log_print(ANDROID_LOG_DEBUG, "cdrom", "failed : Mix_OpenAudio()");
        return NG;
    }
	
	Mix_AllocateChannels(3);
	
	mixer_available = TRUE;
	cdrom_available = TRUE;
	midi_available  = FALSE;
	audio_available = TRUE;

	return OK;
}

int musclient_exit() {
	if(mixer_available){
		mixer_music_stop();
		Mix_CloseAudio();
		mixer_available = FALSE;
	}
	return OK;
}

/*
 * cdrom の演奏開始 
 *   track: トラック番号 (第一トラックは 1)
 *   loop : 繰り返し回数 (0の場合は無限)
 */
int mus_cdrom_start(int track, int loop) {
	int v[2];
	int fd;
	
	if (!cdrom_available) return NG;
	
	mus_cdrom_stop();

	char music_path[256];
	
	//MP3
	if(mixer_music == NULL){
		sprintf(music_path, "cd/track%02d.mp3", track);
		mixer_music = Mix_LoadMUS(music_path);
	}
	
	//OGG
	if(mixer_music == NULL){
		sprintf(music_path, "cd/track%02d.ogg", track);
		mixer_music = Mix_LoadMUS(music_path);
	}

	//WAV
	if(mixer_music == NULL){
		sprintf(music_path, "cd/track%02d.wav", track);
		mixer_music = Mix_LoadMUS(music_path);
	}
	
	if(mixer_music){
		//__android_log_print(ANDROID_LOG_DEBUG, "cdrom", "Mix_PlayMusic() : name=%s track=%d", music_path, track);
		mixer_music_cdrom_track = track;
		Mix_PlayMusic(mixer_music, (loop - 1));
		return OK;
	} else {
		//__android_log_print(ANDROID_LOG_DEBUG, "cdrom", "audio_music is NULL");
		return NG;
	}
	
	return OK;
}

/*
 * cdrom の演奏停止
 */
int mus_cdrom_stop() {
	if (!cdrom_available) return NG;
	
	return mixer_music_stop();
}

/*
 * cdrom の演奏状態の取得
 *   info: 演奏時間(track/min/sec/frame)の状態を格納する場所
 *         停止している場合は 999/999/999/999 が返る
 */
int mus_cdrom_get_playposition(cd_time *tm) {
	void *data;
	
	if (!cdrom_available) return NG;
	
	tm->t = (mixer_music_cdrom_track < 0 ? 999 : mixer_music_cdrom_track);
	tm->m = tm->s = tm->f = 999;
	
	return NG;

	return OK;
}

/*
 * cdrom の最大トラック数の取得
 *   
 */
int mus_cdrom_get_maxtrack() {
	int trk;
	
	if (!cdrom_available) return 0;
	
	trk = 999;
	
	return trk;
}

/*
 * CDROM の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_cdrom_get_state() {
	return cdrom_available;
}

/*
 * midi の演奏開始 
 *   no  : ファイル番号( no >= 1)
 *   loop: 繰り返し回数 (0の場合は無限)
 */
int mus_midi_start(int no, int loop) {
	dridata *dfile;

	if (!midi_available) return NG;
/*
	mus_midi_stop();
	dfile = ald_getdata(DRIFILE_MIDI, (no - 1));
	if(dfile == NULL) {
		return NG;
	}
	
	FILE* fp = fopen("tmp.mid", "wb");
	if(fp == NULL){
		ald_freedata(dfile);
		return NG;
	}
	fwrite(dfile->data, 1, dfile->size, fp);
	fclose(fp);
	
	ald_freedata(dfile);
	
	mixer_music = Mix_LoadMUS("tmp.mid");
	
	if(mixer_music == NULL){
		return NG;
	}
	
	Mix_PlayMusic(mixer_music, (loop - 1));
*/
	return OK;
}

/*
 * midi の演奏停止
 */
int mus_midi_stop(void) {
	if (!midi_available) return NG;
	
	return mixer_music_stop();
}

/*
 * midi の一時停止
 */
int mus_midi_pause(void) {
	if (!midi_available) return NG;
/*	
	Mix_PauseMusic();
*/
	return OK;
}

/*
 * midi の一時停止解除
 */
int mus_midi_unpause(void) {
	if (!midi_available) return NG;
/*
	Mix_ResumeMusic();
*/
	return OK;
}

/*
 * midi の演奏状態の取得
 *  state: 演奏時間や番号の状態を格納する場所
 *         停止している場合は 0 が入る
 */
int mus_midi_get_playposition(midiplaystate *state) {
	if (!midi_available) return NG;

	return OK;
}

/*
 * midi の演奏 flag/variable の状態を設定する
 *   mode : 0 -> flag mode
 *          1 -> variable mode
 *   index: flag/variable 番号
 *   val  : 書き込む値
 */
int mus_midi_set_flag(int mode, int index, int val) {

	if (!midi_available) return NG;

	return OK;
}

/*
 * midi の演奏 flag/variable の状態を取得する
 *   mode : 0 -> flag mode
 *          1 -> variable mode
 *   index: flag/variable 番号
 *
 *   return : flag/variable の値
 */
int mus_midi_get_flag(int mode, int index) {

	int val = 0;

	if (!midi_available) return NG;

	return val;
}

/*
 * MIDI の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_midi_get_state() {
	return midi_available;
}

static dridata* mixer_pcm_dfiles[2] = {NULL, NULL};
static Mix_Chunk* mixer_pcm_chunks[2] = {NULL, NULL};

/*
 * WAV の演奏開始 (command S?)
 *   no  : ファイル番号( no >= 1)
 *   loop: 繰り返し回数 (0の場合は無限)
 */
int mus_pcm_start(int no, int loop) {

	if (!audio_available) return NG;

	mus_pcm_stop(0);
	
	mixer_pcm_dfiles[0] = ald_getdata(DRIFILE_WAVE, (no - 1));
	if (mixer_pcm_dfiles[0] == NULL) {
		//WARNING("DRIFILE_WAVE fail to open %d\n", (no - 1));
		return NG;
	}
	
	mixer_pcm_chunks[0] = Mix_LoadWAV_RW(SDL_RWFromMem(mixer_pcm_dfiles[0]->data, mixer_pcm_dfiles[0]->size), 1);
	if(mixer_pcm_chunks[0] == NULL){
		ald_freedata(mixer_pcm_dfiles[0]);
		mixer_pcm_dfiles[0] = NULL;
		return NG;
	}
	
	Mix_SetPanning(0, 255, 255);
	Mix_PlayChannel(0, mixer_pcm_chunks[0], (loop - 1));

	return OK;
}

/*
 * WAV を左右 mix して演奏
 *   noL : 左用のファイル番号(noL >= 1)
 *   noR : 右用のファイル番号(noR >= 1)
 *   loop: 繰り返し数(0の場合は無限ループ)
 */
int mus_pcm_mix(int noL, int noR, int loop) {

	if (!audio_available) return NG;

	mus_pcm_stop(0);
	
	mixer_pcm_dfiles[0] = ald_getdata(DRIFILE_WAVE, (noL - 1));
	if (mixer_pcm_dfiles[0] == NULL) {
		//WARNING("DRIFILE_WAVE fail to open %d\n", (no - 1));
		return NG;
	}
	mixer_pcm_dfiles[1] = ald_getdata(DRIFILE_WAVE, (noR - 1));
	if (mixer_pcm_dfiles[1] == NULL) {
		//WARNING("DRIFILE_WAVE fail to open %d\n", (no - 1));
		ald_freedata(mixer_pcm_dfiles[0]);
		mixer_pcm_dfiles[0] = NULL;
		return NG;
	}
	
	mixer_pcm_chunks[0] = Mix_LoadWAV_RW(SDL_RWFromMem(mixer_pcm_dfiles[0]->data, mixer_pcm_dfiles[0]->size), 1);
	if(mixer_pcm_chunks[0] == NULL){
		ald_freedata(mixer_pcm_dfiles[1]);
		mixer_pcm_dfiles[1] = NULL;
		ald_freedata(mixer_pcm_dfiles[0]);
		mixer_pcm_dfiles[0] = NULL;
		return NG;
	}
	mixer_pcm_chunks[1] = Mix_LoadWAV_RW(SDL_RWFromMem(mixer_pcm_dfiles[1]->data, mixer_pcm_dfiles[1]->size), 1);
	if(mixer_pcm_chunks[1] == NULL){
		Mix_FreeChunk(mixer_pcm_chunks[0]);
		mixer_pcm_chunks[0] = NULL;
		ald_freedata(mixer_pcm_dfiles[1]);
		mixer_pcm_dfiles[1] = NULL;
		ald_freedata(mixer_pcm_dfiles[0]);
		mixer_pcm_dfiles[0] = NULL;
		return NG;
	}
	
	Mix_SetPanning(0, 255, 0);
	Mix_SetPanning(1, 0, 255);
	
	Mix_PlayChannel(0, mixer_pcm_chunks[0], (loop - 1));
	Mix_PlayChannel(1, mixer_pcm_chunks[1], (loop - 1));

	return OK;
}

/*
 * WAV の演奏停止 (command S?)
 *   msec: 止まるまでの時間(msec), 0の場合はすぐに止まる
 */
int mus_pcm_stop(int msec) {

	if (!audio_available) return NG;
	
	int i;
	for(i=0; i<2; i++){
		if(mixer_pcm_chunks[i]){
			Mix_HaltChannel(i);
			Mix_FreeChunk(mixer_pcm_chunks[i]);
			mixer_pcm_chunks[i] = NULL;
		}
		if(mixer_pcm_dfiles[i]){
			ald_freedata(mixer_pcm_dfiles[i]);
			mixer_pcm_dfiles[i] = NULL;
		}
	}

	return OK;
}

/*
 * WAV ファイルをメモリ上に載せる
 *   no  : ファイル番号( no >= 1)
 */
int mus_pcm_load(int no) {

	if (!audio_available) return NG;

	return OK;
}

/*
 * WAV の演奏状態の取得
 *   pos: 演奏時間を格納する場所(msec)
 *        停止している場合は 0 が入る
 *        loopしている場合は合計時間
 */
int mus_pcm_get_playposition(int *pos) {

	if (!audio_available) return NG;
	
	return OK;
}


/*
 * フェード開始
 *   device: フェードするデバイス(MIX_MAXTER/MIX_PCM/....)
 *   time  : 最終ボリュームまでに達する時間(msec)
 *   volume: 最終ボリューム
 *   stop:   フェード終了時に演奏をストップするかどうか？
 *           0: しない
 *           1: する
 */ 
int mus_mixer_fadeout_start(int device, int time, int volume, int stop) {

	if (!audio_available) return NG;
	
	return OK;
}

/*
 * 指定のデバイスが現在フェード中かどうかを調べる
 *   device: 指定デバイス
 *
 *   return: TRUE  -> フェード中
 *           FALSE -> フェード中でない
 */
boolean mus_mixer_fadeout_get_state(int device) {

	boolean bool = FALSE;

	return bool;
}

/*
 * 指定のデバイスのフェードを途中で止める
 *   device: 指定デバイス
 */
int mus_mixer_fadeout_stop(int device) {

	if (!audio_available) return NG;

	return OK;
}

/*
 * 指定のデバイスのミキサーレベルを取得する
 *   device: 指定デバイス
 *
 *   return: ミキサーレベル(0 - 100) (ゲーム内で設定された値)
 */
int mus_mixer_get_level(int device) {
	int lv = 0;
	
	if (!audio_available) return 0;

	return lv;
}

/*
 * 指定のフォーマットで再生可能かどうか調べる
 *   bit : 8 or 16 bit
 *   rate: frequency
 *   ch  : Mono or Stereo
 *   able: 可能かどうかの状態を受け取る場所
 */
int mus_pcm_check_ability(int bit, int rate, int ch, boolean *able) {
	if (!audio_available) {
		printf("not available\n");
		*able = FALSE;
		return NG;
	}
	
	*able = TRUE;
	return OK;
}

/*
 * 指定のチャンネルに wave file をロード
 *   ch : channel (0-127)
 *   num: ファイル番号 (1-65535)
 */
int mus_wav_load(int ch, int num) {

	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

/*
 * 指定のチャンネルから wave file を破棄
 *   ch : channel
 */
int mus_wav_unload(int ch) {
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

/*
 * WAV の演奏開始 (wavXXXX)
 *   ch  : 再生するチャンネル (0-127)
           (あらかじめ mus_wav_loadでloadしておく)
 *   loop: 繰り返し回数       (0の場合は無限, それ以外は１回のみ)
 */
int mus_wav_play(int ch, int loop) {

	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

/*
 * 指定のチャンネルのWAVの演奏停止 (wavXXX)
 *   ch: channel
 */
int mus_wav_stop(int ch) {
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

/*
 * 指定のチャンネルの演奏状態の取得
 *   ch: channel (0-127)
 *   
 *   return: 演奏時間(msec) 65535ms で飽和
 */
int mus_wav_get_playposition(int ch) {

	int ret = 0;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	
	return ret;
}

 
/*
 * 指定のチャンネルのWAVのフェード
 *   ch: channel(0-127)
 *   time  : 最終ボリュームまでに達する時間(msec)
 *   volume: 最終ボリューム
 *   stop  : フェード終了時に演奏をストップするかどうか？
 *             0: しない
 *             1: する
 */
int mus_wav_fadeout_start(int ch, int time, int volume, int stop) {

	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;

	return OK;
}

/*
 * 指定のチャンネルのフェードを途中で止める
 *   ch: channel (0-127)
 */
int mus_wav_fadeout_stop(int ch) {

	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

/*
 * 指定のチャンネルが現在フェード中かどうかを調べる
 *   ch: channel
 *
 *   return: TRUE  -> フェード中
 *           FALSE -> フェード中でない
 */
boolean mus_wav_fadeout_get_state(int ch) {

	boolean bool = FALSE;
	
	if (!audio_available) return FALSE;
	
	if (ch < 0 || ch > 128) return FALSE;
	
	
	return bool;
}


/*
 * 指定のチャンネルの再生が終了するまで待つ
 *   ch: channel (0-127)
 */
int mus_wav_waitend(int ch) {

	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;

	return OK;
}

/*
 * 指定のチャンネルで時間待ち
 *     再生していないならすぐに戻る。コマンドが発行された瞬間に演奏中で
 *     あれば、演奏が終っても指定時間経過するまで待つ。
 *   ch  : channel (0-127)
 *   time: 待ち時間(msec)
 */
int mus_wav_waittime(int ch, int time) {

	if (!audio_available) return NG;

	if (ch < 0 || ch > 128) return NG;
	

	return OK;
}

/*
 * PCM の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_pcm_get_state() {
	return audio_available;
}

/*
 * 指定のチャンネルのWAVデータの演奏時間の取得
 *   ch: channel
 *   
 *   return: 時間(msec) 65535ms で飽和
 */
int mus_wav_wavtime(int ch) {

	int ret = 0;
	
	if (!audio_available) return 0;
	
	if (ch < 0 || ch > 128) return NG;

	if (ret > 65565) ret = 65535;
	return ret;

}

/*
 * 指定の channel に WAVFILE をセット
 *   ch:    channel
 *   wfile: WAVFILE
 */
int mus_wav_sendfile(int ch, WAVFILE *wfile) {

	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

/*
 * 指定のチャンネルに wave file をLR反転してロード
 *   ch : channel (0-127)
 *   num: ファイル番号 (1-65535)
 */
int mus_wav_load_lrsw(int ch, int num) {

	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	return OK;
}

int mus_bgm_play(int no, int time, int vol) {

	if (!audio_available) return NG;
	
	return OK;
}

int mus_bgm_stop(int no, int time) {

	if (!audio_available) return NG;

	return OK;
}

int mus_bgm_fade(int no, int time, int vol) {

	if (!audio_available) return NG;

	return OK;
}

int mus_bgm_getpos(int no) {

	int ret = 0;
	
	if (!audio_available) return NG;
	
	if (ret > 65565) ret = 65535;
	
	return ret;
}

int mus_bgm_wait(int no, int timeout) {

	if (!audio_available) return NG;

	
	return OK;
}

int mus_bgm_waitpos(int no, int index) {

	if (!audio_available) return NG;

	return OK;
}

int mus_bgm_stopall(int time) {
	if (!audio_available) return NG;
	
	return OK;
}

int mus_bgm_getlength(int no) {

	int ret = 0;
	
	if (!audio_available) return NG;
	
	if (ret > 65565) ret = 65535;
	
	return ret;
}

int mus_vol_set_valance(int *vols, int num) {

	if (!audio_available) return NG;
	
	return OK;
}
