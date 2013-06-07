/*
* Copyright (c) 2008-2011, Helios (helios.vmg@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, 
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the author may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*     * Products derived from this software may not be called "ONSlaught" nor
*       may "ONSlaught" appear in their names without specific prior written
*       permission from the author. 
*
* THIS SOFTWARE IS PROVIDED BY HELIOS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL HELIOS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NONS_Surface_H
#define NONS_Surface_H
#include "Common.h"
#include "Functions.h"
#include "tinyxml/tinyxml.h"
#include <cmath>

#define OVERLOAD_RELATIONAL_OPERATORS(macro)\
	macro(==)\
	macro(!=)\
	macro(<)\
	macro(<=)\
	macro(>=)\
	macro(>)
#define OVERLOAD_RELATIONAL_OPERATORS2(extra,macro)\
	macro(extra,==)\
	macro(extra,!=)\
	macro(extra,<)\
	macro(extra,<=)\
	macro(extra,>=)\
	macro(extra,>)

template <typename T>
struct NONS_SurfaceProperties_basic{
	T *pixels;
	//width and height
	ulong w,h,
	//size in bytes of a single scanline, including padding if it applies
		pitch,
	//size in bytes of the entire surface
		byte_count,
	//size in pixels of the entire surface; equivalent to w*h
		pixel_count,
	//frame count
		frames;
	//If (uchar *)p points to the start of a pixel,
	//p[offset[0]] is the red channel
	//p[offset[1]] is the green channel
	//p[offset[2]] is the blue channel
	//p[offset[3]] is the alpha channel
	uchar offsets[4];
	template <typename T2>
	operator NONS_SurfaceProperties_basic<T2>() const{
		NONS_SurfaceProperties_basic<T2> r;
		r.pixels=this->pixels;
		r.w=this->w;
		r.h=this->h;
		r.pitch=this->pitch;
		r.byte_count=this->byte_count;
		r.pixel_count=this->pixel_count;
		r.offsets[0]=this->offsets[0];
		r.offsets[1]=this->offsets[1];
		r.offsets[2]=this->offsets[2];
		r.offsets[3]=this->offsets[3];
		r.frames=this->frames;
		return r;
	}
	template <typename T2>
	bool same_format(const NONS_SurfaceProperties_basic<T2> &b){
		return
			this->offsets[0]==b.offsets[0] &&
			this->offsets[1]==b.offsets[1] &&
			this->offsets[2]==b.offsets[2] &&
			this->offsets[3]==b.offsets[3];
	}
};
typedef NONS_SurfaceProperties_basic<uchar> NONS_SurfaceProperties;
typedef NONS_SurfaceProperties_basic<const uchar> NONS_ConstSurfaceProperties;

void over_blend(
	const NONS_SurfaceProperties &dst,
	const NONS_LongRect &dst_rect,
	const NONS_ConstSurfaceProperties &src,
	const NONS_LongRect &src_rect,
	long alpha
);

class NONS_CrippledSurface;
struct NONS_Surface_Private;

union pixel{
	Uint8 array[4];
	Uint32 i32;
};

struct NONS_Color{
	Uint8 rgba[4];
	NONS_Color(Uint8 r,Uint8 g,Uint8 b,Uint8 a=0xFF){
		this->rgba[0]=r;
		this->rgba[1]=g;
		this->rgba[2]=b;
		this->rgba[3]=a;
	}
	NONS_Color(Uint32 a=0){
		*this=a;
	}
	const NONS_Color &operator=(Uint32 a){
		this->rgba[0]=(a&0xFF0000)>>16;
		this->rgba[1]=(a&0xFF00)>>8;
		this->rgba[2]=a&0xFF;
		this->rgba[3]=0xFF;
		return *this;
	}
	Uint32 to_rgb(){
		Uint32 r=
			(this->rgba[0]<<16)|
			(this->rgba[1]<<8)|
			this->rgba[2];
		return r;
	}
	Uint32 to_rgba() const{
		Uint32 r=
			(this->rgba[0]<<24)|
			(this->rgba[1]<<16)|
			(this->rgba[2]<<8)|
			this->rgba[3];
		return r;
	}
	void from_rgba(Uint32 a){
		this->rgba[0]=(a&0xFF000000)>>24;
		this->rgba[1]=(a&0xFF0000)>>16;
		this->rgba[2]=(a&0xFF00)>>8;
		this->rgba[3]=a&0xFF;
	}
	Uint32 to_native(uchar *format) const{
		pixel r;
		r.array[0]=this->rgba[format[0]];
		r.array[1]=this->rgba[format[1]];
		r.array[2]=this->rgba[format[2]];
		r.array[3]=this->rgba[format[3]];
		return r.i32;
	}
	bool operator==(const NONS_Color &b) const{
		uchar f[]={0,1,2,3};
		return this->to_native(f)==b.to_native(f);
	}
	bool operator!=(const NONS_Color &b) const{
		return !(*this==b);
	}
	NONS_Color operator*(double factor) const{
		NONS_Color r=*this;
		r.rgba[0]=Uint8(double(r.rgba[0])*factor);
		r.rgba[1]=Uint8(double(r.rgba[1])*factor);
		r.rgba[2]=Uint8(double(r.rgba[2])*factor);
		return r;
	}
	TiXmlElement *save(const char *override_name=0) const{
		TiXmlElement *color=new TiXmlElement(override_name?override_name:"color");
		color->SetAttribute("rgba",itoac(this->to_rgba()));
		return color;
	}
	NONS_Color(TiXmlElement *color){
		this->from_rgba(ato<Uint32>(color->QueryStringAttribute("rgba")));
	}
	static NONS_Color white,
		black,
		black_transparent,
		red,
		green,
		blue;
};

struct NONS_AnimationInfo{	
	enum TRANSPARENCY_METHODS{
		LEFT_UP='l',
		RIGHT_UP='r',
		COPY_TRANS='c',
		PARALLEL_MASK='a',
		SEPARATE_MASK='m'
	} method;
	ulong animation_length;
	/*
	If size==1, the first element contains how long each frame will stay on.
	Otherwise, element i contains how long will frame i stay on, plus the
	value of element i-1 if i>0.
	For example, for the string "<10,20,30>", the resulting contents will be
	{10,30,60}
	*/
	std::vector<ulong> frame_ends;
	enum LOOP_TYPE{
		SAWTOOTH_WAVE_CYCLE=0,
		SINGLE_CYCLE,
		TRIANGLE_WAVE_CYCLE,
		NO_CYCLE
	} loop_type;
	ulong animation_time_offset;
	int animation_direction;
	bool valid;
	static TRANSPARENCY_METHODS default_trans;

	NONS_AnimationInfo()
		:animation_length(1),
		animation_time_offset(0),
		valid(1){}
	NONS_AnimationInfo(const std::wstring &image_string);
	NONS_AnimationInfo(const NONS_AnimationInfo &b);
	NONS_AnimationInfo &operator=(const NONS_AnimationInfo &b);
	void parse(const std::wstring &image_string);
	void resetAnimation();
	long advanceAnimation(ulong msecs);
	long getCurrentAnimationFrame() const;
	const std::wstring &getFilename() const{
		return this->filename;
	}
	const std::wstring &getString() const{
		return this->string;
	}
	const std::wstring &getMaskFilename() const{
		return this->mask_filename;
	}
