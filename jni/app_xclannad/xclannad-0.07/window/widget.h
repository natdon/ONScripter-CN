#ifndef __WIDGET_H__
#define __WIDGET_H__

#include<vector>
#include"font/font.h"
#include"font/text.h"
#include"event.h"
#include"picture.h"

#define TimeCursor WidTimeCursor
#define MouseCursor WidMouseCursor
#define Button WidButton
#define Scale WidScale
#define Label WidLabel
#define Dialog WidDialog
#define TextButton WidTextButton
#define Text WidText
#define AnmTime WidAnmTime
#define AnmMove WidAnmMove
#define AnmAlpha WidAnmAlpha
#define AnmPtnSolid WidAnmPtnSolid
#define AnmPtnAlpha WidAnmPtnAlpha

// namespace Widget {

struct TimeCursor : public Event::Time, PicWidget {
	int x,y,dx,dy, nptn;
	int old_time, count, interval;
	TimeCursor(Event::Container& container, int _interval, PicContainer* parent, const char* fname, int sx, int sy, int sdx, int sdy, int nptn, const Rect& r);
	void Elapsed(unsigned int current_time);
};

struct MouseCursor : public Event::Video, PicWidget {
	int x, y;
	Event::Container& container;
	MouseCursor(Event::Container& container, PicContainer* parent, const char* s, int x, int y, int w, int h);
	MouseCursor(Event::Container& container, PicContainer* parent, Surface* s, int x, int y, int w, int h);
	~MouseCursor();
	static bool Motionfunc(int x, int y, void* pointer);
};

struct Button : public Event::Video, PicWidget {
	int sx, sy, sdx, sdy, nptn;
	bool is_in;
	bool is_toggled;
	bool is_toggle_switch;
	Button(Event::Container& container, PicContainer* parent, const char* s, int sx, int sy, int sdx, int sdy, int nptn, const Rect& r, int _z);
	Button(Event::Container& container, PicContainer* parent, Surface* s, int sx, int sy, int sdx, int sdy, int nptn, const Rect& r, int _z);
	~Button();
	void Press(void);
	void Release(void);
	void Drag(int x_from, int y_from, int x_to, int y_to);
	void In(void);
	void Out(void);
	void Toggle(bool new_toggle);
	typedef void (*PressFunc)(void* pointer, Button* from);
	typedef void (*DragFunc)(int x_from, int y_from, int x_to, int y_to, void* pointer, Button* from);
	PressFunc press_func;
	void* press_pointer;
	DragFunc drag_func;
	void* drag_pointer;

	/* ·Ñ¾µ */
	void activate(void) { Event::Video::activate();}
	void deactivate(void) { Event::Video::deactivate();}
	void SetRegion(const Rect& new_rect) { Event::Video::SetRegion(new_rect);}
};
struct Scale : Event::Video, PicWidget {
private:
	Button* arrow_down, *arrow_up;
	Button* cursor;
	PicContainer* panel;
	Event::Container& container;
	PicContainer* parent;
	Color cursor_color;

	int mouse_x, mouse_y;
	enum {scale_max = 65536};
	int min, max;
	int value;
	int value_add;
	int value_dragstart;
	int cursor_width;
	bool is_vertical;

public:

	Scale(Event::Container& container, PicContainer* parent, const Rect& r_orig, const Color& cursor_color, bool _is_vertical);
	void InitCursor(int cursor_width_ratio); // 1024=max
	void SetRange(int min, int max);
	void SetValue(int value);
	int GetValue(void) const;
	typedef void (*ChangeFunc)(void* pointer, Scale* from);
	ChangeFunc change_func;
	void* change_pointer;
private:
	void Init(Rect r_orig);
	int CalcValue(void);
	void SetScaleValue(int value);

	// callback
	static void PressArrowDown(void* pointer, Button* from);
	static void PressArrowUp(void* pointer, Button* from);
	static void PressCursor(void* pointer, Button* from);
	static void DragCursor(int x_from, int y_from,int x, int y, void* pointer, Button* from);

	// ·Ñ¾µ¡§Event::Video
	void Press(void);
	void Motion(int x, int y);

	/* ·Ñ¾µ : PicWidget */
	void activate(void) { Event::Video::activate();}
	void deactivate(void) { Event::Video::deactivate();}
	void SetRegion(const Rect& new_rect) { Event::Video::SetRegion(new_rect);}
};

struct TextButton : public Button {
	enum Attribute {CENTER=1, REVERSE=2, NOPADDING=4};
	PicRoot& root;
	Surface* surface;
	Attribute attribute;
	int text_size;
	Color fore, pressed, back;
	TextButton(Event::Container& container, PicContainer* parent, const char* s, int text_size, Attribute attr, const Rect& r, int _z, const Color& fore, const Color& pressed, const Color& back);
	void SetText(const char* s, const Color& fore, const Color& pressed, const Color& back);
	void SetText(const char* s) {
		SetText(s, fore, pressed, back);
	}
	~TextButton();
};

struct Text : public Event::Video, Event::Time, PicWidget {
	typedef TextGlyphStream::iterator iterator;

private:
	Event::Container& event;
public:
	PicBase* pictext;
private:
	TimeCursor* cursor;
	Surface* surface;
	TextGlyphStream gstream;
	std::vector<int> bottom_pos; // ¹Ô¹â¤µ(height)¤ÎÎß·×ÃÍ
	XKFont::HorizLayout layout;
	int fontsize;

