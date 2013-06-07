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

#include"widget.h"
#include<algorithm>
#include<map>
#include<string>

Rect DSurfaceRenderText(TextGlyphStream::iterator start, TextGlyphStream::iterator end, const Rect& srcrect,
	Surface* dst, const Rect& dstrect);
void DSurfaceFill(Surface* src, const Rect& rect, int r, int g, int b, int a = 0xff);
void DSurfaceMove(Surface* src, const Rect& srcrect, Surface* dst_o, const Rect& dstrect);

//static char* wdefault_font_orig = "msgothic.ttc;times.ttf;";
static std::map<int, XKFont::HorizLayout*> size_to_layout;
static char* wdefault_font_orig = "times.ttf;msgothic.ttc";
static std::string wdefault_font = wdefault_font_orig;

void SetFont(const char* font) {
	if (font == 0) return;
	std::map<int,XKFont::HorizLayout*>::iterator it;
	for (it=size_to_layout.begin(); it != size_to_layout.end(); it++) {
		delete it->second;
	}
	size_to_layout.clear();
	wdefault_font = font;
}
// namespace Widget {
#define TimeCursor WidTimeCursor
#define MouseCursor WidMouseCursor
#define Button WidButton
#define Scale WidScale
#define TextButton WidTextButton
#define Text WidText
#define AnmTime WidAnmTime
#define AnmMove WidAnmMove
#define AnmAlpha WidAnmAlpha
#define AnmPtnSolid WidAnmPtnSolid
#define AnmPtnAlpha WidAnmPtnAlpha

XKFont::HorizLayout* DefaultLayout(int text_size) {
	if (size_to_layout.find(text_size) == size_to_layout.end()) {
		size_to_layout[text_size] = new XKFont::HorizLayout(wdefault_font.c_str(), text_size);
	}
	return size_to_layout[text_size];
}

void PicWidget::activate(void) {
}
void PicWidget::deactivate(void) {
}
void PicWidget::SetRegion(const Rect& apos) {
}
void PicWidget::show(void) {
	Pic()->show();
}
void PicWidget::hide(void) {
	Pic()->hide();
}
void PicWidget::show_all(void) {
	Pic()->show_all();
}

TimeCursor::TimeCursor(Event::Container& container, int _interval, PicContainer* parent, const char* s, int _sx, int _sy, int _sdx, int _sdy, int _nptn, const Rect& r) :
	Time(container) {
	interval = _interval;
	if (interval < 0) interval = 100;
	nptn = _nptn;
	if (nptn < 0) nptn = 1;
	count = 0; old_time = 0;

	x = _sx; y = _sy; dx = _sdx; dy = _sdy;
	SetPic(parent->create_leaf(r, PicBase::CACHE_BACK));
	Pic()->SetSurface(s, _sx, _sy);
};
void TimeCursor::Elapsed(unsigned int current_time) {
	int move = (current_time-old_time)/interval;
	if (move) {
		old_time += move*interval;
		count += move;
		count %= nptn;
		Pic()->SetSurfacePos(x + count*dx, y + count*dy);
	}
	if (current_time > old_time+interval) SetWakeup(current_time);
	else SetWakeup(old_time+interval);
}

MouseCursor::MouseCursor(Event::Container& _container, PicContainer* parent, const char* s, int x, int y, int w, int h) :
	Event::Video(_container), container(_container) {
	int sx, sy;
	_container.MousePos(sx, sy);
	SetPic(parent->create_leaf(Rect(sx, sy, sx+w, sy+h), 0));
	Pic()->SetSurface(s, x, y);
	x = 0; y = 0;
	container.RegisterGlobalMotionFunc(&Motionfunc, (void*)this);
}
MouseCursor::MouseCursor(Event::Container& _container, PicContainer* parent, Surface* s, int x, int y, int w, int h) :
	Event::Video(_container), container(_container) {
	int sx, sy;
	_container.MousePos(sx, sy);
	SetPic(parent->create_leaf(Rect(sx, sy, sx+w, sy+h), 0));
	Pic()->SetSurface(s, x, y);
	x = 0; y = 0;
	container.RegisterGlobalMotionFunc(&Motionfunc, (void*)this);
}

MouseCursor::~MouseCursor() {
	container.DeleteGlobalMotionFunc(&Motionfunc, (void*)this);
}

bool MouseCursor::Motionfunc(int x, int y, void* pointer) {
	MouseCursor* _this = (MouseCursor*)pointer;
	// 左上がカーソルポイントの場合
	// _this->Pic()->Move(x,y);
	// 左下がカーソルポイントの場合
	_this->Pic()->Move(x,y-_this->Pic()->Height());
	return true;
}

Button::Button(Event::Container& container, PicContainer* parent, const char* s, int _sx, int _sy, int _sdx, int _sdy, int _nptn, const Rect& r, int _z) : sx(_sx), sy(_sy), sdx(_sdx), sdy(_sdy), nptn(_nptn) ,Event::Video(container,r, _z) {
	SetPic(parent->create_leaf(r, 0));
	Pic()->SetSurface(s, _sx, _sy);
	show();
	is_in = false;
	is_toggled = false;
	press_func = 0;
	press_pointer = 0;
	drag_func = 0;
	drag_pointer = 0;
	is_toggle_switch = false;
}
Button::Button(Event::Container& container, PicContainer* parent, Surface* s, int _sx, int _sy, int _sdx, int _sdy, int _nptn, const Rect& r, int _z) : sx(_sx), sy(_sy), sdx(_sdx), sdy(_sdy), nptn(_nptn) ,Event::Video(container,r, _z) {
	SetPic(parent->create_leaf(r, 0));
	Pic()->SetSurface(s, _sx, _sy);
	show();
	is_in = false;
	is_toggled = false;
	press_func = 0;
	press_pointer = 0;
	drag_func = 0;
	drag_pointer = 0;
	is_toggle_switch = false;
}
Button::~Button() {
}
void Button::In(void) {
	is_in = true;
	if (nptn > 1)
		if (! is_toggled)
			Pic()->SetSurfacePos(sx+sdx, sy+sdy);
}
void Button::Out(void) {
	is_in = false;
	if (!is_toggled)
		Pic()->SetSurfacePos(sx, sy);
}
void Button::Toggle(bool new_toggle) {
	if (is_toggled == new_toggle) {
		return;
	}
	is_toggled = new_toggle;
	// if (is_in) return; // is_in に関わらずウィジットの表示を変更することにする
	if (is_toggled) {
		if (nptn > 2)
			Pic()->SetSurfacePos(sx+sdx*2, sy+sdy*2);
		else if (nptn > 1)
			Pic()->SetSurfacePos(sx+sdx, sy+sdy);
		else
			Pic()->SetSurfacePos(sx, sy);
	} else {
		Pic()->SetSurfacePos(sx, sy);
	}
}
void Button::Press(void) {
	is_in = true;
	if (is_toggled) ;
	else if (nptn > 2)
		Pic()->SetSurfacePos(sx+sdx*2, sy+sdy*2);
	else if (nptn > 1)
		Pic()->SetSurfacePos(sx+sdx, sy+sdy);
	if (press_func) press_func(press_pointer, this);
	if (is_toggle_switch) Toggle(!is_toggled);
}
void Button::Release(void) {
	if (is_toggled) ;
	else if (nptn > 1 && is_in)
		Pic()->SetSurfacePos(sx+sdx, sy+sdy);
	else if (nptn > 1)
		Pic()->SetSurfacePos(sx, sy);
}
void Button::Drag(int x_from, int y_from, int x_to, int y_to) {
	if (drag_func) drag_func(x_from,y_from,x_to, y_to,drag_pointer, this);
}

