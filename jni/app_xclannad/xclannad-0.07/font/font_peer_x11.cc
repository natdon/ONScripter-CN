/*
 * Copyright (c) 2004  Kazunor "jagarl" Ueno
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

#include <stdlib.h>

#include "font.h"
#include "font_peer.h"

#if USE_X11

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <stdio.h>
#include<iostream>

#include<vector>
#include<map>
//#include<iostream>
#include<sstream>
#include <string>
#include <stdexcept>

using namespace std;

namespace XKFont {
inline int read_little_endian_int(const char* buf) {
	const unsigned char *p = (const unsigned char *) buf;
	return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

/***********************************
**
** Fontinfo / FontSetInfo
**
**   fontset から特定 pixel size を持つ
**   別のfontsetを作成するためのクラス
*/
struct FontInfo {
	std::map<int, string> fontlist;
	FontInfo(Display* display, const char* fontname_orig);
	string Search(int pixsize);
};
struct FontSetInfo {
	std::vector<FontInfo*> fontlist;
	FontSetInfo(Display* display, const char* fontset_orig);
	string Search(int pixsize);
	~FontSetInfo();
};


/***********************************
**
** Methods of Fontinfo / FontSetInfo
**
*/
FontInfo::FontInfo(Display* display, const char* fontname_orig) {
	/* フォントの大きさ関係の情報を消去 */
	int i;
	char* fontname = new char[strlen(fontname_orig)+50];
	int minus_count = 0; bool is_skip = false; int fc = 0;
	for (i=0; fontname_orig[i]!=0; i++) {
		if (fontname_orig[i] == '-') {
			minus_count++;
			if (minus_count >= 7 && minus_count <= 12) {
				fontname[fc++] = '-';
				fontname[fc++] = '*';
				is_skip = true;
			} else {
				is_skip = false;
			}
		}
		if (! is_skip) fontname[fc++] = fontname_orig[i];
	}
	/* フォント情報を得る */
	fontname[fc] = 0;
	int count;
	char** fontnamelist = XListFonts(display, fontname, 100, &count);
	for (i=0; i<count; i++) {
		char* curfont = fontnamelist[i];
		/* fontname から pixel size 情報を得る */
		int j; int minus_count = 0;
		for (j=0; curfont[j] != 0; j++) {
			if (curfont[j] == '-') minus_count++;
			if (minus_count == 7) {
				int pixsize = atoi(curfont+j+1);
				if (fontlist.find(pixsize) == fontlist.end()) {
					fontlist[pixsize] = string(curfont);
				}
				break;
			}
		}
	}
	/* 検索に失敗した場合、とりあえず fontname を入れておく */
	if (fontlist.find(0) == fontlist.end()) {
		fontlist[0] = string(fontname);
	}
	XFreeFontNames(fontnamelist);
	delete[] fontname;
	return;
}
string FontInfo::Search(int pixsize) {
	int i;
	/* pixsize に近いフォントが(あれば)帰す */
	if (fontlist.find(pixsize) != fontlist.end()) return fontlist[pixsize];
	for (i=1; i<4; i++) {
		if (fontlist.find(pixsize-i) != fontlist.end()) return fontlist[pixsize-i];
		if (fontlist.find(pixsize+i) != fontlist.end()) return fontlist[pixsize+i];
	}
	/* 見つからない：fontlist[0] を加工して帰す */
	/* pt/xres/yres などのフィールドに '-0-' というのがあれば '-*-'に変換
	** pixsize は与えられた pixsize にする
	*/
	string basefont_s = fontlist[0];
	const char* basefont = basefont_s.c_str();
	char* retfont = new char[strlen(basefont)+50];
	int minus_count = 0; int rc = 0; bool is_skip = false;
	for (i=0; basefont[i] != 0; i++) {
		if (basefont[i] == '-') {
			minus_count++;
			is_skip = false;
			if (minus_count == 7) {
				sprintf(retfont+rc, "-%d", pixsize);
				rc = strlen(retfont);
				is_skip = true;
			} else if (minus_count > 7 && minus_count <= 12) {
				if (basefont[i+1] == '0' && basefont[i+2] == '-') {
					retfont[rc++]='-';
					retfont[rc++]='*';
					is_skip = true;
				}
			}
		}
		if (! is_skip) retfont[rc++] = basefont[i];
	}
	retfont[rc] = 0;
	string retfont_str = string(retfont);
	delete[] retfont;
	return retfont_str;
}

