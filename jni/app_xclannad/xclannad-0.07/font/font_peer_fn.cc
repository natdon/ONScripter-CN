/*
 * Copyright (c) 2001 Yuki Sawada
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
 *
 * This code uses some parts of AVG32 for Macintosh by Kenjo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

using namespace std;

#include "font.h"
#include "font_peer.h"

namespace XKFont {

#define FN_DAT_SIZE  2544768

PeerFn::PeerFn(const char *name, int index, int hsize, int vsize) : buffer(0)
{
  FILE *fp = 0;

  buffer = new unsigned char[FN_DAT_SIZE];
  fp = fopen(name, "rb");
  if (!fp) {
	delete[] buffer;
	buffer = 0;
	string err = string("XKFont::PeerFn::PeerFn : Cannot open font file ")+name;
	throw std::invalid_argument(err);
  }
  fread(buffer, 1, FN_DAT_SIZE, fp);
  fclose(fp);
  
  return;
}


PeerFn::~PeerFn() {
	delete[] buffer;
}

static unsigned int
font_glyph_fn_codeconv_euc_to_jis(unsigned int euc)
{
  unsigned int h, l;

  h = (euc >> 8) & 0xff;
  l = euc & 0xff;

  if (h < 0x81) {
    l = h;
    h = 0;
  } else if (l == 0x8e)
    h = 0;
  else {
    h -= 0x80;
    l -= 0x80;
  }
  
  return (h << 8) | l;
}

bool
PeerFn::GlyphCreate(unsigned int code, Glyph* glyph)
{
  unsigned char *p1, *p2;
  unsigned int h, l, offset;
  int x, y;

  l = font_glyph_fn_codeconv_euc_to_jis(code);
  l -= 0x2121;
  h = l >> 8;
  l &= 0xff;
  offset = (h * 0x5e + l) * 12 * 24;
  if (offset > FN_DAT_SIZE - 12 * 24)
    offset = 0;

  glyph->bitmap_left = 0;
  glyph->bitmap_top = 21;
  glyph->bitmap.width = 24;
  glyph->bitmap.rows = 24;

#if 0
  glyph->metrics.ascender = private->vsize - 4;
  glyph->metrics.descender = -4;
#endif
  glyph->advance.x = 24 + 1;
  glyph->advance.y = 24 + 1;

  glyph->bitmap.buffer = new unsigned char[24*24];

  p1 = glyph->bitmap.buffer;
  p2 = buffer + offset;
  for (y = 0; y < 24; y++) {
    for (x = 0; x < 12; x++) {
      unsigned char c = ~*p2++;
      unsigned char c1;
      c1 = (c)   & 0x0f; *p1++ = (c1<<4) | c1;
      c1 = (c>>4)& 0x0f; *p1++ = (c1<<4) | c1;
    }
  }
  return true;
}

} /* end of namespace XKFont */