Scale::Scale(Event::Container& _container, PicContainer* _parent, const Rect& r_orig, const Color& _color, bool _is_vertical) :
	Event::Video(_container,Rect(0,0), 1),
	container(_container), parent(_parent), cursor_color(_color),
	mouse_x(0), mouse_y(0), max(0), min(0),
	value(0), value_add(0), value_dragstart(0), is_vertical(_is_vertical),
	change_func(0), change_pointer(0) {

	arrow_down = 0;
	arrow_up = 0;
	cursor = 0;
	panel = 0;

	Init(r_orig);
}

extern char* create_button(int number, int& width, int& height, int r, int g, int b);
extern char* create_box(int& width, int& height, int r, int g, int b);
void Scale::Init(Rect r_orig) {
	int r=cursor_color.r, g=cursor_color.g, b=cursor_color.b;
	// 矢印
	int arrow_width = -1;
	cursor_width = -1;
	char* button1;
	char* button2;
	if (is_vertical) {
		// 矢印に必要な領域確保
		int arrow_height = r_orig.width();
		button1 = create_button(2, arrow_height, arrow_width, r, g, b);
		button2 = create_button(3, arrow_height, arrow_width, r, g, b);
		if (r_orig.height() < arrow_width*4) {
			if (r_orig.height() < 8) r_orig.by = r_orig.ty + 8; // 小さすぎる場合は強制変更
			free( (void*)button1);
			free( (void*)button2);
			arrow_width = r_orig.height()/4;
			// 再割り当て
			button1 = create_button(2, arrow_height, arrow_width, r, g, b);
			button2 = create_button(3, arrow_height, arrow_width, r, g, b);
		}
		// 矢印ボタンの作成
		Surface* a1s = parent->Root().NewSurfaceFromRGBAData(arrow_height, arrow_width*3, button1, ALPHA_MASK);
		int x = r_orig.lx; int y = r_orig.ty;
		arrow_up = new Button(container, parent, a1s, 0, 0, 0, arrow_width, 3, Rect(x,y,x+arrow_height,y+arrow_width),1);
		arrow_up->Pic()->SetSurfaceFreeFlag();
		Surface* a2s = parent->Root().NewSurfaceFromRGBAData(arrow_height, arrow_width*3, button2, ALPHA_MASK);
		x = r_orig.rx - arrow_height; y = r_orig.by - arrow_width;
		arrow_down = new Button(container, parent, a2s, 0, 0, 0, arrow_width, 3, Rect(x,y,x+arrow_height,y+arrow_width),1);
		arrow_down->Pic()->SetSurfaceFreeFlag();
		// picture作成（ボタンの動く領域）
		Rect r = r_orig;
		r.ty += arrow_width;
		r.by -= arrow_width;
		panel = parent->create_node(r, 0);
		SetPic(panel);
		// ボタンの中心線を描画、設定
		Surface* s = parent->Root().NewSurface(r.width()/2, r.height(), ALPHA_MASK);
		DSurfaceFill(s, Rect(0,0,r.width()/2,r.height()), 0, 0, 0, 0xff);
		Pic()->SetSurface(s, -r.width()/4, 0, 0);
		Pic()->SetSurfaceFreeFlag();
	} else {
		// 矢印に必要な領域確保
		int arrow_height = r_orig.height();
		button1 = create_button(0, arrow_width, arrow_height, r, g, b);
		button2 = create_button(1, arrow_width, arrow_height, r, g, b);
		if (r_orig.width() < arrow_width*4) {
			if (r_orig.width() < 8) r_orig.rx = r_orig.lx + 8; // 小さすぎる場合は強制変更
			free( (void*)button1);
			free( (void*)button2);
			arrow_width = r_orig.width()/4;
			// 再割り当て
			button1 = create_button(2, arrow_width, arrow_height, r, g, b);
			button2 = create_button(3, arrow_width, arrow_height, r, g, b);
		}
		// 矢印ボタンの作成
		Surface* a1s = parent->Root().NewSurfaceFromRGBAData(arrow_width, arrow_height*3, button1, ALPHA_MASK);
		int x = r_orig.lx; int y = r_orig.ty;
		arrow_up = new Button(container, parent, a1s, 0, 0, 0, arrow_height, 3, Rect(x,y,x+arrow_width,y+arrow_height),1);
		arrow_up->Pic()->SetSurfaceFreeFlag();
		Surface* a2s = parent->Root().NewSurfaceFromRGBAData(arrow_width, arrow_height*3, button2, ALPHA_MASK);
		x = r_orig.rx - arrow_width; y = r_orig.by - arrow_height;
		arrow_down = new Button(container, parent, a2s, 0, 0, 0, arrow_height, 3, Rect(x,y,x+arrow_width,y+arrow_height),1);
		arrow_down->Pic()->SetSurfaceFreeFlag();
		// picture作成（ボタンの動く領域）
		Rect r = r_orig;
		r.lx += arrow_width;
		r.rx -= arrow_width;
		panel = parent->create_node(r, 0);
		SetPic(panel);
		// ボタンの中心線を描画、設定
		Surface* s = parent->Root().NewSurface(r.width(), r.height()/2, ALPHA_MASK);
		DSurfaceFill(s, Rect(0,0,r.width(),r.height()/2), 0, 0, 0, 0xff);
		Pic()->SetSurface(s, 0, -r.height()/4, 0);
		Pic()->SetSurfaceFreeFlag();
	}
	arrow_up->press_func = &Scale::PressArrowUp;
	arrow_up->press_pointer = (void*)this;
	arrow_down->press_func = &Scale::PressArrowDown;
	arrow_down->press_pointer = (void*)this;
	arrow_up->show();
	arrow_down->show();
	panel->show();
	InitCursor(0);
}