FontSetInfo::FontSetInfo(Display* display, const char* fontset_orig) {
	char* fontset = new char[strlen(fontset_orig)+1];
	strcpy(fontset, fontset_orig);
	char* cur = fontset;
	while(strchr(cur, ',')) {
		char* font = cur;
		cur = strchr(cur, ',');
		*cur++ = '\0';
		fontlist.push_back(new FontInfo(display, font));
	}
	fontlist.push_back(new FontInfo(display, cur));
	delete[] fontset;
	return;
}
FontSetInfo::~FontSetInfo() {
	std::vector<FontInfo*>::iterator it;
	for (it=fontlist.begin(); it != fontlist.end(); it++) {
		delete (*it);
	}
	return;
}
string FontSetInfo::Search(int pixsize) {
	stringstream s;
	std::vector<FontInfo*>::iterator it;
	for (it=fontlist.begin(); it != fontlist.end(); it++) {
		if (it != fontlist.begin()) s << ",";
		s << (*it)->Search(pixsize);
	}
	s<<ends;
	return string(s.str());
}

/****************************************
**
** FontPeerX11
*/
Display* PeerX11::display = 0;
void PeerX11::InitDisplay(Display* _d) {
	/* d = ((GdkWindowPrivate*)(top_window.gdkobj()))->xdisplay; */
	display = _d;
}

void PeerX11::OpenDisplay(void) {
	if (display) return;

	char* display_name = getenv("DISPLAY");
	if (display_name == 0) display_name = ":0";

	display = XOpenDisplay(display_name);

	if (display == 0) {
		string err = string("XKFont::PeerX11:OpenDisplay() : Cannot open X display ") + display_name;
		throw std::invalid_argument(err);
	}
}

inline int MAX(int a, int b) {
	if (a > b) return a;
	else return b;
}

PeerX11::PeerX11(const char* fontname, int index, int fontsize, int _vsize) :
	fontset(0), gc(0), canvas(0), image(0), colortable(0) {
	OpenDisplay();

	int scr = DefaultScreen(display);
	Window w = RootWindow(display, scr);
	Colormap cmap = DefaultColormap(display, scr);
	visual = DefaultVisual(display, scr);

	if (visual->c_class != TrueColor  && visual->c_class != DirectColor) {
		string err = "XKFont::PeerX11:PeerX11() : No supported Color mode of X : neither TrueColor nor DirectColor";
		throw std::runtime_error(err);
	}
	/* 色の初期化 */
	white = visual->red_mask | visual->green_mask | visual->blue_mask;
	black = 0;
	if (visual->green_mask == 0) {
		string err = "XKFont::PeerX11:PeerX11() : Invalid Visual on X";
		throw std::runtime_error(err);
	}
	shift = 0;
	mask = visual->green_mask;
	while(mask & 0x01) { shift++; mask >>= 1; }

	int tablesize = mask+1;
	colortable = new int[tablesize];
	int i; for (i=0; i< tablesize; i++) {
		colortable[i] = i*255/tablesize;
	}
XSupportsLocale();

	/* font 読み込み */
	FontSetInfo fsinfo(display,fontname);
	string fontset_name = fsinfo.Search(fontsize);
	char** missing_cl; int missing_cc; char* def_s;
printf("fontset %s\n",fontset_name.c_str());
	fontset = XCreateFontSet(display, fontset_name.c_str(), &missing_cl, &missing_cc, &def_s);

	if (fontset == 0) {
		delete[] colortable;
		string err = string("XKFont::PeerX11:PeerX11() : Cannot create fontset ");
		err += fontset_name; err += " (font name "; err += fontname; err += ")";
		throw std::invalid_argument(err);
	}

	if (missing_cc != 0) {
		cerr << "XKFont::PeerX11:PeerX11() : Cannot found some fonts in the fontset"<<endl;
		cerr << "   fontset: "<<fontset_name<<endl;
		cerr << "   not found fonts:"<<endl;
		int i; for (i=0; i<missing_cc; i++) {
			cerr << "        " << missing_cl[i] << endl;
		}
	}

	XFontSetExtents* extents = XExtentsOfFontSet(fontset);

	width = extents->max_ink_extent.width;
	height = extents->max_ink_extent.height;

	/* calculate ascent / descent */
	XFontStruct** font_structs; char** font_names;
	int num_fonts = XFontsOfFontSet(fontset, &font_structs, &font_names);
printf("locale %s\n",XLocaleOfOM(XOMOfOC(fontset)));

	ascent = 0;
	for (i=0; i<num_fonts; i++) {
		ascent = MAX(ascent, font_structs[i]->ascent);
	}
	
	/* 描画用の pixmap を作成 */
	XGCValues gc_values; unsigned int gc_values_mask;
	gc_values.function = GXcopy;
	gc_values.fill_style = FillSolid;
	gc_values.arc_mode = ArcPieSlice;
	gc_values.subwindow_mode = ClipByChildren;
	gc_values.graphics_exposures = False;
	gc_values.foreground = white;
	gc_values.background = black;
	gc_values_mask = GCFunction | GCFillStyle | GCArcMode | GCSubwindowMode | GCGraphicsExposures | GCForeground | GCBackground;
	gc = XCreateGC(display, w, gc_values_mask, &gc_values);

	canvas = XCreatePixmap(display, w, width, height, DefaultDepth(display, scr));

	/* イメージ転送用の image の作成 */
	int ignore;
	use_shm = false;
	if (XShmQueryExtension(display) == True) {
		x_shm_info.shmid = -1;
		x_shm_info.shmaddr = (char*)-1;
		image = XShmCreateImage(display, visual, DefaultDepth(display, scr), ZPixmap, NULL, &x_shm_info, width, height);
		if (image) {
			x_shm_info.shmid = shmget(IPC_PRIVATE, image->bytes_per_line*image->height, IPC_CREAT | 0600);
			if (x_shm_info.shmid == -1) {
				XDestroyImage(image);
				image = 0;
				goto no_shm;
			}
			x_shm_info.readOnly = False;
			x_shm_info.shmaddr = (char*) shmat(x_shm_info.shmid, 0, 0);
			image->data = x_shm_info.shmaddr;
			if (x_shm_info.shmaddr == (char*) -1) {
				XDestroyImage(image);
				shmctl(x_shm_info.shmid, IPC_RMID, 0);
				image = 0;
				goto no_shm;
			}
			XShmAttach(display, &x_shm_info);
			XSync(display, False);
			shmctl(x_shm_info.shmid, IPC_RMID, 0);
			use_shm = true;
		}
	}
no_shm:
	if (image == 0) {
		use_shm = false;
		image = XCreateImage(display, visual, DefaultDepth(display, scr), ZPixmap, 0, 0, width, height, 32, 0);
		image->data = (char*)malloc(image->bytes_per_line * image->height);
		if (image->data == 0) {
			XDestroyImage(image);
			image = 0;
			throw bad_alloc();
		}
	}
Glyph g;
GlyphCreate(0xa1a2,&g);
GlyphCreate(0xa4a3,&g);
}

