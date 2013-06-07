/*
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
#include <stdio.h>
#include"font.h"
#include"font_peer.h"
#include <map>
#include <string>
#include <iostream>

namespace XKFont {

class Cache : public std::map<unsigned int, Glyph*> {
public:
	Cache() {}
	~Cache() {
		iterator it;
		for (it = begin(); it != end(); it++)
			delete (it->second);
	}
};

Face::Face(const char *name_orig, int index, int hsize, int vsize)
{
	cache = new Cache;

	/* name: ';' 区切りで複数指定可能 */
	char* name = new char[strlen(name_orig)+1];
	while(*name_orig != 0) {
		const char* next_name = strchr(name_orig, ';');
		if (next_name) {
			strncpy(name, name_orig, next_name - name_orig);
			name[next_name-name_orig] = 0;
			name_orig = next_name + 1;
		} else {
			strcpy(name, name_orig);
			name_orig += strlen(name_orig);
		}
		if (strstr(name, "fn.dat")) {
			peer.push_back(new PeerFn(name, index, hsize, vsize));
		} else if (strstr(name, ".ttf") || strstr(name, ".ttc")) {
			peer.push_back(new PeerFt2(name, index, hsize, vsize));
#if USE_X11
		} else {
			peer.push_back(new PeerX11(name, index, hsize, vsize));
#endif
		}
	}
	return;
}

Face::~Face() {
	delete cache;
}

Glyph*
Face::GlyphLoad(unsigned int code) {
	if (cache->count(code))
		return (*cache)[code];

	Glyph* g = new Glyph;
	iterator it;
	for (it=peer.begin(); it != peer.end(); it++) {
		if ( (*it)->GlyphCreate(code, g)) break;
	}
	if (it == peer.end()) {
		fprintf(stderr,"Cannot find glyph, code %04x\n",code);
		g->bitmap_left = 0;
		g->bitmap_top = 0;
		g->bitmap.width = 0;
		g->bitmap.rows = 0;
		g->bitmap.buffer = new unsigned char[1];
		g->bitmap.buffer[0] = 0;
		g->advance.x = 0;
		g->advance.y = 0;
	}
	(*cache)[code] = g;
	return g;
}

class FontImpl {
public:
	std::map<int, Face*> cache;
	std::string fontname;
	int size;
	~FontImpl();
};

FontImpl::~FontImpl() {
	std::map<int,Face*>::iterator it;
	for (it=cache.begin(); it!=cache.end(); it++) delete it->second;
}

Font::Font(const char* name, int size) {
	pimpl = new FontImpl;
	pimpl->fontname = name;
	pimpl->size = size;
	vsize = size;
};
Font::~Font() {
	delete pimpl;
}
Face*
Font::FaceLoad(double scale) {
	std::map<int, Face*>& cache = pimpl->cache;
	int size = int(scale * pimpl->size);
	if (cache.find(size) != cache.end()) return cache[size];
	try {
		Face* face = new Face(pimpl->fontname.c_str(), 0, size, size);
		cache[size] = face;
		return face;
	} catch(...) {
		std::cerr << "Cannot create font face; font "<<pimpl->fontname<<", size "<<size<<std::endl;
		/* 別の大きさを探す */
		int i;
		for (i=0; i<size; i++) {
			if (cache.find(size-i) != cache.end()) return cache[size-i];
			if (cache.find(size+i) != cache.end()) return cache[size+i];
			try {
				Face* face = new Face(pimpl->fontname.c_str(), 0, size-i, size-i);
				cache[size-i] = face;
				return face;
			} catch(...) {};
			try {
				Face* face = new Face(pimpl->fontname.c_str(), 0, size+i, size+i);
				cache[size+i] = face;
				return face;
			} catch(...) {};
		}
		/* 見つからない */
		throw;
	}
}

} /* namespace XKFont */