void Scale::InitCursor(int width_ratio) {
	int r=cursor_color.r, g=cursor_color.g, b=cursor_color.b;
	if (cursor) delete cursor;
	cursor = 0;
	Rect region(0,0);
	if (width_ratio < 0) width_ratio = 0;
	else if (width_ratio > 1024) width_ratio = 1024;
	if (is_vertical) {
		if (width_ratio == 0) cursor_width = Pic()->Width() * 3 / 2; // 幅の1.5倍
		else cursor_width = Pic()->Height()*width_ratio/1024;
		if (cursor_width <= 0) return; // カーソルなし（いいのか？）
		region = Rect(0, 0, Pic()->Width(), cursor_width);
	} else { // horizontal
		if (width_ratio == 0) cursor_width = Pic()->Height() * 3 / 2; // 高さの1.5倍
		else cursor_width = Pic()->Width()*width_ratio/1024;
		if (cursor_width <= 0) return; // カーソルなし（いいのか？）
		region = Rect(0, 0, cursor_width, Pic()->Height());
	}

	int height = region.height();
	int width = region.width();
	char* box = create_box(width, height, r, g, b);
	Surface* boxs = parent->Root().NewSurfaceFromRGBAData(width, height*3, box, ALPHA_MASK);
	cursor = new Button(container, panel, boxs, 0, 0, 0, height, 3, region, 2);
	cursor->Pic()->SetSurfaceFreeFlag();

	cursor->press_func = &Scale::PressCursor;
	cursor->press_pointer = (void*)this;
	cursor->drag_func = &Scale::DragCursor;
	cursor->drag_pointer = (void*)this;
	cursor->show();

	// 矢印等をクリックしたときの移動量計算
	int bar_width;
	if (is_vertical) bar_width = Pic()->Height();
	else bar_width = Pic()->Width();
	if (bar_width <= 0) value_add = max-min;
	else if (cursor_width == 0) value_add = 2;
	else value_add = scale_max*cursor_width/bar_width;

	return;
}

void Scale::PressArrowDown(void* pointer, Button* from) {
	Scale* self = (Scale*)pointer;
	self->SetScaleValue(self->value + self->value_add);
}
void Scale::PressArrowUp(void* pointer, Button* from){
	Scale* self = (Scale*)pointer;
	self->SetScaleValue(self->value - self->value_add);
}
void Scale::PressCursor(void* pointer, Button* from){
	Scale* self = (Scale*)pointer;
	self->value_dragstart = self->value;
}
void Scale::DragCursor(int x_from, int y_from,int x, int y, void* pointer, Button* from){
	Scale* self = (Scale*)pointer;
	int dx, w;
	if (self->is_vertical) {
		dx = y-y_from;
		w = self->Event::Video::Region().height();
	} else {
		dx = x-x_from;
		w = self->Event::Video::Region().width();
	}
	if (w == 0) return;
	self->SetScaleValue(self->value_dragstart + dx*scale_max/w);
}
int Scale::CalcValue(void) {
	Rect own_region = Event::Video::Region();
	int x, w;
	if (is_vertical) {
		w = own_region.height();
		x = mouse_y - own_region.ty;
	} else {
		w = own_region.width();
		x = mouse_x - own_region.lx;
	}
	if (w == 0) return 0;
	if (x < 0) x = 0;
	else if (x > w) x = w;
	return x*scale_max/w;
}
void Scale::Press(void){
	int v = CalcValue();
	if (v < value) SetScaleValue(value-value_add);
	else SetScaleValue(value+value_add);
	return;
}
void Scale::Motion(int x, int y){
	mouse_x = x;
	mouse_y = y;
}

void Scale::SetRange(int new_min, int new_max) {
	min = new_min;
	max = new_max;
	SetValue(value);
	return;
}
void Scale::SetValue(int new_value) {
	if (min == max) {
		value = min;
		SetScaleValue(0);
		return;
	}
	int scale_value = (new_value-min) * scale_max / (max-min);
	SetScaleValue(scale_value);
}
int Scale::GetValue(void) const{
	return min + value * (max-min) / scale_max;
}
void Scale::SetScaleValue(int new_value) {
	if (new_value < 0) value = 0;
	else if (new_value > scale_max) value = scale_max;
	else value = new_value;
	if (is_vertical) {
		int h = Pic()->Height();
		int y = (h-cursor_width) * value / scale_max;
		if (y < 0) y = 0;
		cursor->Pic()->Move(0, y);
	} else { // horizontal
		int w = Pic()->Width();
		int x = (w-cursor_width) * value / scale_max;
		if (x < 0) x = 0;
		cursor->Pic()->Move(x, 0);
	}
	if (change_func) {
		(*change_func)(change_pointer, this);
	}
	return;
}

