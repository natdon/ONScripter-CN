#ifndef __MENUITEM_H__
#define __MENUITEM_H__

#include"widget.h"

#define MenuItem WidMenuItem
#define MenuRadioButton WidMenuRadioButton
#define MenuScale WidMenuScale

// 必要なもの：
//	
//	ラベル：surfaceも貼れた方がよいが、まあいいや
//	ラベル位置：ボタンの上、もしくは右
//		右の場合：右中央へjustify するので、ラベル全体の幅と右マージンが必要
//		上の場合：左下へjustify するので、左マージンと下マージンが必要

//	内容：x,y個数を指定、その中で中央割り当て

//		ラベル位置情報：上なら左下にjustify, 右なら右中央にjustify
//		なので、下へのマージンと上へのマージン、
//	ラベル＋四角ボタン（チェックボックス）
//		ボタンが押されるとdeactivateされる
//
//	ラジオボタン
//		テキストボタンの集合体。x,y の個数を指定
//	スケール

struct MenuItem : PicWidget {
	int x_size, y_size;
	int menu_width, menu_height;
	int lb_width, lb_right, lb_left, lb_bottom;
	PicWidget* label;
	int* value_ptr;
	std::vector<PicWidget*> item;
	typedef std::vector<PicWidget*>::iterator iterator;
	MenuItem(PicContainer* parent, const Rect& r_orig, int x_size, int y_size, int* value_ptr);
	void SetLabelLeft(PicWidget* lb, const Rect& min_rect, const Rect& min_margin);
	void SetLabelTop(PicWidget* lb, const Rect& left_margin, const Rect& bottom_margin);
	void pack(void);
	void activate(void);
	void deactivate(void);
	// callback
	typedef void (*SetFunc)(void* pointer, MenuItem* widget);
	SetFunc set_func;
	void* set_pointer;
	void SetValue(int new_value);
	virtual void SetValueImpl(int new_value) {};
};

struct RadioButton : MenuItem {
	Event::Container& container;
	PicContainer* parent;
	int text_size;
	Rect button_rect;
	int buttons;
	Color fore_color, pressed_color, back_color;
	RadioButton(Event::Container& container, PicContainer* parent, const Rect& r_orig, int x_size, int y_size, int* value_ptr, const Rect& button_r, int text_size, const Color& fore, const Color& pressed, const Color& back);
	void Add(const char* s, bool is_center = true);
	void Add(const char* s, const Color& fore, const Color& pressed, const Color& back, bool is_center = true);
	static void PressCallback(void* pointer, Button* from);
	void SetValueImpl(int new_value);
};

#endif /* __MENUITEM_H__ */
