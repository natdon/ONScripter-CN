/*
 * Copyright (c) 2004  Kazunori "jagarl" Ueno
 * Copyright (c) 2000, 2001 Yuki Sawada
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

#ifndef __FONT_TYPES_H__
#define __FONT_TYPES_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include<vector>

class TextHorizLayout;
class TextStream;
class TextGlyphStream;

namespace XKFont {
class Font;
class Face;
class Peer;
class Glyph;
class Cache;

struct Font {
public:
	Font(const char* fontname, int size);
	~Font();
	Face* FaceLoad(double scale);
	int vsize;
private:
	class FontImpl* pimpl;
};

struct Face {
public:
	Face(const char *name, int index, int hsize, int vsize);
	~Face();
	Glyph* GlyphLoad(unsigned int code);

private:
	Cache* cache;
	typedef std::vector<Peer*>::iterator iterator;
	std::vector<Peer*> peer;
};

struct Peer {
	Peer(void) {}
	virtual ~Peer() {};
	virtual bool GlyphCreate(unsigned int code, Glyph* glyph) = 0;
};

struct Glyph {
  int bitmap_left;
  int bitmap_top;
  struct _bitmap {
    int width;
    int rows;
    unsigned char *buffer;
    _bitmap() : buffer(0) {}
    ~_bitmap() { delete[] buffer;}
  } bitmap;
#if 0
  struct _metrics {
    int ascender;
    int descender;
  } metrics;
#endif
  struct _advance {
    int x;
    int y;
  } advance;
  Glyph() : bitmap() {}
  ~Glyph() {}
};

struct HorizLayout {
	HorizLayout(const char* fontname, int size);
	~HorizLayout();
	void Layout(::TextStream& stream, ::TextGlyphStream& glyph, std::vector<int>& lineheights, int width);
	::TextGlyphStream Layout(const char* str, int width, int r=0xff, int g=0xff, int b=0xff);
private:
	Font* font;
	class ::TextHorizLayout* pimpl;
};

}; /* end of namespace XKFont */

#endif
