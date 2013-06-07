#ifndef __EVENT__
#define __EVENT__

#include"rect.h"
#include"SDL.h"

namespace Event {

class Video;
class Time;
class Container;

/*
** マウスの press, ドラッグ、in/out を検出できる
** focus がある時の left/right/space(==press) のキーを判別できる
*/
struct Video {
	virtual void Press(void) {}
	virtual void Release(void) {}
	virtual void Drag(int x_from, int y_from, int x_to, int y_to) {}
	virtual void Motion(int x, int y) {}
	virtual void In(void) {}
	virtual void Out(void) {}
	virtual void KeyLeft(void) {}
	virtual void KeyRight(void) {}

	int point_in(int x, int y); /* z or -1 を返す。大きいほど高いところにある */

	Video(Container& container);
	Video(Container& container, const Rect& init_rect);
	Video(Container& container, const Rect& init_rect, int z);
	void SetRegion(const Rect& new_rect);
	void SetZ(int new_z);
	void activate(void);
	void deactivate(void);
	virtual ~Video();

	Rect Region(void) const { return region;}
private:
	Rect region;
	int z;
	Container& parent;
	bool activated;
	friend bool operator <(const Video& position1, const Video& position2);
};

struct Time {
	enum { NEVER_WAKE = 0xffffffff, FRAME_UPDATE = 0xfffffffe};
	virtual void Elapsed(unsigned int current_time) {wakeup_time = NEVER_WAKE; }; /* next: never elapsed */
	void SetWakeup(unsigned int new_wakeup_time) { wakeup_time = new_wakeup_time; }
	unsigned  Wakeup(void) const { return wakeup_time; }

	Time(Container& container);
	~Time();
private:
	unsigned int wakeup_time;
	Container& parent;
};

struct Container {
#define MOUSE_LEFT 0
#define MOUSE_MIDDLE 1
#define MOUSE_RIGHT 2
#define MOUSE_UP 3
#define MOUSE_DOWN 4
#define KEY_SHIFT 10
#define BUTTON_MAX 32
	int button_pressed;
	int button_presscount[BUTTON_MAX];
	int current_time;

	void MousePos(int& x, int& y);
	bool Exec(unsigned int current_time);

	void Add(Video* item);
	void Delete(Video* item);

	void Add(Time* item);
	void Delete(Time* item);

	typedef bool (*motionfunc)(int x, int y, void* pointer);
	void RegisterGlobalMotionFunc(motionfunc, void* pointer); // マウスの移動のたびに呼び出される関数を登録する
	void DeleteGlobalMotionFunc(motionfunc, void* pointer);
	void RegisterGlobalPressFunc(motionfunc, void* pointer); // マウスのクリックのたびに呼び出される関数を登録する
	void DeleteGlobalPressFunc(motionfunc, void* pointer);

	Container(void);
	~Container(void);
	bool pressed(int mask);
	bool presscount(int mask);
private:
	class ContainerImplVideo* pimpl_video;
	class ContainerImplTime* pimpl_time;
};

}; /* end of namespace Event */


#endif