TextButton::TextButton(Event::Container& container, PicContainer* parent, const char* text, int _text_size, Attribute attr, const Rect& r_orig, int _z, const Color& _fore, const Color& _pressed, const Color& _back) :
	Button(container, parent, (Surface*)0, 0, 0, 0, 0, 0, r_orig, _z),
	root(parent->Root()), surface(0), attribute(attr), text_size(_text_size),
	fore(_fore), pressed(_pressed), back(_back)
{
	bool rect_changed = false;
	// まず、テキスト領域の広さを得る
	Rect r(r_orig);

	if (text == 0) text = "";
	int width = r.width(); int height = r.height();
	if (width == 0) width = parent->Width() - r.lx;

	TextGlyphStream gs = DefaultLayout(text_size)->Layout(text, width);

	if (r.width() == 0) { // 文字に合わせてウィジット作成
		rect_changed = true;
		width = gs.width() + text_size;
		r.rx = r.lx + gs.width();
		attribute = Attribute(attribute | CENTER);
	}
	if (r.height() == 0) {
		rect_changed = true;
		if (attribute & NOPADDING) r.by = r.ty + gs.height();
		else r.by = r.ty + gs.height() + text_size/2;
	}

	if (rect_changed) {
		// 大きさ変更
		Pic()->SetSurfaceRect(r);
	}

	sx = 0; sy = 0; sdx = 0; sdy = r.height(); nptn = 3;
	int x = 0, y = 0;
	if (attribute & CENTER)
		x = (Pic()->Width() - gs.width()) / 2;
	y = (Pic()->Height() - gs.height()) / 2;

	if (back.a == 0) { // 背景なし、もしくはボタン押の状態のみ背景あり
		surface = root.NewSurface(r.width(), r.height()*2, ALPHA_MASK);
		DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
		if (attribute & REVERSE) {
			DSurfaceFill(surface, Rect(0,r.height(),r.width(),r.height()*2), pressed.r, pressed.g, pressed.b, 0xff);
		}
		gs.SetColor(fore.r, fore.g, fore.b);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), surface, Rect(x,y));
		gs.SetColor(pressed.r, pressed.g, pressed.b);
		if (attribute & REVERSE) {
			gs.SetReverse(true);
			DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), surface, Rect(x,y+r.height()));
			gs.SetReverse(false);
		} else {
			DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), surface, Rect(x,y+r.height()));
		}
		nptn = 2;
	} else { // ボタン型の背景あり
		/* ラベル用の Surface を作る */
		width = r.width(); height = r.height();
		char* box = create_box(width, height, back.r, back.g, back.b);
		surface = root.NewSurfaceFromRGBAData(r.width(), r.height()*3, box, ALPHA_MASK);
	
		Surface* text_surface = root.NewSurface(r.width(), r.height(), ALPHA_MASK);
		DSurfaceFill(text_surface, Rect(*text_surface), 0, 0, 0, 0);
	
		gs.SetColor(fore.r, fore.g, fore.b);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), text_surface, Rect(x,y));
		root.BlitSurface(text_surface, Rect(0,0,width,height), surface, Rect(0,0));
		gs.SetColor(pressed.r, pressed.g, pressed.b);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), text_surface, Rect(x,y));
		root.BlitSurface(text_surface, Rect(0,0,width,height), surface, Rect(0,height));
		root.BlitSurface(text_surface, Rect(0,0,width,height), surface, Rect(0,height*2));
		root.DeleteSurface(text_surface);
	}

	Pic()->SetSurface(surface, 0, 0);
	show();
}
void TextButton::SetText(const char* text, const Color& _fore, const Color& _pressed, const Color& _back)
{
	int width = Pic()->Width(); int height = Pic()->Height();
	// まず、テキスト領域の広さを得る
	if (text == 0) text = "";

	TextGlyphStream gs = DefaultLayout(text_size)->Layout(text, width);

	int x = 0, y = 0;
	if (attribute & CENTER) {
		x = (width - gs.width()) / 2;
		y = (height - gs.height()) / 2;
	}
	int surf_x = Pic()->SurfacePosX();
	int surf_y = Pic()->SurfacePosY();
	Pic()->SetSurface( (Surface*)0,0,0);
	root.DeleteSurface(surface);
	surface = 0;

	if (back.a == 0) { // 背景なし
		surface = root.NewSurface(width, height*2, ALPHA_MASK);
		DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
		if (attribute & REVERSE) {
			DSurfaceFill(surface, Rect(0,height,width,height*2), pressed.r, pressed.g, pressed.b, 0xff);
		}
		gs.SetColor(_fore.r, _fore.g, _fore.b);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), surface, Rect(x,y));
		gs.SetColor(_pressed.r, _pressed.g, _pressed.b);
		gs.SetReverse(true);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), surface, Rect(x,y+height));
		gs.SetReverse(false);
		nptn = 2;
	} else {
		/* ラベル用の Surface を作る */
		char* box = create_box(width, height, _back.r, _back.g, _back.b);
		surface = root.NewSurfaceFromRGBAData(width, height*3, box, ALPHA_MASK);
	
		Surface* text_surface = root.NewSurface(width, height, ALPHA_MASK);
		DSurfaceFill(text_surface, Rect(*text_surface), 0, 0, 0, 0);
	
		gs.SetColor(_fore.r, _fore.g, _fore.b);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), text_surface, Rect(x,y));
		root.BlitSurface(text_surface, Rect(0,0,width,height), surface, Rect(0,0));
		gs.SetColor(_pressed.r, _pressed.g, _pressed.b);
		DSurfaceRenderText(gs.begin(), gs.end(), Rect(0,0,gs.width(),gs.height()), text_surface, Rect(x,y));
		root.BlitSurface(text_surface, Rect(0,0,width,height), surface, Rect(0,height));
		root.BlitSurface(text_surface, Rect(0,0,width,height), surface, Rect(0,height*2));
		root.DeleteSurface(text_surface);
	}

	Pic()->SetSurface(surface, surf_x, surf_y);
	show();
}

TextButton::~TextButton() {
	if (surface) root.DeleteSurface(surface);
	surface = 0;
	return;
}

Text::Text(Event::Container& container, PicContainer* parent, const Rect& r, const Rect& text_r, int _fontsize) :
	Event::Video(container),
	Event::Time(container),
	event(container),
	srcrect(0,0,0,0),
	layout(wdefault_font.c_str(), _fontsize), fontsize(_fontsize) {

	SetPic(parent->create_node(r, 0));
	surface = parent->Root().NewSurface(text_r.width(), text_r.height(), ALPHA_MASK);
	pictext = PicNode()->create_leaf(text_r, PicBase::CACHE_BACK);
	pictext->SetSurface(surface, 0, 0);
	pictext->show();
	cursor = 0;
	cursor_activated = false;
	window_activated = false;

	event.RegisterGlobalPressFunc(&Pressed, (void*)this);

	speed = 10;
	wait_delay = -1;
	status = PREPARE;
	old_time = wait_starttime = 0;
}

Text::~Text() {
	event.DeleteGlobalPressFunc(&Pressed, (void*)this);
	PicNode()->Root().DeleteSurface(surface);
}

void Text::SetSpeed(int new_speed) {
	speed = new_speed;
}
void Text::SetWait(int new_wait) {
	if (new_wait < 0) new_wait = 100000000;
	wait_delay = new_wait;
}

