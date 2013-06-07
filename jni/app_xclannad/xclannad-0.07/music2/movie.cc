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

/*
 * movie.cc  smpeg による動画再生
 *
*/

#include<stdio.h>
#include"music.h"
#include<SDL.h>
#include<SDL_mixer.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include"system/file.h"
#include"window/system.h"

#if USE_SMPEG
#include<smpeg/smpeg.h>

static SMPEG* smpeg_handle = 0;
const char* FindMovieFile(const char* path);
void MuSys::PlayMovie(const char* path, int x1, int y1, int x2, int y2, int loop_count) {
	if (!pcm_enable) return;
	FinalizeMusic();
	SMPEG_Info info;
	const char* find_path = FindMovieFile(path);
	if (find_path == 0) return;
	smpeg_handle = SMPEG_new(find_path, &info, true);
	//SMPEG_enableaudio(smpeg_handle,  true);
	//SMPEG_enablevideo(smpeg_handle,  true);
	SMPEG_enableaudio(smpeg_handle,  true);
	SMPEG_enablevideo(smpeg_handle,  true);
	SDL_Surface* surface = SDL_GetVideoSurface();
	System::Main::DisableVideo();
	SMPEG_setdisplay(smpeg_handle,surface,0,0);
	// if (loop_c > 1) SMPEG_loop(smpeg_handle, true);
	//if (x1 != 0 || x2 != 0) SMPEG_setdisplayregion(smpeg_handle,x1, y1, x2-x1, y1-y2);
	SMPEG_play(smpeg_handle);
#if 0
	while(SMPEG_status(smpeg_handle) != SMPEG_PLAYING) {
           	SDL_Delay( 10 );
	}
#endif
	return;
err:
	StopMovie();
	return;
}
const char* FindMovieFile(const char* path) {
	ARCINFO* info = file_searcher.Find(FILESEARCH::MOV,path,"avi");
	if (info == 0) 
		info = file_searcher.Find(FILESEARCH::MOV,path,"mpg");
	if (info == 0) return 0;
	const char* file = info->Path();
	delete info;
	return file;
}

void MuSys::StopMovie(void) {
	if (smpeg_handle) {
		if (SMPEG_status(smpeg_handle) == SMPEG_PLAYING)
			SMPEG_stop(smpeg_handle);
		while(SMPEG_status(smpeg_handle) == SMPEG_PLAYING) {
            		SDL_Delay( 10 );
		}
		SMPEG_delete(smpeg_handle);
	}
	smpeg_handle = 0;
	System::Main::EnableVideo();
	InitMusic();
}
bool MuSys::IsStopMovie(void) {
	if (!smpeg_handle) return true;
	if (SMPEG_status(smpeg_handle) == SMPEG_PLAYING) return false;
	return true;
}

#else /* USE_SMPEG */
void MuSys::PlayMovie(const char* path, int x1, int y1, int x2, int y2, int loop_count) {
}
void MuSys::StopMovie(void) {
}
bool MuSys::IsStopMovie(void) {
	return true;
}
#endif
