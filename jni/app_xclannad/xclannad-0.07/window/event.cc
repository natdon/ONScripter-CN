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

#include"SDL.h"
#include"event.h"
#include<vector>
#include<list>
#include<algorithm>
#include<iostream>
#include<sys/stat.h>

using namespace std;

#ifdef ENABLE_PATCH
extern SDL_Joystick *g_joystick;
#endif

extern bool save_req = false, load_req = false, grpdump_req = false; //  scn2k/scn2k_impl.cc: キーボードからセーブ・ロードできるように
extern bool pressAreq=false,pressFreq=false,pressDreq=false;
namespace Event {
/* Impl: struct Event::Video */

Video::Video(Container& container) : region(0, 0, 0, 0), z(0), parent(container) {
	activated = false;
	parent.Add(this);
}
Video::Video(Container& container, const Rect& init_rect) : region(init_rect), z(0), parent(container) {
	activated = false;
	parent.Add(this);
}
Video::Video(Container& container, const Rect& init_rect, int _z) : region(init_rect), z(_z), parent(container) {
	activated = false;
	parent.Add(this);
}
Video::~Video() {
	parent.Delete(this);
};
void Video::SetRegion(const Rect& new_rect) {
	region = new_rect;
}
void Video::SetZ(int new_z) {
	z = new_z;
}
void Video::activate(void) {
	activated = true;
}
void Video::deactivate(void) {
	activated = false;
}
inline int Video::point_in(int x, int y) {
	if (!activated) return -1;
	if (region.point_in(x,y)) return z;
	else return -1;
}

/* カーソルの動く順序：上、左の順に準位付け */
bool operator <(const Video& pos1, const Video& pos2) {
	if (pos1.region.ty < pos2.region.ty) return true;
	if (pos1.region.ty == pos2.region.ty) return pos1.region.lx < pos2.region.lx;
	if (pos1.region.by >= pos2.region.by) return pos1.region.lx <= pos2.region.lx;
	return false;
}


/* Impl: struct Event::Time */
Time::Time(Container& container) : wakeup_time(container.current_time), parent(container) {
	parent.Add(this);
}
Time::~Time() {
	parent.Delete(this);
}
/* Define: struct Event::ContainerImpl */

struct ContainerImplTime_Item {
	Time* instance;
	bool valid;
	bool operator ==(Time* const& to) const {
		return to == instance;
	}
	ContainerImplTime_Item(Time* _time) :
		instance(_time), valid(true) {
	}
};

class ContainerImplTime : private vector<ContainerImplTime_Item> {
public:
	ContainerImplTime(void);
	bool Exec(unsigned int current_time);
	void Add(Time* new_event);
	void Delete(Time* delete_event);
private:
	static vector<ContainerImplTime_Item> new_item;
	unsigned int prev_execed_time;
	static bool is_invalid(const_reference value) {
		return !value.valid;
	}
};

vector<ContainerImplTime_Item> ContainerImplTime::new_item;

ContainerImplTime::ContainerImplTime(void) {
	prev_execed_time = 0;
}
void ContainerImplTime::Add(Time* event) {
	ContainerImplTime_Item item(event);
	new_item.push_back(item);
}
void ContainerImplTime::Delete(Time* delete_event) {
	iterator it = find(begin(), end(), delete_event);
	if (it != end()) {
		it->valid = false;
		it->instance = 0;
		return;
	}
	it = find(new_item.begin(), new_item.end(), delete_event);
	if (it != end()) {
		it->valid = false;
		it->instance = 0;
		return;
	}
	return;
}
bool ContainerImplTime::Exec(unsigned int current_time) {
	if (current_time == Time::NEVER_WAKE) return true;
	// 呼び出しまでに作製されたitemを追加 
	insert(end(), new_item.begin(), new_item.end());
	new_item.clear();
	if (empty()) return true;
	if (current_time == Time::FRAME_UPDATE) { // ビデオフレームの更新時
		for (iterator it = begin(); it != end(); it++) {
			if (! it->valid) continue;
			
			unsigned tm = it->instance->Wakeup();
			if (tm == Time::FRAME_UPDATE) {
				it->instance->Elapsed(prev_execed_time);
			}
		}
	} else { // 時間変化時
		if (current_time < prev_execed_time) prev_execed_time = 0; /* 時間が一回りして０に戻ったとき */
		for (iterator it = begin(); it != end(); it++) {
			if (! it->valid) continue;
			unsigned tm = it->instance->Wakeup();
			if (tm >= prev_execed_time && tm < current_time) {
				it->instance->Elapsed(current_time);
			}
		}
		prev_execed_time = current_time;
	}
	// 処理中に削除された item を実際に削除
	erase(remove_if(begin(), end(), is_invalid), end());
	return true;
}


class ContainerImplVideo : private vector<Video*> {
public:
	bool Exec(void);