int Text::CalcScrollHeight(void) {
	int i;
	int len = bottom_pos.size();
	int y0 = bottom_pos[line_number];
	int height = Rect(*surface).height();
	for (i=line_number; i<len; i++)
		if (bottom_pos[i] - y0 > height) break;
	if (i == line_number) i = line_number + 1;
	return i - line_number - 1;
}
void Text::Elapsed(unsigned int current_time) {
	SetWakeup(current_time + 50);
	if (status == PREPARE) {
		old_time = current_time;
		return;
	}
	int nChar = speed * (current_time - old_time) / 1000;
	if (speed == -1 || press_count) nChar = -1;
	if (nChar == 0) return;
	if (speed == -1) old_time = current_time;
	else old_time += nChar * 1000 / speed;

	switch(status) {
	case WAIT: goto label_wait;
	case SCROLL: goto label_scroll;
	case DRAW2: goto label_draw2;
	case WAIT2: goto label_wait2;
	}

	status = DRAW;
	if (press_count) {
		nChar = -1;
		press_count = 0;
	}
	DrawText(nChar);
	if (nChar == 0) return;
	status = WAIT;
	cursor_activated = true;
	if (cursor_activated && window_activated && cursor) cursor->show();
	wait_starttime = current_time;
label_wait:
	if (current_time < wait_starttime + wait_delay && press_count == 0) return;
	press_count = 0;
	nChar = 0;
	cursor_activated = false;
	if (cursor) cursor->hide();
	while(cur_pos != gstream.end()) {
		// スクロールしては次行描画、を繰り返す
		for (scroll_height = CalcScrollHeight(); scroll_height > 0; scroll_height--) {
			status = SCROLL;
label_scroll:
			if (press_count) break;
			Scrollup(nChar);
			if (nChar == 0) return;
			status = DRAW2;
label_draw2:
			if (press_count) break;
			DrawText(nChar);
			if (nChar == 0) return;
		}
		if (nChar != 0 && scroll_height) {
			nChar = 100000;
			if (status == SCROLL) Scrollup(nChar);
			DrawText(nChar);
			scroll_height--;
			for (; scroll_height > 0; scroll_height--) {
				Scrollup(nChar);
				DrawText(nChar);
			}
		}
		press_count = 0;
		status = WAIT2;
		cursor_activated = true;
		if (cursor_activated && window_activated && cursor) cursor->show();
		wait_starttime = current_time;
label_wait2:
		if (current_time < wait_starttime + wait_delay && press_count == 0) return;
		press_count = 0;
		nChar = 0;
		cursor_activated = false;
		if (cursor) cursor->hide();
	}
	status = PREPARE;
	return;
}

bool Text::Pressed(int x, int y, void* pointer) {
	Text* wid = (Text*)pointer;
	if (wid->Pic()->IsHidden()) return true;
	wid->press_count++;
	return true;
}

void Text::Clear(void) {
	stream.container.clear();
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
	pictext->ReBlit();
	status = PREPARE;
}

void Text::Flush(void) {
	int nChar = -1;
	DrawText(nChar);
}

void Text::Start(void) {
	gstream.clear();
	bottom_pos.clear();
	layout.Layout(stream, gstream, bottom_pos, pictext->Width()-fontsize/2);

	// height の積算値として bottom_pos を計算
	std::vector<int>::iterator it;
	int pos = 0;
	for (it = bottom_pos.begin(); it != bottom_pos.end(); it++) {
		pos += *it;
		*it = pos;
	}

	cur_pos = gstream.begin();
	line_number = 0;
	scrolled_count = 0;
	srcrect = Rect(0, 0, pictext->Width(), pictext->Height());
	press_count = 0;

	status = DRAW;
	cursor_activated = false;
	if (cursor) cursor->hide();
}

void Text::DrawText(int& nChar) {
	// 描画範囲を得る
	iterator end = gstream.end();
	iterator it = cur_pos;
	while(nChar && it != end) { // nChar < 0 なら出来るだけの文字を描画
		if (! (it->flag & TextGlyph::Group)) nChar--;
		if (it->flag & TextGlyph::LineEnd) {
			if (bottom_pos[line_number+1] > srcrect.by) { //改行すると画面から出てしまう
				it++;
				if (nChar == 0) nChar = 1;
				break;
			}
			line_number++;
		}
		it++;
	}
	// 描画する
	Rect r = DSurfaceRenderText(cur_pos, it, srcrect, surface, Rect(0,0,0,0));
	pictext->ReBlit(r);
	cur_pos = it;
	return;
}

void Text::Scrollup(int& nChar) {
	if (nChar < 0) { // 一画面分スクロールする
		DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
		pictext->ReBlit();
		srcrect = Rect(*surface);
		srcrect.rmove(0, bottom_pos[line_number]);
		line_number++;
		scrolled_count = 0;
		return;
	}
	// スクロール幅を求める
	const int max_scroll_count = 8;
	int dy = bottom_pos[line_number+1] - bottom_pos[line_number];
	int cur_dy;
	if (scrolled_count+nChar >= max_scroll_count) {
		cur_dy = dy - (scrolled_count*dy/max_scroll_count);
		nChar -= max_scroll_count-scrolled_count;
		nChar++;
		scrolled_count = 0;
		line_number++;
		srcrect.rmove(0, dy);
	} else {
		cur_dy = (scrolled_count+nChar)*dy/max_scroll_count;
		cur_dy -=(scrolled_count*dy/max_scroll_count);
		scrolled_count += nChar;
		nChar = 0;
	}
	Rect r(*surface);
	DSurfaceMove(surface, r, surface, Rect(0, -cur_dy, 0, 0));
	r.ty = r.by-cur_dy;
	DSurfaceFill(surface, r, 0, 0, 0, 0);
	pictext->ReBlit();
	return;
}

void Text::activate(void) {
	event.RegisterGlobalPressFunc(&Pressed, (void*)this);
	Event::Video::activate();
	window_activated = true;
	if (cursor_activated && window_activated && cursor) cursor->show();
}
void Text::deactivate(void) {
	event.DeleteGlobalPressFunc(&Pressed, (void*)this);
	Event::Video::deactivate();
	window_activated = false;
	if (cursor) cursor->hide();
}
void Text::SetCursor(TimeCursor* c) {
	cursor = c;
	if (c) {
		if (cursor_activated && window_activated) c->show();
		else c->hide();
	}
}

Label::Label(PicContainer* parent, const Rect& r_orig, bool _is_center, const char* text, int _text_size) :
	is_center(_is_center),
	root(parent->Root()),
	text_size(_text_size) {
	Rect r(r_orig);

	if (text == 0) text = "";
	int width = r.width(); int height = r.height();
	if (width == 0) width = parent->Width() - r.lx;

	TextGlyphStream gs = DefaultLayout(text_size)->Layout(text, width);

	if (r.width() == 0) { // 文字に合わせてウィジット作成
		width = gs.width();
		r.rx = r.lx + gs.width();
	}
	if (r.height() == 0) {
		r.by = r.ty + gs.height();
	}

	SetPic(parent->create_leaf(r, 0));

	/* ラベル用の Surface を作る */
	surface = parent->Root().NewSurface(r.width(), r.height(), ALPHA_MASK);
	
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
	int x = 0, y = 0;
	if (is_center) {
		x = (Pic()->Width() - gs.width()) / 2;
		y = (Pic()->Height() - gs.height()) / 2;
	}

	DSurfaceRenderText(gs.begin(), gs.end(), Rect(*surface), surface, Rect(x,y));

	Pic()->SetSurface(surface, 0, 0);
	show();
}
Label::~Label() {
	root.DeleteSurface(surface);
}
void Label::SetText(const char* text) {
	if (text == 0) text = "";
	TextGlyphStream gs = DefaultLayout(text_size)->Layout(text, Pic()->Width());
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
	int x = 0, y = 0;
	if (is_center) {
		x = (Pic()->Width() - gs.width()) / 2;
		y = (Pic()->Height() - gs.height()) / 2;
	}

	DSurfaceRenderText(gs.begin(), gs.end(), Rect(*surface), surface, Rect(x,y));
	Pic()->ReBlit();
}