private:
	std::wstring filename;
	std::wstring string;
	std::wstring mask_filename;
};

class NONS_Matrix{
	double matrix[4];
public:
	NONS_Matrix(){
		this->matrix[0]=
		this->matrix[1]=
		this->matrix[2]=
		this->matrix[3]=0.0;
	}
	NONS_Matrix(double a,double b,double c,double d){
		this->matrix[0]=a;
		this->matrix[1]=b;
		this->matrix[2]=c;
		this->matrix[3]=d;
	}
	double determinant() const{
		return this->matrix[0]*this->matrix[3]-this->matrix[1]*this->matrix[2];
	}
	NONS_Matrix operator!() const{
		double a=1.0/this->determinant();
		return NONS_Matrix(
			a*this->matrix[3],
			a*-this->matrix[1],
			a*-this->matrix[2],
			a*this->matrix[0]
		);
	}
	NONS_Matrix operator*(const NONS_Matrix &m) const{
		return NONS_Matrix(
			this->matrix[0]*m.matrix[0]+this->matrix[1]*m.matrix[2],
			this->matrix[0]*m.matrix[1]+this->matrix[1]*m.matrix[3],
			this->matrix[2]*m.matrix[0]+this->matrix[3]*m.matrix[2],
			this->matrix[2]*m.matrix[1]+this->matrix[3]*m.matrix[3]
		);
	}
	const double &operator[](unsigned i)const{ return this->matrix[i]; }
	static NONS_Matrix rotation(double alpha){
		return NONS_Matrix(cos(alpha),-sin(alpha),sin(alpha),cos(alpha));
	}
	static NONS_Matrix scale(double x,double y){
		return NONS_Matrix(x,0,0,y);
	}
	static NONS_Matrix shear(double x,double y){
		return NONS_Matrix(1,x,y,1);
	}
};

