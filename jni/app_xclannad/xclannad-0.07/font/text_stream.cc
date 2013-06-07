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

#include "text.h"
#include "codeconv.h"
#include<stdio.h>

/************************************************************************
**
**	TextStream
**
*/

TextStream::TextStream(void) {
	kanji_type = euc;
}
void TextStream::SetSize(double scale) {
	TextElem elem;
	elem.type = TextElem::size;
	elem.impl.Size.scale = scale;
	container.push_back(elem);
}
void TextStream::SetColor(unsigned char r, unsigned char g, unsigned char b) {
	TextElem elem;
	elem.type = TextElem::color;
	elem.impl.Color.r = r;
	elem.impl.Color.g = g;
	elem.impl.Color.b = b;
	container.push_back(elem);
}
void TextStream::InsertColor(int begin_pos, int end_pos, unsigned char r, unsigned char g, unsigned char b) {
	TextElem elem;
	if (begin_pos < 0) begin_pos = 0;
	if (begin_pos > container.size()) begin_pos = container.size();
	if (end_pos < 0) end_pos = 0;
	if (end_pos > container.size()) end_pos = container.size();
	if (begin_pos >= end_pos) return;

	elem.type = TextElem::color;
	elem.impl.Color.r = 255;
	elem.impl.Color.g = 255;
	elem.impl.Color.b = 255;
	container.insert(container.begin()+end_pos, elem);
	elem.impl.Color.r = r;
	elem.impl.Color.g = g;
	elem.impl.Color.b = b;
	container.insert(container.begin()+begin_pos, elem);
	Iterator it = container.begin()+begin_pos+1;
	Iterator end = container.begin()+end_pos+1;
	for (; it != end; it++) {
		if (it->type == TextElem::color) {
			TextElem& elem = *it;
			if (elem.impl.Color.r == 255 && elem.impl.Color.g == 255 && elem.impl.Color.b == 255) {
				elem.impl.Color.r = r;
				elem.impl.Color.g = g;
				elem.impl.Color.b = b;
			}
		}
	}
}
void TextStream::Clear(void) {
	container.clear();
}
void TextStream::Add(const char* str) {
	TextElem elem;
	for (; *str; str++) {
		if (*str >= 0x20) {
			elem.type = TextElem::glyph;
			elem.impl.Glyph.code = *str;
		} else if (*str < 0 && str[1] != 0) {
			elem.type = TextElem::glyph;
			elem.impl.Glyph.code = ((int(*(unsigned char*)str))<<8) | int(*(unsigned char*)(str+1));
			if (kanji_type == sjis) elem.impl.Glyph.code = codeconv_sjis_to_euc(elem.impl.Glyph.code);
			str++;
		} else {
			continue;
		}
		container.push_back(elem);
	}
}
void TextStream::AddReturn(void) {
	TextElem elem;
	elem.type = TextElem::escape;
	elem.impl.Escape.type = TextElem::ret;
	container.push_back(elem);
}
void TextStream::AddName(const char* str) {
	TextElem elem;
	elem.type = TextElem::escape;
	elem.impl.Escape.type = TextElem::name_start;
	container.push_back(elem);
	Add(str);
	elem.impl.Escape.type = TextElem::name_end;
	container.push_back(elem);
}

void TextStream::AddRuby(const char* str, const char* ruby) {
	TextElem elem;
	elem.type = TextElem::escape;
	elem.impl.Escape.type = TextElem::ruby_start;
	container.push_back(elem);
	Add(str);
	elem.impl.Escape.type = TextElem::ruby_startruby;
	container.push_back(elem);
	Add(ruby);
	elem.impl.Escape.type = TextElem::ruby_end;
	container.push_back(elem);
}

void TextStream::RemoveName(char* name, int namelen) {
	Iterator it;
	for (it = container.begin(); it != container.end(); it++) {
		if (it->type == TextElem::escape && it->impl.Escape.type == TextElem::name_start) {
			// 行頭の名前？
			int pt = it - container.begin();
			Iterator name_start = it;
			for (; it != container.end(); it++) {
				if (it->type == TextElem::escape && it->impl.Escape.type == TextElem::name_end) break;
			}
			if (it != container.end()) {
				// 名前が見つかったので削除
				if (name) { // 保存先があるなら保存する
					Iterator name_end = it;
					int pos = 0;
					namelen--;
					for (it=name_start; it != name_end; it++) {
						if (it->type == TextElem::glyph) {
							unsigned int code = it->impl.Glyph.code;
							if (code < 0x100) {
								if (pos < namelen) name[pos++] = code;
							} else {
								if (pos < namelen) name[pos++] = code>>8;
								if (pos < namelen) name[pos++] = code;
							}
						}
					}
					name[pos] = 0;
					name = 0; // 最初に出た名前のみ保存する
				}
				it++;
				container.erase(name_start, it);
				it = container.begin() + pt;
			}
		}
		for (;it != container.end(); it++) {
			if (it->type == TextElem::escape && it->impl.Escape.type == TextElem::ret) break;
		}
		if (it == container.end()) break;
	}
}

