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

#include<SDL.h>
#include"system.h"
#include<iostream>
#include<stdio.h>

using namespace std;

// void SDL_SetEventFilter(SDL_EventFilter filter);
// typedef int (*SDL_EventFilter)(const SDL_Event *event);
namespace System {

Main* Main::instance = 0;

int Main::event_filter(const SDL_Event* event) {
	return 1; /* throw all event */
}

Main::Main(void) {
	instance = this;
	framerate = 20;
	cursor = 0;
}
Main::~Main() {
	if (cursor) delete cursor;
}
void Main::Quit(void) {
	is_exit = true;
}
void Main::EnableVideo(void) {
	is_video_update = true;
}
void Main::DisableVideo(void) {
	is_video_update = false;
}
bool Main::is_exit = false;
bool Main::is_video_update = true;

void Main::Mainloop(void) {
	SDL_SetEventFilter(&event_filter);
	Uint32 old_time = 0;
	while(!is_exit) {
		Uint32 start_time = SDL_GetTicks();
		if (! event.Exec(start_time)) break;
		if (start_time - old_time > 1000/framerate) {
			if (is_video_update) root.ExecUpdate();
			event.Exec(Event::Time::FRAME_UPDATE);
			cout.flush();
			old_time = start_time;
		}

// 問題：
// z 軸と xy 軸の相互干渉；高速化
// 移動するウィジット描画の高速化
// キャッシュ
// 文字列の一部のみ更新の高速化
// 「階層 z で x なる領域無効化、y なる領域生成」で良い？＞Expose
/*
		Uint32 end_time = SDL_GetTicks();
		Uint32 delay = (end_time-start_time);
		if(delay < 1000/framerate) SDL_Delay(1000/framerate - delay);
		else SDL_Delay(0);
*/
		SDL_Delay(0);
	};
}

void Main::SetCursor(Surface* s, const Rect& r) {
	if (instance == 0) return;
	if (instance->cursor) delete instance->cursor;
	if (s == 0) { // カーソル消去
		instance->cursor = 0;
	} else if (s == DEFAULT_MOUSECURSOR) {
		instance->cursor = 0;
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		instance->cursor = new WidMouseCursor(instance->event, instance->root.root, s, r.lx, r.ty, r.width(), r.height());
		instance->cursor->show();
		SDL_ShowCursor(SDL_DISABLE);
	}
}
}