class NONS_Surface;
typedef std::map<std::pair<ulong,ulong>,NONS_LongRect> optim_t;

//Reference-counted, thread-safe class that abstracts common image operations.
//Note: all surfaces use 32-bit RGBA pixels. The pixels are ordered in a
//left-to-right, top-to-bottom fashion.
class NONS_DECLSPEC NONS_ConstSurface{
protected:
	NONS_Surface_Private *data;
	friend class NONS_CrippledSurface;
	friend class NONS_Surface;
public:
	//Create a null surface.
	NONS_ConstSurface():data(0){}
	//Make a shallow copy of a surface. The pointed surface's reference count
	//gets incremented.
	NONS_ConstSurface(const NONS_ConstSurface &original);
	NONS_ConstSurface &operator=(const NONS_ConstSurface &);
	virtual ~NONS_ConstSurface();
	//Returns whether the object points to something valid.
	operator bool() const{ return !!this->data; }
	//Invalidates the object. The pointed surface's reference count gets
	//decremented.
	void unbind();
	//If the pointer is !0, returns a copy of the object the pointer points to.
	//Otherwise, returns a copy of the surface's rectangle.
	NONS_LongRect default_box(const NONS_LongRect *) const;
	//Call to perform pixel-wise read operations on the surface.
	void get_properties(NONS_ConstSurfaceProperties &sp) const;
	//Returns a copy of the surface's rectangle.
	NONS_LongRect clip_rect() const{ return this->default_box(0); }
	void save_bitmap(const std::wstring &filename,bool fill_alpha=0,bool threaded=1) const;
	void get_optimized_updates(optim_t &dst) const;

	//The following are pure functions; they don't have side effects (as far as
	//the calling object is concerned).

	//Returns an object that points to a deep copy of this object's surface.
	//The new surface has the same size and pixel data as the original.
	NONS_Surface clone() const;
	//Same as clone(), but doesn't copy the pixel data.
	NONS_Surface clone_without_pixel_copy() const;
	//Copies the surface while scaling it.
	NONS_Surface scale(double x,double y) const;
	//Copies the surface while resizing it.
	NONS_Surface resize(long x,long y) const;
	//Copies the surface while rotating it.
	NONS_Surface rotate(double alpha) const;
	//Copies the surface while applying a linear transformation to it.
	NONS_Surface transform(const NONS_Matrix &m,bool fast=0) const;
};

class NONS_FontCache;
struct NONS_StandardOutput;

NONS_LongRect GetBoundingBox(const std::wstring &str,NONS_FontCache &cache,const NONS_LongRect &limit);

//Midifiable version of NONS_ConstSurface. While NONS_ConstSurface references a
//read-only surface, NONS_Surface references a writable surface.
class NONS_DECLSPEC NONS_Surface:public NONS_ConstSurface{
	typedef void (*interpolation_f)(NONS_SurfaceProperties,NONS_Rect,NONS_SurfaceProperties,NONS_Rect,double,double);
	NONS_Surface(int){}
public:
	static const NONS_Surface null;
	//By default, create a reference to nothing.
	NONS_Surface(){}
	NONS_Surface(const NONS_Surface &a):NONS_ConstSurface(){
		this->data=0;
		*this=a;
	}
	//Create a surface from a NONS_CrippledSurface. See below.
	NONS_Surface(const NONS_CrippledSurface &original);
	//Create a surface from a width and height.
	NONS_Surface(ulong w,ulong h){
		this->data=0;
		this->assign(w,h);
	}
	//Create a surface from an existing file. The function will use the virtual
	//file system.
	NONS_Surface(const std::wstring &a){
		this->data=0;
		*this=a;
	}
	NONS_Surface(
		const std::wstring &str,
		NONS_FontCache &fc,
		const NONS_LongRect &limit,
		float center,
		const NONS_LongRect *bounding_box
	);
	NONS_Surface(
		ulong w,
		ulong h,
		const std::wstring &str,
		NONS_StandardOutput &so,
		const NONS_Color &col
	);
	void assign(ulong w,ulong h);
	NONS_Surface &operator=(const std::wstring &name);
	void get_properties(NONS_SurfaceProperties &sp) const;

