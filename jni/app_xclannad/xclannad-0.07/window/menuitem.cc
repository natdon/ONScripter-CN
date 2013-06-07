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


#include"menuitem.h"

#define Button WidButton
#define Scale WidScale
#define Label WidLabel
#define TextButton WidTextButton
#define Text WidText

#define MenuItem WidMenuItem
#define RadioGroup WidRadioGroup
#define ScaleMenu WidScaleMenu
void fill(Surface* src, const Rect& rect, int r, int g, int b, int a = 0xff);

MenuItem::MenuItem(PicContainer* parent, const Rect& r_orig, int _x_size, int _y_size, int* _value_ptr) :
		x_size(_x_size), y_size(_y_size), value_ptr(_value_ptr), set_func(0), set_pointer(0) {
	SetPic(parent->create_node(r_orig, 0));
	menu_width = r_orig.width();
	menu_height = r_orig.height();
	label = 0;
	lb_width  = 0; lb_right = 0;
	lb_left = -1; lb_bottom = -1;
	int i;
	for (i=0; i<x_size*y_size; i++) item.push_back(0);
};
void MenuItem::SetLabelLeft(PicWidget* lb, const Rect& min_rect, const Rect& min_margin) {
	lb_width = min_rect.width();
	lb_right = min_margin.width();
	lb_left = -1;
	lb_bottom = -1;
	label = lb;
}
void MenuItem::SetLabelTop(PicWidget* lb, const Rect& left_margin, const Rect& bottom_margin) {
	lb_left = left_margin.width();
	lb_bottom = bottom_margin.height();
	lb_width = -1;
	lb_right = -1;
	label = lb;
}
void MenuItem::SetValue(int new_value) {
	SetValueImpl(new_value);
	if (value_ptr) *value_ptr = new_value;
	if (set_func) (*set_func)(set_pointer, this);
}
void MenuItem::activate(void) {
	iterator it;
	for (it=item.begin(); it!=item.end(); it++) {
		if (*it == 0) continue;
		(*it)->activate();
	}
}
void MenuItem::deactivate(void) {
	iterator it;
	for (it=item.begin(); it!=item.end(); it++) {
		if (*it == 0) continue;
		(*it)->deactivate();
	}
}
void MenuItem::pack(void) {
	int x_min = 0, y_min = 0;
	if (lb_width == -1) { // 上にラベルを貼る
		if (lb_left < 0) lb_left = 0;
		if (lb_bottom < 0) lb_bottom = 0;
		if (label && label->Pic()) {
			label->Pic()->Move(lb_left, 0);
			y_min = lb_bottom + label->Pic()->Height();
		} else {
			y_min = lb_bottom;
		}
	} else { // 左にラベルを貼る
		if (lb_right < 0) lb_right = 0;
		if (label && label->Pic()) {
			int label_width = label->Pic()->Width();
			int label_height = label->Pic()->Height();
			if (label_width > lb_width - lb_right) {
				x_min = label_width + lb_right;
			} else {
				x_min = lb_width;
			}
		} else {
			x_min = lb_width;
		}
	}
	int* item_width = new int[x_size];
	int* item_height = new int[y_size];
	int* item_x = new int[x_size];
	int* item_y = new int[y_size];
	int i,j;
	for (i=0; i<x_size; i++) item_width[i]=0;
	for (i=0; i<y_size; i++) item_height[i]=0;
	
	iterator it = item.begin();
	for (i=0; i<y_size; i++) {
		for (j=0; j<x_size; j++) {
			if (*it != 0 && (*it)->Pic() != 0) {
				PicBase* pic = (*it)->Pic();
				if (item_width[j] < pic->Width()) item_width[j] = pic->Width();
				if (item_height[i] < pic->Height()) item_height[i] = pic->Height();
			}
			it++;
		}
	}
	int width=0, height=0;
	for (i=0; i<x_size; i++) {
		width += item_width[i];
	}
	for (i=0; i<y_size; i++) {
		height += item_height[i];
	}

	int x=x_min, y=y_min;
	// width / height の再割り当て
	if (menu_width > width + x_min) {
		int dif = menu_width - width - x_min;
		int n = x_size + 1;
		x += dif/n;
		for (i=0; i<x_size; i++) {
			item_x[i] = x + item_width[i] / 2;
			x += item_width[i] + dif*(i+2)/n - dif*(i+1)/n;
		}
	} else {
		if (menu_width == 0) {
			Pic()->SetSurfaceRect(Rect(0, 0, width+x_min, Pic()->Height()));
		}
		for (i=0; i<x_size; i++) {
			item_x[i] = x + item_width[i] / 2;
			x += item_width[i];
		}
	}
	if (menu_height > height+y_min) {
		int dif = menu_height - height - y_min;
		int n = y_size + 1;
		y += dif/n;
		for (i=0; i<y_size; i++) {
			item_y[i] = y + item_height[i] / 2;
			y += item_height[i] + dif*(i+2)/n - dif*(i+1)/n;
		}
	} else {
		if (menu_height == 0) {
			Pic()->SetSurfaceRect(Rect(0, 0, Pic()->Width(), height+y_min));
		}
		for (i=0; i<y_size; i++) {
			item_y[i] = y + item_height[i] / 2;
			y += item_height[i];
		}
	}
	// 位置の再割り当て
	it = item.begin();
	for (i=0; i<y_size; i++) {
		for (j=0; j<x_size; j++) {
			if (*it != 0 && (*it)->Pic() != 0) {
				PicBase* pic = (*it)->Pic();
				int x0 = item_x[j]-pic->Width()/2;
				int y0 = item_y[i]-pic->Height()/2;
				pic->Move(x0, y0);
			}
			it++;
		}
	}
	// 左にラベルがある場合、ラベルの高さ方向のセンタリング
	if (label && label->Pic() && lb_width != -1) {
		int label_width = label->Pic()->Width();
		int label_height = label->Pic()->Height();
		label->Pic()->Move(x_min-label_width-lb_right, (Pic()->Height() - label_height) / 2);
	}

	delete[] item_width;
	delete[] item_height;
	delete[] item_x;
	delete[] item_y;
};

