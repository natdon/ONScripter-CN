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

#include"font/font.h"
#include"font/text.h"
#include"rect.h"
#include"SDL.h"
#include"surface.h"
#include<stdio.h>

Rect DSurfaceRenderText(TextGlyphStream::iterator start, TextGlyphStream::iterator end, const Rect& srcrect,
	Surface* dst, const Rect& dstrect); // 文字描画
void DSurfaceFill(Surface* src, const Rect& rect, int r, int g, int b, int a); // クリア
void DSurfaceFillA(Surface* src, const Rect& rect, int r, int g, int b, int a); // テキストウィンドウ背景の設定
void DSurfaceMove(Surface* src_o, const Rect& srcrect, Surface* dst_o, const Rect& dstrect); // コピー
void DSurfaceBlitAlpha(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o, const unsigned char* alpha, const Rect& alpharect);
void DSurfaceBlitSaturate(Surface* src_o, const Rect& srcrect, Surface* dst_o, const Rect& dstrect, unsigned char alpha);
void DSurfaceBlitMultiply(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o);

#ifndef ALPHA_MAX
#define ALPHA_MAX 255
#endif

Rect::Rect(const TextGlyph& letter) {
	lx = letter.x + letter.glyph->bitmap_left;
	rx = lx + letter.glyph->bitmap.width;

	ty = letter.y - letter.glyph->bitmap_top;
	by = ty + letter.glyph->bitmap.rows;
}

Rect::Rect(const class Surface& so) {
	SDL_Surface* s = (SDL_Surface*)(&so);
	lx = 0; ty = 0;
	rx = s->w; by = s->h;
}
inline Rect _Rect(const SDL_Rect& r) {
	return Rect(r.x, r.y, r.x+r.w, r.y+r.h);
};
inline SDL_Rect SDLed(const Rect& rect) {
	SDL_Rect r;
	r.x = rect.lx;
	r.y = rect.ty;
	r.w = rect.rx-rect.lx;
	r.h = rect.by-rect.ty;
	return r;
}

Rect DSurfaceRenderText(TextGlyphStream::iterator start, TextGlyphStream::iterator end, const Rect& srcrect,
	Surface* dst_o, const Rect& dstrect) {

	SDL_Surface* dst = (SDL_Surface*)dst_o;
	Rect confine_to(srcrect);
	Rect dst_clip(_Rect(dst->clip_rect));
	Rect drawn_rect(0,0,0,0);

	int dif_x = dstrect.lx - srcrect.lx;
	int dif_y = dstrect.ty - srcrect.ty;

	dst_clip.rmove(-dif_x, -dif_y);
	confine_to.intersect(dst_clip);

	TextGlyphStream::iterator it;
	SDL_PixelFormat& fmt = *dst->format;
	int ashift = fmt.Ashift - fmt.Aloss; int amask = fmt.Amask;
	int dbpl = dst->pitch;
	int dbpp = fmt.BytesPerPixel;

	SDL_LockSurface(dst);
	for (it=start; it!=end; it++) {
		Rect letter_r(*it);
		letter_r.intersect(confine_to);
		if (letter_r.empty()) continue;
		drawn_rect.join(letter_r);

		int w = letter_r.width();
		int h = letter_r.height();
		int sbpl = it->glyph->bitmap.width;

		unsigned char* smem = it->glyph->bitmap.buffer +
			(letter_r.ty - (it->y-it->glyph->bitmap_top)) * sbpl +
			(letter_r.lx - (it->x+it->glyph->bitmap_left));
		char* dmem = (char*)dst->pixels + 
			(letter_r.ty + dif_y) * dbpl +
			(letter_r.lx + dif_x) * dbpp;
		Uint32 pixel = ((int(it->r)<<fmt.Rshift)&fmt.Rmask) |((int(it->g)<<fmt.Gshift)&fmt.Gmask) |((int(it->b)<<fmt.Bshift)&fmt.Bmask);
		int i,j;
		if (!it->is_rev) {
			if (dbpp == 4) {
				for (i=0; i<h; i++) {
					for (j=0; j<w; j++) {
						int alpha = smem[j];
						((Uint32*)dmem)[j] = pixel | ((alpha << ashift)&amask);
					}
					smem += sbpl; dmem += dbpl;
				}
			} else if (dbpp == 2) {
				for (i=0; i<h; i++) {
					for (j=0; j<w; j++) {
						int alpha = smem[j];
						((Uint16*)dmem)[j] = pixel | ((alpha << ashift)&amask);
					}
					smem += sbpl; dmem += dbpl;
				}
			} else abort();
		} else { /* reversed */
			if (dbpp == 4) {
				for (i=0; i<h; i++) {
					for (j=0; j<w; j++) {
						int alpha = 255 - smem[j];
						((Uint32*)dmem)[j] = pixel | ((alpha << ashift)&amask);
					}
					smem += sbpl; dmem += dbpl;
				}
			} else if (dbpp == 2) {
				for (i=0; i<h; i++) {
					for (j=0; j<w; j++) {
						int alpha = 255 - smem[j];
						((Uint16*)dmem)[j] = pixel | ((alpha << ashift)&amask);
					}
					smem += sbpl; dmem += dbpl;
				}
			} else abort();
		}
	}
	SDL_UnlockSurface(dst);
	drawn_rect.rmove(dif_x, dif_y);
	return drawn_rect;
}
void DSurfaceFill(Surface* src_o, const Rect& rect, int r, int g, int b, int a) {
	SDL_Rect sr = SDLed(rect); 
	SDL_Surface* src = (SDL_Surface*)src_o;
	SDL_FillRect( src, &sr, SDL_MapRGBA(src->format, r, g, b, a));
	return;
}