	ContainerImplVideo(void);
	~ContainerImplVideo();
	void Add(Video* item);
	void Delete(Video* item);
	void RegisterGlobalMotionFunc(Container::motionfunc, void* pointer);
	void DeleteGlobalMotionFunc(Container::motionfunc, void* pointer);
	void RegisterGlobalPressFunc(Container::motionfunc, void* pointer);
	void DeleteGlobalPressFunc(Container::motionfunc, void* pointer);
private:
	struct Motionfunc {
		Container::motionfunc func;
		void* pointer;
		bool operator ==(const Motionfunc& m) const { return func == m.func && pointer == m.pointer;}
	};
	list<Motionfunc> motion_vec;
	list<Motionfunc> press_vec;
	typedef list<Motionfunc>::iterator MotionIterator;
	bool is_sorted;
public:
	int button_pressed;
	int button_released;
	int mouse_x, mouse_y;
	int new_mouse_x, new_mouse_y;
private:
	void SetChanged(void);
	static bool SortLess(const Video* pos1, const Video* pos2) {
		return pos1 < pos2;
	}
	void Sort(void);
	void Motion(int x, int y); // mouse motion
	void Press(void);
	void TakeScreenshot(void);
	iterator cur_pos;
	Video* cur_item; // 現在のフォーカス位置
	int cur_pressed_x, cur_pressed_y;
};

void ContainerImplVideo::SetChanged(void) {
	if (is_sorted) {
		if (cur_item) {
			cur_pos = find(begin(), end(), cur_item);
			if (cur_pos == end()) cur_item = 0;
		}
		is_sorted = false;
	}
}

void ContainerImplVideo::Sort(void) {
	sort(begin(), end(), SortLess);
	if (cur_item) {
		cur_pos = lower_bound(begin(), end(), cur_item, SortLess);
	} else {
		cur_pos = end();
	}
	is_sorted = true;
}

ContainerImplVideo::ContainerImplVideo(void) {
	is_sorted = false;
	button_pressed = 0;
	button_released = 0;
	cur_item = 0;
	mouse_x = 0; mouse_y = 0;
	new_mouse_x = 0; new_mouse_y = 0;
}
ContainerImplVideo::~ContainerImplVideo(void) {
};
void ContainerImplVideo::Add(Video* event) {
	push_back(event);
	SetChanged();
}
void ContainerImplVideo::Delete(Video* delete_event) {
	iterator it = find(begin(), end(), delete_event);
	if (it != end()) {
		erase(it);
		SetChanged();
	} else {
		fprintf(stderr,"\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		fprintf(stderr,"X  ContainerImplVideo: Cannot delete node %x\n",delete_event);
		fprintf(stderr,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n\n");
		fprintf(stderr,"vector from:\n");
		for(it=begin(); it!=end(); it++) {
			fprintf(stderr,"%x, ",*it);
		}
		fprintf(stderr,"\n");
	}
	if (delete_event == cur_item) {
		cur_pos = end();
		cur_item = 0;
		Motion(mouse_x, mouse_y);
	}
	return;
}
void ContainerImplVideo::RegisterGlobalMotionFunc(Container::motionfunc func, void* pointer) {
	Motionfunc f;
	f.func = func;
	f.pointer = pointer;
	if (find(motion_vec.begin(), motion_vec.end(), f) == motion_vec.end()) {
		motion_vec.push_back(f);
	}
}
void ContainerImplVideo::DeleteGlobalMotionFunc(Container::motionfunc func, void* pointer) {
	Motionfunc f;
	f.func = func;
	f.pointer = pointer;
	list<Motionfunc>::iterator it = find(motion_vec.begin(), motion_vec.end(), f);
	if (it != motion_vec.end())
		motion_vec.erase(it);
	return;
}
void ContainerImplVideo::RegisterGlobalPressFunc(Container::motionfunc func, void* pointer) {
	Motionfunc f;
	f.func = func;
	f.pointer = pointer;
	if (find(press_vec.begin(), press_vec.end(), f) == press_vec.end()) {
		press_vec.push_back(f);
	}
}
void ContainerImplVideo::DeleteGlobalPressFunc(Container::motionfunc func, void* pointer) {
	Motionfunc f;
	f.func = func;
	f.pointer = pointer;
	list<Motionfunc>::iterator it = find(press_vec.begin(), press_vec.end(), f);
	if (it != press_vec.end())
		press_vec.erase(it);
	return;
}
void ContainerImplVideo::Motion(int x, int y) {
	mouse_x = x; mouse_y = y;
	MotionIterator mit;
	for (mit=motion_vec.begin(); mit != motion_vec.end();) {
		MotionIterator mit_next = mit;
		mit_next++;
		if (!(*mit->func)(x, y, mit->pointer)) motion_vec.erase(mit);
		mit = mit_next;
	
	}
	
	/* @@@ ドラッグ処理とマウスを押す処理のバッティングで「二回ボタンを押さないと云々」関連のバグの可能性あり */
	if (button_pressed & (1<<MOUSE_LEFT)) {
		if (cur_item) cur_item->Drag(cur_pressed_x, cur_pressed_y, x, y);
		return;
	}
	if (cur_item) cur_item->Motion(x,y);
	int z = -1; iterator z_it;
	iterator it;
	for (it = begin(); it != end(); it++) {
		int new_z = (*it)->point_in(x, y);
		if (z < new_z) {
			z = new_z;
			z_it = it;
		}
	}
	if (z != -1) {
		if (cur_item == *z_it) return;
		if (cur_item) cur_item->Out();
		cur_pos = z_it;
		cur_item = *z_it;
		cur_item->In();
		return;
	} else {
		if (cur_item) cur_item->Out();
		cur_pos = end();
		cur_item = 0;
	}
	return;
}

void ContainerImplVideo::Press(void) {
	if (cur_item) {
		cur_pressed_x = mouse_x;
		cur_pressed_y = mouse_y;
		cur_item->Press();
		return;
	}
	MotionIterator mit;
	for (mit=press_vec.begin(); mit != press_vec.end(); ) {
		MotionIterator mit_next = mit;
		mit_next++;
		if (!(*mit->func)(mouse_x, mouse_y, mit->pointer)) {
			press_vec.erase(mit);
		}
		mit = mit_next;
	}
}
void ContainerImplVideo::TakeScreenshot(void) {
	int n=0;
	char filename[1024];
	struct stat buffer;
	for(n=0; n<9999; n++) {
		// XXX: put screenshots in a seperate dir?
		sprintf(filename, "xclannad_%04i.bmp", n);
		if(stat(filename, &buffer) == -1) break;
	}
	SDL_SaveBMP(SDL_GetVideoSurface(), filename);
}
#ifdef ENABLE_PATCH
static SDLKey transJoystickButton(Uint8 button)
{
#if defined(PSP)
    SDLKey button_map[] = {
		SDLK_ESCAPE, /* TRIANGLE */
		SDLK_RETURN, /* CIRCLE   */
		SDLK_SPACE,  /* CROSS    */
		SDLK_LSHIFT, /* SQUARE   */
		SDLK_s,      /* LTRIGGER */
		SDLK_l,      /* RTRIGGER */
		SDLK_DOWN,   /* DOWN     */
		SDLK_LEFT,   /* LEFT     */
		SDLK_UP,     /* UP       */
		SDLK_RIGHT,  /* RIGHT    */
		SDLK_0,      /* SELECT   */
		SDLK_a,      /* START    */
		SDLK_UNKNOWN,/* HOME     */ /* kernel mode only */
		SDLK_UNKNOWN,/* HOLD     */};
    return button_map[button];
#endif
	return SDLK_UNKNOWN;
}
#endif
bool ContainerImplVideo::Exec(void) {

	bool is_mouse_motion = false;
	int motion_x = 0, motion_y = 0;
	SDL_Event event;

	SDL_PumpEvents();
	while(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS) == 1) {
		switch(event.type) {
		case SDL_QUIT: return false; // @@@ なにかやらないと
		case SDL_ACTIVEEVENT: // なにもしない
			// cout<<"active : gain "<<int(event.active.gain)<<", state "<<int(event.active.state)<<endl;
			break;
#ifdef ENABLE_PATCH
		case SDL_JOYBUTTONDOWN:
            event.key.keysym.sym = transJoystickButton(event.jbutton.button);
#endif
		case SDL_KEYDOWN:
			if (!is_sorted) Sort();
			switch(event.key.keysym.sym) {
			case SDLK_F12:
			case SDLK_PRINT:
			case SDLK_p:  // for Zaurus
				TakeScreenshot();
				break;
			// Some window managers (eg enlightenment) use Alt-Enter for
			// themselves, F11 is a good alternative
			case SDLK_F11:
				SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
				break;
			case SDLK_RETURN:
				if (SDL_GetModState() & KMOD_ALT) {
					SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
					break;
				}
#ifdef ENABLE_PATCH
				button_pressed |= (1<<MOUSE_LEFT);
#endif
			case SDLK_SPACE:
				Press();
				break;
			case SDLK_TAB: // move to next widget
				if (cur_pos != end())  cur_pos++;
				if (cur_pos == end()) cur_pos = begin();
				if (cur_pos != end())  {
					cur_item = *cur_pos;
					cur_item->In();
				} else {
					cur_item = 0;
				}
				break;
			case SDLK_LEFT: if (cur_pos != end()) (*cur_pos)->KeyLeft(); break;
			case SDLK_RIGHT:if (cur_pos != end()) (*cur_pos)->KeyRight(); break;
#ifdef ENABLE_PATCH
			case SDLK_LCTRL:  case SDLK_RCTRL:
#endif
			case SDLK_LSHIFT: case SDLK_RSHIFT: button_pressed |= (1<<KEY_SHIFT); break;
			case SDLK_ESCAPE: button_pressed |= (1<<MOUSE_RIGHT); break; /* for Zaurus */
			case SDLK_s: save_req = true; break;
			case SDLK_l: load_req = true; break;
			case SDLK_g: grpdump_req = true; break;
			case SDLK_a: pressAreq = true; break;
			case SDLK_d: pressDreq = true; break;
			case SDLK_f: pressFreq = true; break;
			default: break;
			}
			break;
#ifdef ENABLE_PATCH
		case SDL_JOYBUTTONUP:
            event.key.keysym.sym = transJoystickButton(event.jbutton.button);
#endif
		case SDL_KEYUP:
			// cout << "keyup which "<<int(event.key.which)<<", sym "<<int(event.key.keysym.sym)<<endl;
			switch(event.key.keysym.sym) {
			case SDLK_RETURN:
				button_pressed &= ~(1<<MOUSE_LEFT); button_released |= 1<<MOUSE_LEFT;
			case SDLK_SPACE:
				if (cur_item) cur_item->Release();
#ifdef ENABLE_PATCH
			case SDLK_LCTRL:  case SDLK_RCTRL:
#endif
			case SDLK_LSHIFT: case SDLK_RSHIFT: button_pressed &= ~(1<<KEY_SHIFT); button_released |= 1<<KEY_SHIFT; break;
			case SDLK_ESCAPE: button_pressed &= ~(1<<MOUSE_RIGHT); button_released |= 1<<MOUSE_RIGHT; break; /* for Zaurus */
			default: break;
			}
			break;
		case  SDL_MOUSEMOTION:
			motion_x = event.motion.x;
			motion_y = event.motion.y;
			is_mouse_motion = true;
			// Motion(event.motion.x, event.motion.y);
			// cout<< "motion which "<<int(event.motion.which)<<
			//	"x "<<event.motion.x << "y "<<event.motion.y<<endl;
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == 1) {
				Motion(event.button.x, event.button.y);
				is_mouse_motion = false;
				if (cur_item) cur_item->Release();
			}
			switch(event.button.button) {
			case 1: button_pressed &= ~(1<<MOUSE_LEFT); button_released |= 1<<MOUSE_LEFT; break;
			case 2: button_pressed &= ~(1<<MOUSE_MIDDLE); button_released |= 1<<MOUSE_MIDDLE; break;
			case 3: button_pressed &= ~(1<<MOUSE_RIGHT); button_released |= 1<<MOUSE_RIGHT; break;
			case 4: button_pressed &= ~(1<<MOUSE_UP); button_released |= 1<<MOUSE_UP; break;
			case 5: button_pressed &= ~(1<<MOUSE_DOWN); button_released |= 1<<MOUSE_DOWN; break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == 1) {
				Motion(event.button.x, event.button.y);
				is_mouse_motion = false;
				Press();
			}
			switch(event.button.button) {
			case 1: button_pressed |= (1<<MOUSE_LEFT); break;
			case 2: button_pressed |= (1<<MOUSE_MIDDLE); break;
			case 3: button_pressed |= (1<<MOUSE_RIGHT); break;
			case 4: button_pressed |= (1<<MOUSE_UP); break;
			case 5: button_pressed |= (1<<MOUSE_DOWN); break;
			}
			// cout << "mouse which "<<int(event.button.which)<<"button "<<int(event.button.button)<<
			//	"state "<<int(event.button.state)<<"x "<<event.button.x << "y "<<event.button.y<<endl;
			break;
		case SDL_VIDEOEXPOSE: // redraw の必要がある？
			// cout<<"expose."<<endl;
			break;
		default:
			break;
		}
	}
	// Motion 呼び出しは一回だけ
	if (is_mouse_motion)
		Motion(motion_x, motion_y);
	return true;
};
/* Impl: struct Event::Container */
Container::Container(void) {
	pimpl_video = new ContainerImplVideo;
	try {
		pimpl_time = new ContainerImplTime;
	} catch(...) {
		delete pimpl_video;
		throw;
	}
	button_pressed = 0;
	current_time = 0;
	int i; for (i=0; i<BUTTON_MAX; i++) button_presscount[i] = 0;
	return;
}
Container::~Container(void) {
	delete pimpl_video;
	delete pimpl_time;
}
void Container::Add(Video* item) {
	pimpl_video->Add(item);
}
void Container::Delete(Video* item) {
	pimpl_video->Delete(item);
}
void Container::Add(Time* item) {
	pimpl_time->Add(item);
}
void Container::Delete(Time* item) {
	pimpl_time->Delete(item);
}
void Container::RegisterGlobalMotionFunc(Container::motionfunc f, void* pointer) {
	pimpl_video->RegisterGlobalMotionFunc(f, pointer);
}
void Container::DeleteGlobalMotionFunc(Container::motionfunc f, void* pointer) {
	pimpl_video->DeleteGlobalMotionFunc(f, pointer);
}
void Container::RegisterGlobalPressFunc(Container::motionfunc f, void* pointer) {
	pimpl_video->RegisterGlobalPressFunc(f, pointer);
}
void Container::DeleteGlobalPressFunc(Container::motionfunc f, void* pointer) {
	pimpl_video->DeleteGlobalPressFunc(f, pointer);
}
bool Container::Exec(unsigned int time) {
	current_time = time;
	bool ret = true;
	ret = ret && pimpl_video->Exec();
	ret = ret && pimpl_time->Exec(time);
	int i; int mask = 1;
	int new_button_pressed = pimpl_video->button_pressed;
	for (i=0; i<BUTTON_MAX; i++) {
		if (pimpl_video->button_released&mask) {
			button_presscount[i]++;
		}
		mask <<= 1;
	}
	pimpl_video->button_released = 0;
	button_pressed = pimpl_video->button_pressed;
	return ret;
}

void Container::MousePos(int& x, int& y) {
	x = pimpl_video->mouse_x;
	y = pimpl_video->mouse_y;
}

bool Container::pressed(int mask) {
	if (mask < 0 || mask >= BUTTON_MAX) return 0;
	return (button_pressed & (1<<mask)) != 0;
}
bool Container::presscount(int mask) {
	if (mask < 0 || mask >= BUTTON_MAX) return 0;
	int count = button_presscount[mask];
	button_presscount[mask] = 0;
	return count;
}

}; /* end of namespace Container */

// 問題：
// z 軸と xy 軸の相互干渉；高速化
// 移動するウィジット描画の高速化
// キャッシュ
// 文字列の一部のみ更新の高速化
// 「階層 z で x なる領域無効化、y なる領域生成」で良い？＞Expose