	iterator cur_pos;
	int line_number;
	Rect srcrect;
	int press_count;
	int scrolled_count;
	int scroll_height;
	bool window_activated;
	bool cursor_activated;

	int speed; // chars / sec or -1
	int wait_delay; // msec
	int old_time;
	int wait_starttime;

	int CalcScrollHeight(void);
	void DrawText(int& nChar);
	void Scrollup(int& nChar);
public:
	Text(Event::Container& container, PicContainer* parent, const Rect& r, const Rect& text_r, int fontsize);
	~Text();

	TextStream stream;
	enum {PREPARE, DRAW, WAIT, SCROLL, DRAW2, WAIT2} status;

	void Clear(void);
	void Start(void);
	void Flush(void);

	void Elapsed(unsigned int current_time);
	static bool Pressed(int x, int y, void* pointer);
	void activate(void);
	void deactivate(void);
	void SetSpeed(int new_speed);
	void SetWait(int new_wait);

	void SetCursor(TimeCursor* cursor);
};

extern void SetFont(const char* fontname);

struct Label : PicWidget{
private:
	Surface* surface;
	bool is_center;
	PicRoot& root;
	int text_size;
public:
	Label(PicContainer* parent, const Rect& r_orig, bool is_center=true, const char* text=0, int textsize = 26);
	~Label();
	void SetText(const char* text);
};

class Dialog : public Event::Video, PicWidget {
	Surface* surface_btn;
	Surface* surface_diag;
public:
	enum { WAIT, OK, CANCEL} status;
	Dialog(Event::Container& container, PicContainer* parent, const char* text, bool with_cancel);
	~Dialog();
	static void press_ok(void* pointer, Button* btn);
	static void press_cancel(void* pointer, Button* btn);
	static void DrawBox(Surface* s, const Rect& r);
	typedef void (*SetFunc)(void* pointer, Dialog* from);
	SetFunc set_func;
	void* set_pointer;
};

struct AnmTime : public Event::Time, PicAnm {
	enum { PLAYING=1, FINISHED=3 } status;
	unsigned int start_time;
	unsigned int total_time;
	int all_count;
	
	AnmTime(Event::Container& container, PicBase* _pic, int total_time, int all_count = 0);
	AnmTime(Event::Container& container, std::vector<PicBase*> _pic, int total_time, int all_count = 0);
	virtual ~AnmTime() {}
	void SetAllCount(int new_all_count) { all_count = new_all_count; }
	void SetTotalTime(int new_total) { total_time = new_total; }
	void Elapsed(unsigned int current_time);
	void Play(void) {
		start_time = 0;
		status = PLAYING;
	}

	virtual void Start(void) {};
	virtual void Exec(int count) = 0;
	virtual void Finish(void) {};
	void Abort(void);
	bool IsEnd(void);
};

struct AnmMove : public AnmTime {
	Rect from, to;
	AnmMove(Event::Container& container, PicBase* _pic, const Rect& to, int total_time);
	void Exec(int count);
};
#define ALPHA_MAX 255
struct AnmAlpha : public AnmTime {
	int from, to;
	unsigned char alpha; Rect alpha_r;
	AnmAlpha(Event::Container& container, PicBase* _pic,  int alpha_from, int alpha_to, int total_time);
	AnmAlpha(Event::Container& container, std::vector<PicBase*> _pic,  int alpha_from, int alpha_to, int total_time);
	void Start(void);
	void Exec(int count);
	void Finish(void);
};
struct AnmAlphaMove : public AnmTime {
	struct Ptn {
		Rect pos;
		Rect surface_pos;
		unsigned char alpha;
		unsigned int next_tick;
		Ptn(const Rect& _r, const Rect& _surface_r, unsigned char _a, unsigned int _n) :
			pos(_r), surface_pos(_surface_r), alpha(_a), next_tick(_n) {}
	};
	std::vector<Ptn> ptns;
	int cur_count;
	AnmAlphaMove(Event::Container& container, PicBase* _pic);
	void SetPtn(void);
	void Exec(int count);
	void Finish(void);
};
struct AnmPtnSolid : public AnmTime {
	AnmPtnSolid(Event::Container& container, PicBase* _pic, const unsigned char* ptn, const Rect& alpha_r, int total_time);
	~AnmPtnSolid() { delete[] alpha; }
	const unsigned char* ptn;
	int ptn_len;
	unsigned char* alpha;
	Rect alpha_r;
	
	void Start(void);
	void Exec(int count);
	void Finish(void);
};
struct AnmPtnAlpha : public AnmTime {
	AnmPtnAlpha(Event::Container& container, PicBase* _pic, const unsigned char* ptn, const Rect& alpha_r, int alpha_bandwidth, int total_time);
	~AnmPtnAlpha() { delete[] alpha; }
	const unsigned char* ptn;
	int ptn_len;
	int band;
	unsigned char* alpha;
	Rect alpha_r;
	void Start(void);
	void Exec(int count);
	void Finish(void);
};

// } /* end of namespace Widget */

#undef Text

#endif