static void clip_rect(Rect& srcrect, Rect& dstrect, SDL_Surface* dstsurf) {
	Rect confine_to(srcrect);
	Rect dst_clip(_Rect(dstsurf->clip_rect));

	int dif_x = dstrect.lx - srcrect.lx;
	int dif_y = dstrect.ty - srcrect.ty;

	dst_clip.rmove(-dif_x, -dif_y);
	confine_to.intersect(dst_clip);

	srcrect = confine_to;
	dstrect = confine_to;
	dstrect.rmove(dif_x, dif_y);
}

void DSurfaceMove(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o) {

	SDL_Surface* dst = (SDL_Surface*)dst_o;
	SDL_Surface* src = (SDL_Surface*)src_o;

	if (dst->format->BytesPerPixel != src->format->BytesPerPixel) return; // RGB変換はできない

	Rect srcrect(srcrect_o), dstrect(dstrect_o);
	clip_rect(srcrect, dstrect, dst);

	SDL_LockSurface(dst); SDL_LockSurface(src);
	int sbpl = src->pitch;
	int dbpl = dst->pitch;
	int bpp = dst->format->BytesPerPixel;
	int width = bpp * srcrect.width();
	int height = srcrect.height();
	char* smem = src_o->mem(srcrect);
	char* dmem = dst_o->mem(dstrect);
	char* smem_end = src_o->mem_end(srcrect);
	char* dmem_end = dst_o->mem_end(dstrect);

	// メモリに重なりがあり、src が上位側の場合、コピー方向を逆転する
	if (smem < dmem && dmem < smem_end) {
		int i,j;
		for (i=0; i<height; i++) {
			char* s = smem_end; char* d = dmem_end;
			for (j=0; j<width; j++)
				*--d = *--s;
			dmem_end -= dbpl; smem_end -= sbpl;
		}
	} else {
		int i;
		for (i=0; i<height; i++) {
			memcpy(dmem, smem, width);
			dmem += dbpl; smem += sbpl;
		}
	}
	SDL_UnlockSurface(dst);
	SDL_UnlockSurface(src);
	return;
}

void DSurfaceFillA(Surface* dsto, const Rect& rect, int r, int g, int b, int a) {
	SDL_Surface* dst = (SDL_Surface*)dsto;
	SDL_LockSurface( dst);
	int dbpl = dst->pitch;
	int bpp = dst->format->BytesPerPixel;
	int width = rect.width();
	int height = rect.height();
	int amask = dst->format->Amask;
	int ashift = dst->format->Ashift - dst->format->Aloss;

	char* dmem = (char*)(dst->pixels) + rect.ty*dbpl + rect.lx*bpp;
	unsigned int pixel = SDL_MapRGBA(dst->format, r, g, b, 0);
	unsigned int pixela = SDL_MapRGBA(dst->format, r, g, b, a);
	a += a>>7; /* 0-256 にする */
	int i,j;
	for (i=0; i<height; i++) {
		char* d = dmem;
		if (bpp == 4) {
			for (j=0; j<width; j++) {
				int alpha = (*(Uint32*)d)>>ashift;
				if (alpha == 0) (*(Uint32*)d) = pixel;
				else if (alpha == 255) *((Uint32*)d) = pixela;
				else { *((Uint32*)d) = pixel | ((((alpha*a)>>8)<<ashift)&amask); }
				d += bpp;
			}
		} else if (bpp == 2) {
			for (j=0; j<width; j++) {
				int alpha = (*(Uint16*)d)>>ashift;
				if (alpha == 0) *((Uint16*)d) = pixel;
				else if (alpha == 255) *((Uint16*)d) = pixela;
				else { *((Uint16*)d) = pixel | ((((alpha*a)>>8)<<ashift)&amask); }
				d += bpp;
			}
		}
		dmem += dbpl;
	}
	SDL_UnlockSurface( dst);
	return;
}

#define ASHIFT 24
#define CMASK1 0xff00ff
#define CMASK2 0x00ff00