Dialog::Dialog(Event::Container& container, PicContainer* parent, const char* text, bool with_cancel) :
	Event::Video(container) {

	int x,y;
	status = WAIT;
	set_func = 0;
	set_pointer = 0;

	XKFont::HorizLayout& layout = *DefaultLayout(26);
	int dialog_width = parent->Width() / 2;
	TextGlyphStream s_ok = layout.Layout("ＯＫ", dialog_width);
	TextGlyphStream s_cancel = layout.Layout("取消", dialog_width);
	TextGlyphStream s_text = layout.Layout(text, dialog_width);

	Rect r_text(0, 0, s_text.width(), s_text.height());
	Rect r_ok(0, 0, s_ok.width(), s_ok.height());
	Rect r_cancel(0, 0, s_cancel.width(), s_cancel.height());

	/* ダイアログボックスの Surface を作る */
	int dwidth = r_text.width() + (r_text.width()/10)*2 + 6;
	int dheight = r_text.height() + r_ok.height() + r_cancel.height()*3 + 4;
	surface_diag = parent->Root().NewSurface(dwidth, dheight, NO_MASK); // alpha なし
	DSurfaceFill(surface_diag, Rect(*surface_diag), 0xf0, 0xd0, 0xa0);
	DrawBox(surface_diag, Rect(0,0,dwidth,dheight));

	Surface* surface_text = parent->Root().NewSurface(r_text.width(), r_text.height(), ALPHA_MASK);
	s_text.SetColor(0x38, 0x20, 0x18);
	DSurfaceRenderText(s_text.begin(), s_text.end(), r_text, surface_text, Rect(0, 0, 0, 0));
	x = r_text.width()/10 + 3;
	y = r_cancel.height()+2;
	parent->Root().BlitSurface(surface_text, r_text, surface_diag, Rect(x, y, x+r_text.width(), y+r_text.height()));
	parent->Root().DeleteSurface(surface_text);

	/* panel をつくる */
	x = (parent->Width()-dwidth)/2;
	y = (parent->Height()-dheight)/2;
	SetPic(parent->create_node(Rect(x, y, x+dwidth, y+dheight), 0));

	/* ボタンを作成する */
	/* f8d8c8 背景(明)*/
	/* f0d0a0 背景*/
	/* b08040 枠（明)*/
	/* 805010 枠*/
	/* 382018 黒*/
	/* 9890f8 青*/ 
	/* dc6448 赤*/
	/* 各ボタンは左右にボタン幅の 1/4, 上下にボタン幅の 1/4 のマージンを持つ */
	Rect r_btn(r_ok); r_btn.join(r_cancel);
	int btn_width = r_btn.width() * 3 / 2;
	int btn_height = r_btn.height() * 3 / 2;
	surface_btn = parent->Root().NewSurface(btn_width, btn_height*4, ALPHA_MASK);
	DSurfaceFill(surface_btn, Rect(*surface_btn), 0, 0, 0, 0);
	s_ok.SetColor(0x38, 0x20, 0x18);
	DSurfaceRenderText(s_ok.begin(), s_ok.end(), r_ok, surface_btn, Rect( (btn_width-r_ok.width())/2,(btn_height-r_ok.height())/2));
	s_ok.SetColor(0x98, 0x90, 0xf8);
	DSurfaceRenderText(s_ok.begin(), s_ok.end(), r_ok, surface_btn, Rect( (btn_width-r_ok.width())/2,(btn_height-r_ok.height())/2 + btn_height));
	s_cancel.SetColor(0x38, 0x20, 0x18);
	DSurfaceRenderText(s_cancel.begin(), s_cancel.end(), r_cancel, surface_btn, Rect( (btn_width-r_cancel.width())/2,(btn_height-r_cancel.height())/2 + btn_height*2));
	s_cancel.SetColor(0xdc, 0x64, 0x48);
	DSurfaceRenderText(s_cancel.begin(), s_cancel.end(), r_cancel, surface_btn, Rect( (btn_width-r_cancel.width())/2,(btn_height-r_cancel.height())/2 + btn_height*3));

	x = (dwidth - btn_width*2) / 3;;
	y = r_cancel.height()*3/2 + r_text.height() + 2;
	if (!with_cancel) x = (dwidth - btn_width) / 2;
	Button* b_ok = new Button(container, PicNode(), surface_btn, 0, 0, 0, btn_height, 2, Rect(x, y, x+btn_width, y+btn_height), 1);
	DrawBox(surface_diag, Rect(x-3,y-2,x+btn_width+3,y+btn_height+2));
	b_ok->press_pointer = (void*)this;
	b_ok->press_func = &press_ok;
	if (with_cancel) {
		x += x + btn_width;
		Button* b_cancel = new Button(container, PicNode(), surface_btn, 0, btn_height*2, 0, btn_height, 2, Rect(x, y, x+btn_width, y+btn_height), 1);
		DrawBox(surface_diag, Rect(x-3,y-2,x+btn_width+3,y+btn_height+2));
		b_cancel->press_pointer = (void*)this;
		b_cancel->press_func = &press_cancel;
	}

	Pic()->SetSurface(surface_diag, 0, 0);
	Pic()->ZMove(ZMOVE_TOP);

	show_all();
}
Dialog::~Dialog() {
	PicRoot& root = PicNode()->Root();
	SetPic(0);
	root.DeleteSurface(surface_btn);
	root.DeleteSurface(surface_diag);
	return;
}

void Dialog::press_ok(void* pointer, Button* btn) {
	if (pointer) {
		Dialog* wid = (Dialog*)pointer;
		wid->status = OK;
		if (wid->set_func) {
			(*wid->set_func)(wid->set_pointer, wid);
		}
	}
}
void Dialog::press_cancel(void* pointer, Button* btn) {
	if (pointer) {
		Dialog* wid = (Dialog*)pointer;
		wid->status = CANCEL;
		if (wid->set_func) {
			(*wid->set_func)(wid->set_pointer, wid);
		}
	}
}