	//Alpha-blend src over this surface
	//src_rect: reading from src will begin at (src_rect->x,src_rect->y), and at
	//          most src_rect->w*src_rect->h pixels will be read.
	//dst_rect: writing to dst will begin at (dst_rect->x,dst_rect->y).
	//          Members w and h are ignored.
	//alpha: src is blended with this opacity (255=opaque, 0=invisible [blending
	//       is skipped])
	//frame: this frame from src is blended onto dst. If frame is too large, its
	//       value is saturated.
	void over(const NONS_ConstSurface &src,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0);
	void over_with_alpha(const NONS_ConstSurface &src,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0,long alpha=255);
	void over_frame(const NONS_ConstSurface &src,ulong frame,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0);
	void over_frame_with_alpha(const NONS_ConstSurface &src,ulong frame,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0,long alpha=255);

	//Same as over(), but using the multiply operation. The parameters have the same meanings.
	void multiply(const NONS_ConstSurface &src,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0);
	void multiply_frame(const NONS_ConstSurface &src,ulong frame,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0);

	//Similar to over, but using direct pixel copy, instead of alpha blending.
	//Prefer this if you're sure you don't need alpha blending, as it's much
	//faster.
	void copy_pixels(const NONS_ConstSurface &src,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0);
	void copy_pixels_frame(const NONS_ConstSurface &src,ulong frame,const NONS_LongRect *dst_rect=0,const NONS_LongRect *src_rect=0);

	//Set all pixels on the surface to color.
	void fill(const NONS_Color &color);
	//Set the pixels in area to color.
	void fill(const NONS_LongRect area,const NONS_Color &color);
	//Same as fill(), but doesn't touch the alpha channel.
	void color(NONS_Color color);

	//If the object points to the screen, this updates it and locks access to it
	//while it's doing it.
	void update(ulong x=0,ulong y=0,ulong w=0,ulong h=0) const;

#define NONS_Surface_DECLARE_RELATIONAL_OP(type,op) bool operator op(const type &b) const;
	OVERLOAD_RELATIONAL_OPERATORS2(NONS_Surface,NONS_Surface_DECLARE_RELATIONAL_OP)

#define NONS_Surface_DECLARE_INTERPOLATION_F(name) \
	void name(                                     \
		NONS_Surface src,                          \
		NONS_Rect dst_rect,                        \
		NONS_Rect src_rect,                        \
		double x,                                  \
		double y                                   \
	)
#define NONS_Surface_DECLARE_INTERPOLATION_F_INTERNAL(extra,name) \
	void extra name(                                              \
		interpolation_f f,                                        \
		const NONS_Surface &src,                                  \
		const NONS_Rect &dst_rect,                                \
		const NONS_Rect &src_rect,                                \
		double x,                                                 \
		double y                                                  \
	)
	NONS_Surface_DECLARE_INTERPOLATION_F_INTERNAL(,interpolation);
	//These functions were designed for speed, not ease of use, and thus
	//they're rather complicated. Unless you're really hurting for speed,
	//consider using NONS_ConstSurface::scale(), which gives good quality in
	//general or NONS_ConstSurface::transform(), which is slightly faster.
	//Otherwise, see NONS_VirtualScreen::updateScreen() for an example.
	NONS_Surface_DECLARE_INTERPOLATION_F(NN_interpolation);
	NONS_Surface_DECLARE_INTERPOLATION_F(bilinear_interpolation);
	NONS_Surface_DECLARE_INTERPOLATION_F(bilinear_interpolation2);
	typedef void (NONS_Surface::*public_interpolation_f)(NONS_Surface,NONS_Rect,NONS_Rect,double,double);

