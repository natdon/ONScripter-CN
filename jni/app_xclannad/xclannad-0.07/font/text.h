/*
 * Copyright (c) 2004  Kazunori "jagarl" Ueno
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

#ifndef __TEXT_H__
#define __TEXT_H__

#include<vector>
#include<string>

struct TextElem {
	typedef enum {glyph, escape, color, size} ElemType;
	typedef enum { ret, pos_reset, name_start, name_end, ruby_start, ruby_startruby, ruby_end} EscapeType;
	ElemType type;
	union Impl {
		struct {
			int code;
		} Glyph;
		struct {
			EscapeType type;
		} Escape;
		struct {
			unsigned char r, g, b;
		} Color;
		struct {
			float scale;
		} Size;
	} impl;
};
			
struct TextStream {
	std::vector<TextElem> container;
	typedef std::vector<TextElem>::iterator Iterator;
	enum {sjis, euc} kanji_type;

	std::string Save(void);
	void Load(const std::string&);

	void SetSize(double size);
	void SetColor(unsigned char r, unsigned char g, unsigned char b);
	void SetDefaultColor(unsigned char r, unsigned char g, unsigned char b);
	void InsertColor(int begin_pos, int end_pos, unsigned char r, unsigned char g, unsigned char b);
	void Clear(void);
	void Add(const char* str);
	void AddReturn(void);
	void AddName(const char* name);
	void AddRuby(const char* str, const char* ruby);

	void RemoveName(char* name, int namelen);

	TextStream(void);
	TextStream& operator =(const TextStream& from) {
		container = from.container;
		kanji_type = from.kanji_type;
		return *this;
	}
};

namespace XKFont {
	class Glyph;
	class Font;
};

struct TextGlyph {
	int x,y;
	enum Flag { Group = 1, LineEnd = 2, Kinsoku = 4, PhraseEnd = 8} flag;
	unsigned char r, g, b;
	bool is_rev;
	XKFont::Glyph* glyph;
};

struct TextGlyphStream : public std::vector<TextGlyph> {
	XKFont::Font* font;
	int width(void);
	int height(void);
	void SetColor(int r, int g, int b);
	void SetReverse(bool rev);
};

#endif /* __TEXT_H__ */
