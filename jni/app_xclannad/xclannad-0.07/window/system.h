#ifndef __SYSTEM__
#define __SYSTEM__

#include<SDL.h>
#include"event.h"
#include"picture.h"
#include"widget.h"

namespace System {
struct Main {
	int framerate;
	static int event_filter(const SDL_Event* event);
	static bool is_exit;
	static bool is_video_update;
	static Main* instance;
	WidMouseCursor* cursor;
public:
	Event::Container event;
	PicRoot root;
	Main(void);
	~Main();
	void Mainloop(void);
	static void Quit(void);
	static void DisableVideo(void);
	static void EnableVideo(void);
#define DEFAULT_MOUSECURSOR (Surface*)0xffff0000
	static void SetCursor(Surface* s, const Rect& r);
};
};

#endif /* __SYSTEM__ */
