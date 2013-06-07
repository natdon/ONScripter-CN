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


#ifndef __RECT_H__
#define __RECT_H__

#include<vector>

struct Rect {
	int lx, rx; // x = [lx,rx)
	int ty, by; // y = [ty,by)

	Rect(int x1, int y1);
	Rect(int x1, int y1, int x2, int y2);
	Rect(const Rect& rect);
	Rect(const class Surface& rect);
	Rect(const class TextGlyph& glyph);
	void intersect(const Rect& rect);
	bool is_crossed(const Rect& rect) const;
	bool is_inner(const Rect& inner_rect) const;
	bool is_nearly_inner(const Rect& inner_rect, int delta) const;
	void join(const Rect& rect);
	void rmove(int add_x, int add_y);
	/** Subtracts rect from this. The resulting area is the set of pixels contained in this but not in the rect.
	  * result will be push_backed to ret_array.
	  */
	void subtract(const Rect& rect, std::vector<Rect>& ret_array) const;
	bool point_in(int x, int y) const;
	bool empty(void) const {
		return (lx == rx) || (ty == by);
	}
	int width(void) const {
		return rx-lx;
	}
	int height(void) const {
		return by-ty;
	}
};

inline bool operator ==(const Rect& r1, const Rect& r2) {
	return (r1.lx == r2.lx && r1.rx == r2.rx && r1.ty == r2.ty && r1.by == r2.by);
}
inline bool Rect::point_in(int x, int y) const {
	return (lx <= x && x < rx && ty <= y && y < by);
}

struct Color {
	int r,g,b,a;
	Color(int _r, int _g, int _b) : r(_r),g(_g),b(_b),a(0xff) {}
	Color(int _r, int _g, int _b, int _a) : r(_r),g(_g),b(_b),a(_a) {}
};
#endif