PeerX11::~PeerX11() {
	if (display) {
		if (fontset) XFreeFontSet(display, fontset);
		if (gc) XFlushGC(display, gc);
		if (canvas) XFreePixmap(display, canvas);
		if (image) {
			if (use_shm) XShmDetach(display, &x_shm_info);
			XDestroyImage(image);
			if (use_shm) shmdt(x_shm_info.shmaddr);
		}
		if (colortable) delete[] colortable;
	}
}

bool PeerX11::GlyphCreate(unsigned int code, Glyph* glyph) {
	XRectangle ink, logic;

	char str[3]={0,0,0};

	if ( (code>>8)&0xff){
		str[0]=code>>8;
		str[1]=code & 0xff;
	} else {
		str[0]=code;
	}

	XmbTextExtents(fontset, str, strlen(str),&ink,&logic);
	int cwidth = logic.width;
	XmbDrawImageString(display, canvas, fontset, gc, 0, -logic.y, str, strlen(str));
	if (use_shm) {
		XShmGetImage(display, canvas, image, 0, 0, AllPlanes);
	} else {
		XGetSubImage(display, canvas, 0, 0, width, height, AllPlanes, ZPixmap, image, 0, 0);
	}
	XSync(display, False);
	glyph->bitmap.buffer = new unsigned char[logic.width*logic.height];
	int i;
	unsigned char* mem = (unsigned char*) image->data;
	unsigned char* dest = glyph->bitmap.buffer;
	int bpp = image->bytes_per_line/width;
	int bpl = image->bytes_per_line;
	for (i=0; i<logic.height; i++) {
		unsigned char* m = mem;
		int j; for (j=0; j<cwidth; j++) {
			*dest = colortable[((read_little_endian_int((char*)m))>>shift) & mask];
			dest++;
			m += bpp;
		}
		mem += bpl;
	}
	glyph->bitmap_left = logic.x;
	glyph->bitmap_top  = -logic.y;
	glyph->bitmap.width = logic.width;
	glyph->bitmap.rows = logic.height;
	glyph->advance.x = logic.width + 1;
	glyph->advance.y = logic.height+ 1;

	return true;
}
};

#endif /* USE_X11 */