void Dialog::DrawBox(Surface* s, const Rect& r) {
	DSurfaceFill(s, Rect(r.lx, r.ty, r.rx, r.ty+1), 0x80, 0x50, 0x10);
	DSurfaceFill(s, Rect(r.lx, r.ty+1, r.rx, r.ty+2), 0x38, 0x20, 0x18);
	DSurfaceFill(s, Rect(r.lx, r.by-2, r.rx, r.by-1), 0x80, 0x50, 0x10);
	DSurfaceFill(s, Rect(r.lx, r.by-1, r.rx, r.by), 0x38, 0x20, 0x18);

	DSurfaceFill(s, Rect(r.lx, r.ty, r.lx+1, r.by), 0xb0, 0x80, 0x40);
	DSurfaceFill(s, Rect(r.lx+1, r.ty+1, r.lx+2, r.by-1), 0x80, 0x50, 0x10);
	DSurfaceFill(s, Rect(r.lx+1, r.ty+2, r.lx+2, r.by-2), 0x38, 0x20, 0x18);
	DSurfaceFill(s, Rect(r.rx-3, r.ty+2, r.rx-2, r.by-2), 0xb0, 0x80, 0x40);
	DSurfaceFill(s, Rect(r.rx-2, r.ty+1, r.rx-1, r.by-1), 0x80, 0x50, 0x10);
	DSurfaceFill(s, Rect(r.rx-1, r.ty, r.rx, r.by), 0x38, 0x20, 0x18);
}

AnmTime::AnmTime(Event::Container& container, PicBase* _pic, int _total_time, int _all_count) :
	Event::Time(container),
	PicAnm(_pic), start_time(0), total_time(_total_time), all_count(_all_count) {
	status = FINISHED;
	if (total_time == 0) total_time = 1;
}
AnmTime::AnmTime(Event::Container& container, std::vector<PicBase*> _pic, int _total_time, int _all_count) :
	Event::Time(container),
	PicAnm(_pic), start_time(0), total_time(_total_time), all_count(_all_count) {
	status = FINISHED;
	if (total_time == 0) total_time = 1;
}
void AnmTime::Elapsed(unsigned int current_time) {
	if (total_time == 0) return;
	if (status == FINISHED || current_time == 0) {SetWakeup(current_time+1); return;}
	if (start_time == 0) {
		start_time = current_time;
		Start();
	}
	unsigned int time_elapsed = current_time - start_time;
	if (time_elapsed < total_time) {
		int count = time_elapsed * all_count / total_time;
		Exec(count);
		int next_time = start_time + (count+1) * total_time / all_count;
		SetWakeup(next_time);
	} else {
		Exec(all_count);
		Finish();
		status = FINISHED;
	}
	return;
}
void AnmTime::Abort(void) { 
	if (status == FINISHED) return;
	if (start_time == 0) {
		Start();
		Exec(all_count);
	}
	if (total_time) {
		Finish();
	}
	status = FINISHED;
}
bool AnmTime::IsEnd(void) {
	return status == FINISHED;
}
AnmMove::AnmMove(Event::Container& container, PicBase* _pic, const Rect& _to, int total_time) :
	AnmTime(container, _pic, total_time),
	from(0,0), to(_to) {
	from.lx = _pic->PosX();
	from.ty = _pic->PosY();
	from.rx = from.lx + _pic->Width();
	from.by = from.ty + _pic->Height();

	int dx = to.lx - from.lx;
	int dy = to.ty - from.ty;
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	if (dx < dy) dx = dy;
	if (dx == 0) dx = 1;
	SetAllCount(dx);
}
void AnmMove::Exec(int count) {
	Rect r(from);
	int dx = to.lx - from.lx;
	int dy = to.ty - from.ty;
	r.rmove(dx*count/all_count, dy*count/all_count);
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->Move(r.lx, r.ty);
}
AnmAlpha::AnmAlpha(Event::Container& container, PicBase* _pic, int alpha_from, int alpha_to, int total_time) :
	AnmTime(container, _pic, total_time),
	from(alpha_from), to(alpha_to), alpha_r(0,0,1,1) {
	if (from < 0) from = 0;
	if (from >= ALPHA_MAX) from = ALPHA_MAX;
	if (to < 0) to = 0;
	if (to >= ALPHA_MAX) to = ALPHA_MAX;
	int c = from - to;	
	if (c < 0) c = -c;
	if (c == 0) c = 1;
	SetAllCount(c);
}
AnmAlpha::AnmAlpha(Event::Container& container, std::vector<PicBase*> _pic, int alpha_from, int alpha_to, int total_time) :
	AnmTime(container, _pic, total_time),
	from(alpha_from), to(alpha_to), alpha_r(0,0,1,1) {
	if (from < 0) from = 0;
	if (from >= ALPHA_MAX) from = ALPHA_MAX;
	if (to < 0) to = 0;
	if (to >= ALPHA_MAX) to = ALPHA_MAX;
	int c = from - to;	
	if (c < 0) c = -c;
	if (c == 0) c = 1;
	SetAllCount(c);
}
void AnmAlpha::Start(void) {
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->show();
}
void AnmAlpha::Exec(int count) {
	alpha = (from * (all_count-count) + (to-from) * count) / all_count;
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->SetSurfaceAlpha(&alpha, alpha_r);
}
void AnmAlpha::Finish(void) {
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) {
		if (to == 0) (*it)->hide();
		else if (to != ALPHA_MAX) fprintf(stderr,"Warning in AnmAlpha::Finish: alpha value suddenly changed.\n");
		(*it)->SetSurfaceAlpha(0,Rect(0,0));
	}
}
AnmAlphaMove::AnmAlphaMove(Event::Container& container, PicBase* _pic) :
	AnmTime(container, _pic, 0) {
}
void AnmAlphaMove::SetPtn(void) {
	int total = 0;
	std::vector<Ptn>::iterator it;
	for (it=ptns.begin(); it!=ptns.end(); it++) {
		if (total < it->next_tick) total = it->next_tick;
	}
	SetAllCount(total);
	SetTotalTime(total);
	cur_count = 0;
}
void AnmAlphaMove::Exec(int count) {
	if (ptns.empty()) return;
	if (cur_count != 0 && ptns[cur_count].next_tick > count) return;
	if (cur_count >= ptns.size()) return;
	// 次のパターンを探す
	// count <= it->next_tick なる条件を満たす最後の it を探す
	std::vector<Ptn>::iterator it;
	for (it=ptns.begin()+cur_count; it != ptns.end(); it++) {
		if (count <= it->next_tick) break;
	}
	if (it == ptns.end()) {
		fprintf(stderr,"end\n");
		it = ptns.end() - 1;
	}
	cur_count = it - ptns.begin();

	iterator p;
	for (p=pic.begin(); p!=pic.end(); p++) {
		// move
		(*p)->Move(it->pos.lx, it->pos.ty);
		(*p)->SetSurfacePos(it->surface_pos.lx, it->surface_pos.ty);
		// alpha set
		if (it->alpha == 0) (*p)->hide();
		else if (it->alpha == ALPHA_MAX) { (*p)->show(); (*p)->SetSurfaceAlpha(0, Rect(0,0)); }
		else { (*p)->show(); (*p)->SetSurfaceAlpha( &(it->alpha), Rect(0,0,1,1)); }
	}
}
void AnmAlphaMove::Finish(void) {
	if (ptns.empty()) return;
	if (cur_count >= ptns.size() - 1) return;
	cur_count = ptns.size() - 1;
	Exec(ptns[cur_count].next_tick); // 最後の pattern の状態にする
}