inline void blit_pixel(Uint32* dmem, Uint32* smem, const unsigned char* amem, bool use_srcalpha) {
	Uint32 d = *dmem;
	Uint32 s = *smem;
	Uint32 as = s>>ASHIFT;
	if (as == 255 || (!use_srcalpha) ) {
		as = *amem;
	} else {
		as += as>>7; /* 0-0xff -> 0-0x100 */
		as *= *amem;
		as >>= 8;
	}
	as += as>>7;
	Uint32 s1 = s & CMASK1;
	Uint32 d1 = d & CMASK1;
	d1 = (d1 + (((s1-d1) * as) >> 8)) & CMASK1;
	s &= CMASK2;
	d &= CMASK2;
	d = (d + (((s-d) * as) >> 8)) & CMASK2;
	*dmem = d1 | d | 0xff000000;
}
static void blit_line(Uint32* dmem, Uint32* smem, const unsigned char* amem,int ax0, int ax1, int awidth, int aj0, int aj1, bool use_srcalpha) {
	int j;
	int ax = ax0;
	const unsigned char* a = amem + ax0;
	Uint32* d = dmem;
	Uint32* s = smem;
	if (awidth == 1) { //  わりとよくあるので最適化
		for (j=aj0; j < aj1; j++) {
			blit_pixel(d++, s++, amem, use_srcalpha);
		}
	} else {
		for (j=aj0; j < aj1; j++) {
			for (; ax<awidth; ax++)
				blit_pixel(d++, s++, a++, use_srcalpha);
			ax = 0;
			a = amem;
		}
		for (; ax < ax1; ax++) blit_pixel(d++, s++, a++, use_srcalpha);
	}
}