	//Do not use:

	void divide_into_cells(ulong amount);

	//SDL-related:
	SDL_Surface *get_SDL_screen() const;
	static NONS_Surface assign_screen(SDL_Surface *,bool get_screen=1);
	static NONS_Surface get_screen();
	static void init_loader();
	static bool filelog_check(const std::wstring &string);
	static void filelog_writeout();
	static void filelog_commit();
	static void use_fast_svg(bool);
	static void set_base_scale(double x,double y);
};

//Class that implements a weak reference of sorts.
//While a surface is pointed to by a NONS_CrippledSurface, it won't be freed
//by the surface manager, but also it can't be read or written. Only a VERY
//limited amount of information can be obtained from it, such as its
//dimensions. In order to use it, the get_surface() method has to be called.
//This method performs a mutex lock if necessary. This guarantees that one,
//and ONLY ONE thread can possibly access the screen at any given time.
class NONS_CrippledSurface{
	NONS_Surface_Private *data;
	friend class NONS_Surface;
	NONS_Surface *inner;
public:
	NONS_CrippledSurface():data(0),inner(0){}
	NONS_CrippledSurface(const NONS_Surface &original);
	NONS_CrippledSurface(const NONS_CrippledSurface &original):data(0),inner(0){ *this=original; }
	const NONS_CrippledSurface &operator=(const NONS_CrippledSurface &);
	~NONS_CrippledSurface();
	void get_dimensions(ulong &w,ulong &h);
	bool good() const{ return !!this->data; }
	operator bool() const{ return this->good(); }
	template <typename T>
	NONS_BasicRect<T> get_dimensions(){
		ulong a,b;
		this->get_dimensions(a,b);
		return NONS_BasicRect<T>(0,0,(T)a,(T)b);
	}
	NONS_Surface get_surface() const{ return (this->inner)?*this->inner:NONS_Surface(*this); }
	void strong_bind();
	void unbind();
	OVERLOAD_RELATIONAL_OPERATORS2(NONS_CrippledSurface,NONS_Surface_DECLARE_RELATIONAL_OP)

	static void copy_surface(NONS_CrippledSurface &dst,const NONS_ConstSurface &src){
		NONS_Surface s=src.clone();
		dst=s;
		dst.strong_bind();
	}
};

template<typename T>
bool fix_rects(
		NONS_BasicRect<T> &dst1,
		NONS_BasicRect<T> &src1,
		const NONS_BasicRect<T> *dst0,
		const NONS_BasicRect<T> *src0,
		const NONS_ConstSurface &dst,
		const NONS_ConstSurface &src){
	NONS_BasicRect<T> dst_rect=NONS_BasicRect<T>(dst.default_box(0)),
		src_rect=NONS_BasicRect<T>(src.default_box(0));
	dst1=NONS_BasicRect<T>(dst.default_box(dst0));
	src1=NONS_BasicRect<T>(src.default_box(src0));
	src1=src1.intersect(src_rect);
	if (src1.w<=0 || src1.h<=0)
		return 0;
	T temp_x=dst1.x-src1.x,
		temp_y=dst1.y-src1.y;
	src1.x+=temp_x;
	src1.y+=temp_y;
	src1=src1.intersect(dst_rect);
	if (src1.w<=0 || src1.h<=0)
		return 0;
	src1.x-=temp_x;
	src1.y-=temp_y;
	dst1.w=src1.w;
	dst1.h=src1.h;
	dst1=dst1.intersect(dst_rect);
	if (dst1.w<=0 || dst1.h<=0)
		return 0;
	return 1;
}

#define _APPLY_ALPHA(c0,c1,a) ((((a)^0xFF)*(c1)+(a)*(c0))/255)
#if !defined _DEBUG
#define APPLY_ALPHA _APPLY_ALPHA
#else
inline uchar APPLY_ALPHA(ulong c0,ulong c1,ulong a){
	return (uchar)_APPLY_ALPHA(c0,c1,a);
}
#endif
#endif
