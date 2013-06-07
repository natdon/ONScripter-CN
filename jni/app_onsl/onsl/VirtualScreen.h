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

#ifndef NONS_VIRTUALSCREEN_H
#define NONS_VIRTUALSCREEN_H

#include "Common.h"
#include "ThreadManager.h"
#include "ErrorCodes.h"
#include "Functions.h"
#include "Image.h"
#include "SDLhead.h"
#include <string>
#include <set>

#define FILTER_EFFECT_F(name)          \
	void name(                         \
		ulong effectNo,                \
		NONS_Color color,              \
		const NONS_ConstSurface &src,  \
		const NONS_ConstSurface &rule, \
		const NONS_Surface &dst,       \
		NONS_LongRect area             \
	)
#define ASYNC_EFFECT_INIT_F(name) \
	void *name(ulong effectNo)
#define ASYNC_EFFECT_F(name)          \
	bool name(                        \
		ulong effectNo,               \
		const NONS_ConstSurface &src, \
		const NONS_Surface &dst,      \
		void *userData                \
	)
#define ASYNC_EFFECT_UNINIT_F(name) \
	void name(ulong effectNo,void *userData)
typedef FILTER_EFFECT_F((*filterFX_f));
typedef ASYNC_EFFECT_INIT_F((*asyncInit_f));
typedef ASYNC_EFFECT_F((*asyncEffect_f));
typedef ASYNC_EFFECT_UNINIT_F((*asyncUninit_f));

struct asyncFXfunctionSet{
	asyncInit_f initializer;
	asyncEffect_f effect;
	asyncUninit_f uninitializer;
	asyncFXfunctionSet(asyncInit_f a,asyncEffect_f b,asyncUninit_f c)
		:initializer(a),effect(b),uninitializer(c){}
};

struct pipelineElement{
	enum{
		MONOCHROME=0,
		NEGATIVE=1
	};
	ulong effectNo;
	NONS_Color color;
	NONS_Surface rule;
	std::wstring ruleStr;
	pipelineElement(){}
	pipelineElement(ulong effectNo,const NONS_Color &color,const std::wstring &rule,bool loadRule);
	pipelineElement(const pipelineElement &o){ *this=o; }
	pipelineElement(TiXmlElement *);
	void operator=(const pipelineElement &);
	TiXmlElement *save(const char *override_name=0);
};

#define VIRTUAL 0
#define PRE_ASYNC 2
#define REAL 3
#define OVERALL_FILTER 0
#define INTERPOLATION 1
#define ASYNC_EFFECT 2

/*
Surface processing pipeline:

x*         = x points to its own surface
x -> y     = x points to y
x -(y)-> z = x is processed by y onto z

                     Has the
                        |
 size of virtual screen |      size of real screen
                        |
0 ---------------------------------------------------> 3
0*-(filter)------------------------------------------> 3
0*---------------(interpolation)---------------------> 3
0*-(filter)-> 1*-(interpolation)---------------------> 3
0 -------------------------------> 2*-(async effect)-> 3
0*-(filter)----------------------> 2*-(async effect)-> 3
0*---------------(interpolation)-> 2*-(async effect)-> 3
0*-(filter)-> 1*-(interpolation)-> 2*-(async effect)-> 3
*/

class NONS_VirtualScreen{
	static const size_t screens_s=4;
	static const size_t usingFeature_s=screens_s-1;
	NONS_CrippledSurface screens[screens_s];
	NONS_Mutex mutex;
	NONS_Rect outRect;
	bool fullscreen;
	void init_fullscreen();
public:
	ulong post_filter,
		pre_inter,
		post_inter;
	bool usingFeature[usingFeature_s];
	NONS_Rect inRect;
	float x_multiplier,
		y_multiplier,
		x_divisor,
		y_divisor;
	NONS_Surface::public_interpolation_f normalInterpolation;

	NONS_Thread asyncEffectThread;
	bool killAsyncEffect;
	ulong aeffect_no,
		aeffect_freq;
	std::vector<filterFX_f> filters;
	std::vector<asyncInit_f> initializers;
	std::vector<asyncEffect_f> effects;
	std::vector<asyncUninit_f> uninitializers;
	std::vector<pipelineElement> filterPipeline;

	NONS_VirtualScreen(ulong w,ulong h);
	NONS_VirtualScreen(ulong iw,ulong ih,ulong ow,ulong oh);
	~NONS_VirtualScreen();
	void blitToScreen(NONS_Surface &src,NONS_LongRect *srcrect,NONS_LongRect *dstrect);
	NONS_DECLSPEC void updateScreen(ulong x,ulong y,ulong w,ulong h,bool fast=1);
	void updateWholeScreen(bool fast=1){
		this->updateScreen(0,0,(ulong)this->inRect.w,(ulong)this->inRect.h,fast);
	}
	NONS_Surface get_screen(){ return this->screens[VIRTUAL]; }
	NONS_Surface get_preasync_surface(){ return this->screens[PRE_ASYNC]; }
	NONS_Surface get_real_screen(){ return this->screens[REAL]; }
	//If 0, to window; if 1, to fullscreen; if 2, toggle.
	bool toggleFullscreen(uchar mode=2);
	SDL_Surface *toggleFullscreenFromVideo(const NONS_Surface &s);
	template <typename T>
	float convertX(T x){
		return this->convertW(x)+this->outRect.x;
	}
	template <typename T>
	float convertY(T y){
		return this->convertH(y)+this->outRect.y;
	}
	template <typename T>
	float unconvertX(T x){
		return ((float)x-this->outRect.x)/this->x_multiplier;
	}
	template <typename T>
	float unconvertY(T y){
		return ((float)y-this->outRect.y)/this->y_multiplier;
	}
	template <typename T>
	float convertW(T w){
		return (float)w*this->x_multiplier;
	}
	template <typename T>
	float convertH(T h){
		return (float)h*this->y_multiplier;
	}
	template <typename T>
	float unconvertW(T w){
		return (float)w/this->x_multiplier;
	}
	template <typename T>
	float unconvertH(T h){
		return (float)h/this->y_multiplier;
	}
	std::wstring takeScreenshot(const std::wstring &filename=L"");
	void initEffectList();
	ErrorCode callEffect(ulong effectNo,ulong frequency);
	void stopEffect();
	ErrorCode applyFilter(ulong effectNo,const NONS_Color &color,const std::wstring &rule);
	void changeState(bool switchAsyncState,bool switchFilterState);
	void printCurrentState();
	bool get_fullscreen(){
		NONS_MutexLocker ml(this->mutex);
		return this->fullscreen;
	}
	TiXmlElement *save_async_fx(const char *override_name=0);
	void load_async_fx(TiXmlElement *,const char *name=0);
	TiXmlElement *save_filter_pipeline(const char *override_name=0);
	void load_filter_pipeline(TiXmlElement *,const char *name=0);
	const NONS_Rect &get_outRect(){ return this->outRect; }
};

FILTER_EFFECT_F(effectMonochrome);
FILTER_EFFECT_F(effectNegative);
#endif
