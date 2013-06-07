#ifndef __SURFACE_H__
#define __SURFACE_H__

#include<SDL.h>
#include"rect.h"

struct Surface : public SDL_Surface {
	char* mem(const Rect& r) {
		return (char*)(pixels) + r.ty * pitch + r.lx * format->BytesPerPixel;
	}
	char* mem_end(const Rect& r) {
		return (char*)(pixels) + (r.by-1) * pitch + r.rx * format->BytesPerPixel;
	}
};

#endif /* __SURFACE_H__ */