void DSurfaceBlitAlpha(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o, const unsigned char* alpha, const Rect& alpharect) {
	SDL_Surface* dst = (SDL_Surface*)dst_o;
	SDL_Surface* src = (SDL_Surface*)src_o;
	SDL_PixelFormat& fmt = *dst->format;

	Rect srcrect(srcrect_o), dstrect(dstrect_o);
	clip_rect(srcrect, dstrect, dst);

	int awidth = alpharect.width();
	int aheight = alpharect.height();
	if (awidth == 0 || aheight == 0) return;
	int ax0 = srcrect.lx % awidth;
	int ay0 = srcrect.ty % aheight;
	int aj0 = srcrect.lx / awidth;
	int ai0 = srcrect.ty / aheight;
	int ax1 = srcrect.rx % awidth;
	int ay1 = srcrect.by % aheight;
	int aj1 = srcrect.rx / awidth;
	int ai1 = srcrect.by / aheight;

	SDL_LockSurface(dst);
	SDL_LockSurface(src);
	int dbpl = dst->pitch;
	int sbpl = src->pitch;
	int bpp = dst->format->BytesPerPixel;

	char* dmem = dst_o->mem(dstrect);
	char* smem = src_o->mem(srcrect);

	const unsigned char* amem = alpha + ay0 * awidth;
	int i;
	int ay = ay0;
	for (i = ai0; i < ai1; i++) {
		for (; ay < aheight; ay++) {
			if (src->format->Amask == 0)
				blit_line( (Uint32*)dmem, (Uint32*)smem, amem,ax0, ax1, awidth, aj0, aj1, false);
			else
				blit_line( (Uint32*)dmem, (Uint32*)smem, amem,ax0, ax1, awidth, aj0, aj1, true);
			amem += awidth; dmem += dbpl; smem += sbpl;
		}
		ay = 0; amem = alpha;
	}
	for (; ay < ay1; ay++) {
		if (src->format->Amask == 0)
			blit_line( (Uint32*)dmem, (Uint32*)smem, amem,ax0, ax1, awidth, aj0, aj1, false);
		else
			blit_line( (Uint32*)dmem, (Uint32*)smem, amem,ax0, ax1, awidth, aj0, aj1, true);
		amem += awidth; dmem += dbpl; smem += sbpl;
	}
	SDL_UnlockSurface(src);
	SDL_UnlockSurface(dst);
	return;
}
void DSurfaceBlitSaturate(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o, unsigned char alpha) {
	SDL_Surface* dst = (SDL_Surface*)dst_o;
	SDL_Surface* src = (SDL_Surface*)src_o;

	if (dst->format->BytesPerPixel != src->format->BytesPerPixel) return; // RGB変換はできない

	Rect srcrect(srcrect_o), dstrect(dstrect_o);
	clip_rect(srcrect, dstrect, dst);

	SDL_LockSurface(dst); SDL_LockSurface(src);
	int sbpl = src->pitch;
	int dbpl = dst->pitch;
	int bpp = dst->format->BytesPerPixel;
	int width = srcrect.width();
	int height = srcrect.height();
	char* smem = src_o->mem(srcrect);
	char* dmem = dst_o->mem(dstrect);
	char* smem_end = src_o->mem_end(srcrect);
	char* dmem_end = dst_o->mem_end(dstrect);

	SDL_PixelFormat& fmt = *dst->format;
	int rshift = fmt.Rshift - fmt.Rloss; int rmask = fmt.Rmask;
	int gshift = fmt.Gshift - fmt.Gloss; int gmask = fmt.Gmask;
	int bshift = fmt.Bshift - fmt.Bloss; int bmask = fmt.Bmask;
	int allmask = rmask | gmask | bmask;
	int i;
	for (i=0; i<height; i++) {
		char* d = dmem; char* s = smem;
		int j; for (j=0; j<width; j++) {
			Uint32 sd = *(Uint32*)s;
			Uint32 dd = *(Uint32*)d;
			if (sd&allmask) {
				Uint32 sr = (sd&rmask)>>rshift;
				Uint32 sg = (sd&gmask)>>gshift;
				Uint32 sb = (sd&bmask)>>bshift;
				if (alpha != ALPHA_MAX) {
					sr = (sr*alpha)>>8;
					sg = (sg*alpha)>>8;
					sb = (sb*alpha)>>8;
				}
				Uint32 dr = sr + ((dd&rmask)>>rshift);
				Uint32 dg = sg + ((dd&gmask)>>gshift);
				Uint32 db = sb + ((dd&bmask)>>bshift);
				if (dr > 255) dr = 255;
				if (dg > 255) dg = 255;
				if (db > 255) db = 255;
				*(Uint32*)d = ((dr<<rshift)&rmask)| ((dg<<gshift)&gmask)| ((db<<bshift)&bmask);
			}
			s += bpp; d += bpp;
		}
		dmem += dbpl; smem += sbpl;
	}
	SDL_UnlockSurface(src);
	SDL_UnlockSurface(dst);
	return;
}
void DSurfaceBlitMultiply(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o) {
	SDL_Surface* dst = (SDL_Surface*)dst_o;
	SDL_Surface* src = (SDL_Surface*)src_o;

	Rect srcrect(srcrect_o), dstrect(dstrect_o);
	clip_rect(srcrect, dstrect, dst);

	SDL_LockSurface(dst);
	SDL_LockSurface(src);
	int dbpl = dst->pitch;
	int sbpl = src->pitch;
	int bpp = dst->format->BytesPerPixel;
	int width = srcrect.width();
	int height = srcrect.height();
	char* dmem = dst_o->mem(dstrect);
	char* smem = src_o->mem(srcrect);

	SDL_PixelFormat& fmt = *dst->format;
	/* dst の 0-255 を 0-pixel に変換する（積算） */
	int i;
	int table1[256], table2[256], table3[256];
	Uint32 src_pixel = *(Uint32*)smem;
	src_pixel |= 0xff000000;
	int c1=src_pixel&255, c2=(src_pixel>>8)&255, c3=(src_pixel>>16)&255;
	for (i=0; i<256; i++) {
		int n = i + (i>>7);
		table1[i] = (c1*n)>>8;
		table2[i] = (c2*n)&0xff00;
		table3[i] = ((c3*n)<<8) & 0xff0000;
	}
	int err_count = 0;
	for (i=0; i<height; i++) {
		Uint32* d = (Uint32*)dmem;
		Uint32* s = (Uint32*)smem;
		int j; for (j=0; j<width; j++) {
			Uint32 dd = *d;
			Uint32 ss = *s;
			if (ss == src_pixel) {
				*(Uint32*)d = table1[ dd & 0xff] | table2[ (dd>>8) & 0xff] | table3[ (dd>>16) & 0xff];
			} else if ( ((ss^src_pixel) & 0xffffff) == 0) { // r,g,b of ss == src_pixel
				Uint32 as = ss>>ASHIFT;
				as += as>>7;
				ss = table1[ dd & 0xff] | table2[ (dd>>8) & 0xff] | table3[ (dd>>16) & 0xff];
				Uint32 s1 = ss&CMASK1;
				Uint32 d1 = dd&CMASK1;
				d1 = (d1 + (((s1-d1) * as) >> 8)) & CMASK1;
				ss &= CMASK2;
				dd &= CMASK2;
				dd = (dd + (((ss-dd) * as) >> 8)) & CMASK2;
				*(Uint32*)d = d1 | dd | 0xff000000;
			} else err_count++;
			d++; s++;
		}
		dmem += dbpl; smem += sbpl;
	}
	if (err_count) {
		fprintf(stderr,"multipy_blit : surface does not have unique color(%08x); %d pixels in width %d, height %d\n",
			src_pixel, err_count, width, height);
	}
	SDL_UnlockSurface(dst);
	SDL_UnlockSurface(src);
	return;
}