RadioButton::RadioButton(Event::Container& _container, PicContainer* _parent, const Rect& r_orig, int _x_size, int _y_size, int* _value_ptr, const Rect& _button_r, int _text_size, const Color& _fore, const Color& _pressed, const Color& _back) :
	MenuItem(_parent, r_orig, _x_size, _y_size,_value_ptr),
	container(_container), parent(_parent), text_size(_text_size), button_rect(_button_r), buttons(0),
	fore_color(_fore), pressed_color(_pressed), back_color(_back) {
}
void RadioButton::Add(const char* s, bool is_center) {
	Add(s, fore_color, pressed_color, back_color, is_center);
}
void RadioButton::Add(const char* s, const Color& fore, const Color& pressed, const Color& back, bool is_center) {
	if (buttons >= x_size*y_size) {
		fprintf(stderr,"too many buttons (%d/%d) in RadioButton::Add ; text = %s\n",x_size,y_size,s);
		return;
	}
	TextButton* wid = new TextButton(container, PicNode(), s, text_size, TextButton::Attribute(is_center ? TextButton::CENTER : 0), button_rect, 1, fore, pressed, back);

	wid->press_func = &PressCallback;
	wid->press_pointer = (void*)this;

	if (value_ptr && buttons == *value_ptr) wid->Toggle(true);
	int x_pos, y_pos;
	if (y_size == 0) x_pos=buttons;
	else x_pos = buttons / y_size, y_pos = buttons % y_size;
	item[x_pos + y_pos*x_size] = wid;
	buttons++;
};
void RadioButton::PressCallback(void* pointer, Button* from) {
	RadioButton* wid = (RadioButton*)pointer;
	int i;
	for (i=0; i<wid->x_size*wid->y_size; i++) {
		if (from == wid->item[i]) {
			wid->SetValue(i);
			return;
		}
	}
	return;
}
void RadioButton::SetValueImpl(int new_value) {
	int i;
	for (i=0; i<x_size*y_size; i++) {
		Button* wid = dynamic_cast<Button*>(item[i]);
		if (wid) {
			if (i == new_value) wid->Toggle(true);
			else wid->Toggle(false);
		}
	}
}