/************************************************************************
**
**	TextStream::Save
**
*/

/* escape sequence : '=XX' ; x < 0x30 || 'x>0x39 && x<0x40' (symbols) */
/*  !"#$%&'()*+,-./:;<=>? */

void TextStream::Load(const std::string& from_s) {
	/* kigou :  !"#$%&'()*+,-./:;<=>? */
	const char* s = from_s.c_str();
	container.clear();
	if (*s == '=' && *s == 'e') {kanji_type = euc; s+=2;}

	TextElem e;
	while(s && *s) {
		e.type = TextElem::glyph;
		if (*s == '=') {
			if (s[1] >= 'A' && s[1] <= 'Q' && s[2] >= 'A' && s[2] <= 'Q') { // symbols
				e.type = TextElem::glyph;
				e.impl.Glyph.code = (s[1]-'A')*16 + (s[2]-'A');
				s += 3;
			} else if (s[1] == 'x') {
				e.type = TextElem::escape;
				e.impl.Escape.type = TextElem::EscapeType(s[2]-'A');
				s += 3;
			} else if (s[1] == 'c') {
				int c;
				e.type = TextElem::color;
				sscanf(s+2, "%x", &c);
				e.impl.Color.r = (c>>16)&0xff;
				e.impl.Color.g = (c>>8)&0xff;
				e.impl.Color.b = (c)&0xff;
				s = strchr(s, '.');
				if (s) s++;
			} else if (s[1] == 's') {
				e.impl.Size.scale = TextElem::size;
				sscanf(s+2, "%f", &e.impl.Size.scale);
				s = strchr(s, '.');
				if (s) s++;
			} else {
				fprintf(stderr,"TextStream::Load(): Cannot convert text-stream from Serialized-data\n");
				s++;
			}
		} else {
			if (*s < 0) { // kanji-code
				if (s[1] == 0) break;
				if (s[1] >= 0 && s[1] < 0x40) break; // not EUC nor SJIS
				e.type = TextElem::glyph;
				e.impl.Glyph.code = codeconv_sjis_to_euc(int(*(unsigned char*)(s))*0x100 + int(*(unsigned char*)(s+1)));
				s += 2;
			} else { // ascii-code
				if (s[0] < 0x30) break; // must be escaped
				if (s[0] > 0x39 && s[0] < 0x40) break; // must be escaped
				e.type = TextElem::glyph;
				e.impl.Glyph.code = s[0];
				s++;
			}
		}
		container.push_back(e);
	}
}

std::string TextStream::Save(void) {
	/* kigou :  !"#$%&'()*+,-./:;<=>? */
	Iterator it;
	std::string ret_s;
	char buf_orig[1024];
	char* buf = buf_orig;
	for (it=container.begin(); it != container.end(); it++) {
		TextElem& e = *it;
		switch(e.type) {
		case TextElem::glyph: {
			int code = e.impl.Glyph.code;
			if (code < 0x30 || (code > 0x39 && code < 0x40)) {
				*buf++ = '=';
				*buf++ = (code/0x10) + 'A';
				*buf++ = (code%0x10) + 'A';
			} else {
				code = codeconv_euc_to_sjis(code); // save file の漢字コードはSJIS
				*buf++ = code/256;
				*buf++ = code%256;
			}
			break;
			}
		case TextElem::escape:
			sprintf(buf, "=x%c", e.impl.Escape.type+'A');
			buf += 3;
			break;
		case TextElem::color:
			sprintf(buf, "=c%x.",int(e.impl.Color.r)*256*256+int(e.impl.Color.g)*256+e.impl.Color.b);
			buf += strlen(buf);
			break;
		case TextElem::size:
			sprintf(buf, "=s%f.", e.impl.Size.scale);
			buf += strlen(buf);
			break;
		}
		if (buf-buf_orig > 1000) {
			*buf = 0;
			ret_s += buf_orig;
			buf = buf_orig;
		}
	}
	*buf = 0;
	ret_s += buf_orig;
	buf = buf_orig;
	return ret_s;
}

/************************************************************************
**
**	TextGlyphStream
**
*/

void TextGlyphStream::SetColor(int r, int g, int b) {
	iterator it;
	for (it=begin(); it != end(); it++) {
	//	if (it->r == 255 && it->g == 255 && it->b == 255) {
			it->r = r;
			it->g = g;
			it->b = b;
	//	}
	}
}
void TextGlyphStream::SetReverse(bool is_rev) {
	iterator it;
	for (it=begin(); it != end(); it++) {
		it->is_rev = is_rev;
	}
}
