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


#include "rect.h"

using namespace std;

inline int MAX(int a, int b) {
	if (a>b) return a;
	return b;
}
inline int MIN(int a, int b) {
	if (a>b) return b;
	return a;
}

Rect::Rect(int x1, int y1) {
	lx = rx = x1;
	ty = by = y1;
}
Rect::Rect(int x1, int y1, int x2, int y2) {
	lx = MIN(x1,x2);
	rx = MAX(x1,x2);
	ty = MIN(y1,y2);
	by = MAX(y1,y2);
}
Rect::Rect(const Rect& r) {
	lx = r.lx;
	rx = r.rx;
	ty = r.ty;
	by = r.by;
}

bool Rect::is_inner(const Rect& inner_rect) const {
	Rect r = *this;
	r.intersect(inner_rect);
	return r == inner_rect;
}
bool Rect::is_nearly_inner(const Rect& inner_rect, int delta) const {
	Rect r = *this;
	r.lx -= delta;
	r.ty -= delta;
	r.rx += delta;
	r.by += delta;
	r.intersect(inner_rect);
	return r == inner_rect;
}
bool Rect::is_crossed(const Rect& rect) const {
	Rect r = *this;
	r.intersect(rect);
	return !(r.empty());
}
void Rect::intersect(const Rect& r) {
	if (lx > r.rx) rx = lx;
	else if (rx < r.lx) lx = rx;
	else {
		lx = MAX(lx, r.lx);
		rx = MIN(rx, r.rx);
	}

	if (ty > r.by) by = ty;
	else if (by < r.ty) ty = by;
	else {
		ty = MAX(ty, r.ty);
		by = MIN(by, r.by);
	}
}
void Rect::join(const Rect& r) {
	lx = MIN(lx, r.lx);
	rx = MAX(rx, r.rx);
	ty = MIN(ty, r.ty);
	by = MAX(by, r.by);
}
void Rect::rmove(int add_x, int add_y) {
	lx += add_x;
	rx += add_x;
	ty += add_y;
	by += add_y;
}
void Rect::subtract(const Rect& rect, vector<Rect>& ret_array) const {
	Rect r = *this;
	r.intersect(rect);
	if (r.empty()) { // not intersect the rects
		ret_array.push_back(*this);
		return;
	}
	if (r ==*this) { // identical; no rect rests
		return;
	}
	// push top area
	if (ty != r.ty) {
		ret_array.push_back(Rect(lx, ty, rx, r.ty));
	}
	// push bottom area
	if (by != r.by) {
		ret_array.push_back(Rect(lx, r.by, rx, by));
	}
	// push left area
	if (lx != r.lx) {
		ret_array.push_back(Rect(lx, r.ty, r.lx, r.by));
	}
	// push right area
	if (rx != r.rx) {
		ret_array.push_back(Rect(r.rx, r.ty, rx, r.by));
	}
	return;
}