AnmPtnSolid::AnmPtnSolid(Event::Container& container, PicBase* _pic, const unsigned char* _ptn, const Rect& _alpha_r, int total_time) :
	AnmTime(container, _pic, total_time),
	ptn(_ptn), alpha_r(_alpha_r)
{
	ptn_len = alpha_r.width() * alpha_r.height();
	alpha = new unsigned char[ptn_len];
	int max_a = 0;
	for (int i=0; i<ptn_len; i++) {
		if (ptn[i] > max_a) max_a = ptn[i];
	}
	if (max_a == 0) max_a = 1;
	SetAllCount(max_a);
}

AnmPtnAlpha::AnmPtnAlpha(Event::Container& container, PicBase* _pic, const unsigned char* _ptn, const Rect& _alpha_r, int _band_width, int total_time) :
	AnmTime(container, _pic, total_time),
	ptn(_ptn), band(_band_width), alpha_r(_alpha_r)
{
	ptn_len = alpha_r.width() * alpha_r.height();
	alpha = new unsigned char[ptn_len];
	if (band <= 0) band = 1;
	SetAllCount(ALPHA_MAX+band);
}

void AnmPtnSolid::Exec(int count) {
	int i;
	for (i=0; i<ptn_len; i++) {
		if (ptn[i] <= count) alpha[i] = ALPHA_MAX;
		else alpha[i] = 0;
	}
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->SetSurfaceAlpha(alpha, alpha_r);
}
void AnmPtnAlpha::Exec(int count) {
	int i;
	int ptn_zero = count;
	int ptn_max = count - band;
	for (i=0; i<ptn_len; i++) {
		if (ptn[i] >= ptn_zero) alpha[i] = 0;
		else if (ptn[i] < ptn_max) alpha[i] = ALPHA_MAX;
		else alpha[i] = (ptn_zero-ptn[i])*ALPHA_MAX/band;
	}
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->SetSurfaceAlpha(alpha, alpha_r);
}

void AnmPtnSolid::Start(void) {
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->show();
}
void AnmPtnSolid::Finish(void) {
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->SetSurfaceAlpha(0, Rect(0,0));
}
void AnmPtnAlpha::Start(void) {
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->show();
}
void AnmPtnAlpha::Finish(void) {
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++) (*it)->SetSurfaceAlpha(0, Rect(0,0));
}

/*

Widget の種類
	Mouse: マウス位置に連動する。Surface と origin が必須
	Panel : なにも存在しないところに張りつけていく
		背景張りつけも可能
		ButtonPanel : 無効化可能。Group の Button がカーソルに入ったら、全Button / Label が「カーソル下」状態になる
			同一変数を扱うボタンは原則、同一ButtonPanel の下に入ること（同期する。高速化可能）
			そうでない場合、ボタンの GlobalVariable フラグを立てる必要がある
		背景種類：Window
			内部の透明度と枠形を別々に指定可能。枠形は全枠、部分枠どちらの形でも可能
			（部分枠なら、内部的には上枠、下枠、左右枠と別 Surface で管理する）
		DragButton
			Panel 全体をドラッグし、場所変更できるボタン。
	Button: 無効化＞通常＞カーソル下＞ボタン押下 のpicture / animation
		Toggle Button にできる(Group化すればRadioButtonにもできる)
	Label : 無効化＞通常＞カーソル下　のanimation
		animation は
		・上への変化
		・下への変化
		・常時変形
		の３つの形式をもつ。
		形式は
		・x / y increment による（全領域と x,y の大きさを指定すると左上から右上、左下、という方へ勝手に領域を変更していく）
		・色変化（明度変化）。色テーブルを指定する。Surface は alpha のみとする
			どちらも、一つのラベルに使う時間の長さを指定する
		・callback による。指定した一定時間以上が立つとCallBack が呼び出され、新たなSurface , origin を指定する。

		・Surface は
			普通の画像
			文字列（適当に仮想化）
			画像数値列
		のいずれか
	Cursor
		リターンカーソル。Label の一種。
	Number
		数字を表示する。フォントの大きさ、もしくは画像数値列
	Text
		テキストを表示する
		パネルの大きさだけ指定すると適当にやってくれる
		カーソルの位置（文字の次/最終）を指定すること
		機能：文字送り速度設定、読み飛ばし設定（常に最高速で押しっぱなし）
	ProgressBar など
		バーの長さ、あるいは位置で変数の大きさを示す。
		Tick, Max を指定、変数の変化には適当に対応できるようにする
		バーの方向として縦/横。Surface は繰り返しで使う（速度上、32pixel くらいあったほうがいいかも？）
		バーの代わりにボタンも使える。Surface 指定のメソッドが違うだけ。
		オプション：バーのどこかをクリックされたとき、そこに移動するかそこに向かって移動するか
		オプション？：矢印ボタン（いらないか）
	ScrollBar
		横/縦。Panel と連動する(専用, ProgressBar の一種として実装)
		(Panel 側で「見えない部分はdelete, 見える部分は自動で作成」機能をつける？：バックログ）

	
	
メニューの出し方
	右クリック
	ボタンを押す
	上の方、右の方など領域に行くとヌっと出てくる
メニューモード内
	ボタンを押して終了
	マップを作っておき、各メニューに名前を割り振ると二次元に広がったメニューになる
	名前を割り振ると上に名前リストがでてくる
	名前を割り振ると横に名前リストが出てくる

*/
// }
