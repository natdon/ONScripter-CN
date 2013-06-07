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

#include "GFX.h"
#include "IOFunctions.h"
#include "ThreadManager.h"
#include "Plugin/LibraryLoader.h"
#include <cmath>
#include <iostream>

#if defined _DEBUG && !defined BENCHMARK_EFFECTS
#define BENCHMARK_EFFECTS
#endif

bool NONS_GFX::listsInitialized=0;
std::vector<filterFX_f> NONS_GFX::filters;
std::vector<transitionFX_f> NONS_GFX::transitions;

ulong NONS_GFX::effectblank=0;

NONS_GFX::NONS_GFX(ulong effect,ulong duration,const std::wstring *rule){
	this->effect=effect;
	this->duration=duration;
	if (rule)
		this->rule=*rule;
	this->type=TRANSITION;
	this->stored=0;
}

NONS_GFX::NONS_GFX(const NONS_GFX &b){
	*this=b;
}

NONS_GFX &NONS_GFX::operator=(const NONS_GFX &b){
	this->effect=b.effect;
	this->duration=b.duration;
	this->rule=b.rule;
	this->type=b.type;
	this->stored=b.stored;
	return *this;
}

void NONS_GFX::initializeLists(){
	NONS_GFX::listsInitialized=1;
	NONS_GFX::filters.push_back(effectMonochrome);
	NONS_GFX::filters.push_back(effectNegative);
	return;
	/*pluginLibraryFunction fp=(pluginLibraryFunction)pluginLibrary.getFunction("getFunctionPointers");
	if (!fp)
		return;
	{
		//1=Get filter effect function pointers.
		std::vector<filterFX_f> vec=*(std::vector<filterFX_f> *)fp((void *)1);
		NONS_GFX::filters.insert(NONS_GFX::filters.end(),vec.begin(),vec.end());
	}
	{
		//0=Get transition effect function pointers.
		std::vector<transitionFX_f> vec=*(std::vector<transitionFX_f> *)fp((void *)0);
		NONS_GFX::transitions.insert(NONS_GFX::transitions.end(),vec.begin(),vec.end());
	}*/
}

ErrorCode NONS_GFX::callEffect(
		ulong number,
		long duration,
		const std::wstring *rule,
		const NONS_ConstSurface &src,
		const NONS_Surface &dst0,
		NONS_VirtualScreen &dst){
	NONS_GFX effect(number,duration,rule);
	ErrorCode ret=effect.call(src,dst0,&dst);
	return ret;
}

ErrorCode NONS_GFX::callFilter(
		ulong number,
		const NONS_Color &color,
		const std::wstring &rule,
		const NONS_ConstSurface &src,
		const NONS_Surface &dst){
	NONS_GFX effect;
	effect.effect=number;
	effect.color=color;
	effect.rule=rule;
	effect.type=POSTPROCESSING;
	ErrorCode ret=effect.call(src,dst,0);
	return ret;
}

ErrorCode NONS_GFX::call(const NONS_ConstSurface &src,const NONS_Surface &dst0,NONS_VirtualScreen *dst){
	typedef void(NONS_GFX::*transitionFunction)(const NONS_ConstSurface &,const NONS_ConstSurface &,NONS_VirtualScreen &);
	static transitionFunction builtInTransitions[]={
		&NONS_GFX::effectNothing,		//0
		&NONS_GFX::effectOnlyUpdate,	//1
		&NONS_GFX::effectRshutter,		//2
		&NONS_GFX::effectLshutter,		//3
		&NONS_GFX::effectDshutter,		//4
		&NONS_GFX::effectUshutter,		//5
		&NONS_GFX::effectRcurtain,		//6
		&NONS_GFX::effectLcurtain,		//7
		&NONS_GFX::effectDcurtain,		//8
		&NONS_GFX::effectUcurtain,		//9
		&NONS_GFX::effectCrossfade,		//10
		&NONS_GFX::effectRscroll,		//11
		&NONS_GFX::effectLscroll,		//12
		&NONS_GFX::effectDscroll,		//13
		&NONS_GFX::effectUscroll,		//14
		&NONS_GFX::effectHardMask,		//15
		&NONS_GFX::effectMosaicIn,		//16
		&NONS_GFX::effectMosaicOut,		//17
		&NONS_GFX::effectSoftMask		//18
	};
	if (!NONS_GFX::listsInitialized)
		NONS_GFX::initializeLists();
	NONS_Surface ruleFile;
	if (this->rule.size())
		ruleFile=this->rule;
	if (this->type==TRANSITION){
		if (this->effect<=18){
			(this->*(builtInTransitions[this->effect]))(src,ruleFile,*dst);
		}else if (this->effect-19<NONS_GFX::transitions.size()){
			NONS_GFX::transitions[this->effect-19](this->effect,this->duration,src,ruleFile,*dst);
			if (!CURRENTLYSKIPPING && NONS_GFX::effectblank)
				waitNonCancellable(NONS_GFX::effectblank);
		}else
			return NONS_NO_EFFECT;
	}else{
		if (this->effect<NONS_GFX::filters.size())
			NONS_GFX::filters[this->effect](this->effect+1,this->color,src,NONS_Surface::null,dst0,src.clip_rect());
		else
			return NONS_NO_EFFECT;
	}
	return NONS_NO_ERROR;
}

void NONS_GFX::effectNothing(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){}

void NONS_GFX::effectOnlyUpdate(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	dst.get_screen().copy_pixels(src);
	dst.updateWholeScreen();
}

NONS_DECLSPEC bool effect_standard_check(NONS_LongRect &dst,const NONS_ConstSurface &s,NONS_VirtualScreen &d){
	NONS_Surface screen=d.get_screen();
	NONS_LongRect a=s.clip_rect(),
		b=screen.clip_rect();
	if (a.w!=b.w || a.h!=b.h)
		return 0;
	if (CURRENTLYSKIPPING){
		screen.over(s);
		screen.unbind();
		d.updateWholeScreen();
		return 0;
	}
	dst=a;
	return 1;
}

NONS_DECLSPEC void effect_epilogue(){
	if (!CURRENTLYSKIPPING && NONS_GFX::effectblank)
		waitNonCancellable(NONS_GFX::effectblank);
}

void NONS_GFX::effectRshutter(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_LongRect rect(0,0,1,src_rect.h);
	long shutterW=src_rect.w/40;
	EFFECT_INITIALIZE_DELAYS(shutterW);
	for (long a=0;a<shutterW;a++){
		EFFECT_ITERATION_PROLOGUE(a<shutterW-1,rect.w++;);
		rect.x=a-rect.w+1;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<40;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.x+=shutterW;
			}
		}
		dst.updateWholeScreen();
		rect.w=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectLshutter(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_LongRect rect(0,0,1,src_rect.h);
	long shutterW=src_rect.w/40;
	EFFECT_INITIALIZE_DELAYS(shutterW);
	for (long a=shutterW-1;a>=0;a--){
		EFFECT_ITERATION_PROLOGUE(a>0,rect.w++;);
		rect.x=a;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<40;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.x+=shutterW;
			}
		}
		dst.updateWholeScreen();
		rect.w=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectDshutter(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_LongRect rect(0,0,src_rect.w,1);
	long shutterH=src_rect.h/30;
	EFFECT_INITIALIZE_DELAYS(shutterH);
	for (long a=0;a<shutterH;a++){
		EFFECT_ITERATION_PROLOGUE(a<shutterH-1,rect.h++;);
		rect.y=a-rect.h+1;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<30;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.y+=shutterH;
			}
		}
		dst.updateWholeScreen();
		rect.h=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectUshutter(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_LongRect rect(0,0,src_rect.w,1);
	long shutterH=src_rect.h/30;
	EFFECT_INITIALIZE_DELAYS(shutterH);
	for (long a=shutterH-1;a>=0;a--){
		EFFECT_ITERATION_PROLOGUE(a>0,rect.h++;);
		rect.y=a;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<30;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.y+=shutterH;
			}
		}
		dst.updateWholeScreen();
		rect.h=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectRcurtain(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_LongRect rect(0,0,1,src_rect.h);
	long shutterW=(long)sqrt(double(src_rect.w));
	EFFECT_INITIALIZE_DELAYS(shutterW*2);
	for (long a=0;a<shutterW*2;a++){
		EFFECT_ITERATION_PROLOGUE(a<shutterW-1,rect.w++;);
		rect.x=Sint16(a-rect.w+1);
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<=a;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.x+=shutterW;
			}
		}
		dst.updateWholeScreen();
		rect.w=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectLcurtain(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	long shutterW=(long)sqrt(double(src_rect.w));
	NONS_LongRect rect(0,0,1,src_rect.h);
	EFFECT_INITIALIZE_DELAYS(shutterW*2);
	for (long a=0;a<shutterW*2;a++){
		EFFECT_ITERATION_PROLOGUE(a<shutterW-1,rect.w++;);
		rect.x=src_rect.w-a;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<=a;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.x-=shutterW;
			}
		}
		dst.updateWholeScreen();
		rect.w=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectDcurtain(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	long shutterH=(long)sqrt(double(src_rect.h));
	NONS_LongRect rect(0,0,src_rect.w,1);
	EFFECT_INITIALIZE_DELAYS(shutterH*2);
	for (long a=0;a<shutterH*2;a++){
		EFFECT_ITERATION_PROLOGUE(a<shutterH-1,rect.h++;);
		rect.y=a-rect.h+1;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<=a;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.y+=shutterH;
			}
		}
		dst.updateWholeScreen();
		rect.h=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectUcurtain(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	long shutterH=(long)sqrt(double(src_rect.h));
	NONS_LongRect rect(0,0,src_rect.w,1);
	EFFECT_INITIALIZE_DELAYS(shutterH*2);
	for (long a=0;a<shutterH*2;a++){
		EFFECT_ITERATION_PROLOGUE(a<shutterH-1,rect.h++;);
		rect.y=src_rect.h-a;
		{
			NONS_Surface screen=dst.get_screen();
			for (long b=0;b<=a;b++){
				screen.copy_pixels(src,&rect,&rect);
				rect.y-=shutterH;
			}
		}
		dst.updateWholeScreen();
		rect.h=1;
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectCrossfade(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	const long step=1;
	NONS_Surface dst_copy=dst.get_screen().clone();
	EFFECT_INITIALIZE_DELAYS(256/step);
#ifdef BENCHMARK_EFFECTS
	ulong steps=0;
#endif

	
	for (long a=step;a<256;a+=step){
		EFFECT_ITERATION_PROLOGUE(a<255,);
		{
			NONS_Surface screen=dst.get_screen();
			screen.copy_pixels(dst_copy);
			screen.over_with_alpha(src,0,0,a);
		}
		dst.updateWholeScreen();
#ifdef BENCHMARK_EFFECTS
		steps++;
#endif
		EFFECT_ITERATION_EPILOGUE;
	}
	
#ifdef BENCHMARK_EFFECTS
	double speed=steps*1000.0/this->duration;
	STD_COUT <<"effectCrossfade(): "<<speed<<" steps per second.\n";
#endif
	
	
	effect_epilogue();
}

void NONS_GFX::effectRscroll(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_Surface dst_copy=dst.get_screen().clone();
	EFFECT_INITIALIZE_DELAYS(src_rect.w);
	NONS_LongRect
		src_rect_A=src_rect,
		src_rect_B=src_rect,
		dst_rect_A=src_rect,
		dst_rect_B=src_rect;
	for (long a=src_rect.w-1;a>=0;a--){
		EFFECT_ITERATION_PROLOGUE(a>0,);
		src_rect_A.x=a;
		dst_rect_B.x=src_rect_A.w-src_rect_A.x;
		{
			NONS_Surface screen=dst.get_screen();
			screen.copy_pixels(src     ,&dst_rect_A,&src_rect_A);
			screen.copy_pixels(dst_copy,&dst_rect_B,&src_rect_B);
		}
		dst.updateWholeScreen();
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectLscroll(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_Surface dst_copy=dst.get_screen().clone();
	EFFECT_INITIALIZE_DELAYS(src_rect.w);
	NONS_LongRect
		src_rect_A=src_rect,
		src_rect_B=src_rect,
		dst_rect_A=src_rect,
		dst_rect_B=src_rect;
	for (ulong a=0;a<(ulong)src_rect.w;a++){
		EFFECT_ITERATION_PROLOGUE(a<(ulong)src_rect.w-1,);
		src_rect_A.x=a;
		dst_rect_B.x=src_rect_A.w-src_rect_A.x;
		{
			NONS_Surface screen=dst.get_screen();
			screen.copy_pixels(dst_copy,&dst_rect_A,&src_rect_A);
			screen.copy_pixels(src     ,&dst_rect_B,&src_rect_B);
		}
		dst.updateWholeScreen();
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectDscroll(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_Surface dst_copy=dst.get_screen().clone();
	EFFECT_INITIALIZE_DELAYS(src_rect.h);
	NONS_LongRect
		src_rect_A=src_rect,
		src_rect_B=src_rect,
		dst_rect_A=src_rect,
		dst_rect_B=src_rect;
	for (long a=src_rect.h-1;a>=0;a--){
		EFFECT_ITERATION_PROLOGUE(a>0,);
		src_rect_A.y=a;
		dst_rect_B.y=src_rect_A.h-src_rect_A.y;
		{
			NONS_Surface screen=dst.get_screen();
			screen.copy_pixels(src     ,&dst_rect_A,&src_rect_A);
			screen.copy_pixels(dst_copy,&dst_rect_B,&src_rect_B);
		}
		dst.updateWholeScreen();
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void NONS_GFX::effectUscroll(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	NONS_Surface dst_copy=dst.get_screen().clone();
	EFFECT_INITIALIZE_DELAYS(src_rect.h);
	NONS_LongRect
		src_rect_A=src_rect,
		src_rect_B=src_rect,
		dst_rect_A=src_rect,
		dst_rect_B=src_rect;
	for (ulong a=0;a<(ulong)src_rect.h;a++){
		EFFECT_ITERATION_PROLOGUE(a<(ulong)src_rect.w-1,);
		src_rect_A.y=a;
		dst_rect_B.y=src_rect_A.h-src_rect_A.y;
		{
			NONS_Surface screen=dst.get_screen();
			screen.copy_pixels(dst_copy,&dst_rect_A,&src_rect_A);
			screen.copy_pixels(src     ,&dst_rect_B,&src_rect_B);
		}
		dst.updateWholeScreen();
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void effectHardMask_threaded(NONS_ConstSurfaceProperties src0,NONS_SurfaceProperties src1,NONS_SurfaceProperties dst,long a){
	for (ulong y=0;y<src0.h;y++){
		for (ulong x=0;x<src0.w;x++){
			uchar *b1=src1.pixels+src1.offsets[2];
			if (*b1<=a){
				dst.pixels[dst.offsets[0]]=src0.pixels[src0.offsets[0]];
				dst.pixels[dst.offsets[1]]=src0.pixels[src0.offsets[1]];
				dst.pixels[dst.offsets[2]]=src0.pixels[src0.offsets[2]];
				dst.pixels[dst.offsets[3]]=src0.pixels[src0.offsets[3]];
				*b1=0xFF;
			}
			src0.pixels+=4;
			src1.pixels+=4;
			dst.pixels+=4;
		}
	}
}

NONS_Surface copy_mask(NONS_LongRect &src_rect,const NONS_ConstSurface &mask,NONS_VirtualScreen &screen){
	NONS_Surface r=screen.get_screen().clone_without_pixel_copy();
	NONS_LongRect mask_rect[2];
	mask_rect[0]=mask.clip_rect();
	mask_rect[1]=mask_rect[0];
	//copy the rule as a tile
	for (mask_rect[1].y=0;mask_rect[1].y<src_rect.h;mask_rect[1].y+=mask_rect[0].h)
		for (mask_rect[1].x=0;mask_rect[1].x<src_rect.w;mask_rect[1].x+=mask_rect[0].w)
			r.copy_pixels(mask,mask_rect+1,mask_rect);
	return r;
}

void NONS_GFX::effectHardMask(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst){
	NONS_Surface mask_copy;
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src0,dst))
		return;
	mask_copy=copy_mask(src_rect,src1,dst);

	NONS_SurfaceProperties screen_sp;
#ifndef USE_THREAD_MANAGER
	std::vector<NONS_Thread> threads(cpu_count);
#endif
	BINDER_TYPEDEF_4(function_parameters,NONS_ConstSurfaceProperties,NONS_SurfaceProperties,NONS_SurfaceProperties,long);
	std::vector<function_parameters> parameters(cpu_count);
	//prepare for threading
	ulong division=ulong(float(src_rect.h)/float(cpu_count));
	src0.get_properties(parameters[0].pt0);
	mask_copy.get_properties(parameters[0].pt1);
	parameters.front().f=effectHardMask_threaded;
	parameters.front().pt0.h=division;
	parameters.front().p=4;
	parameters.front().free_after_first_use=0;
	for (ulong a=0;a<cpu_count;a++){
		function_parameters &p=parameters[a];
		if (a>0)
			p=parameters.front();
		p.pt0.pixels+=p.pt0.pitch*a*division;
		p.pt1.pixels+=p.pt1.pitch*a*division;
	}
	parameters.back().pt0.h=src_rect.h-division*(cpu_count-1);

	EFFECT_INITIALIZE_DELAYS(256);
	for (long a=0;a<256;a++){
		EFFECT_ITERATION_PROLOGUE(a<255,);
		{
			NONS_Surface screen=dst.get_screen();
			screen.get_properties(screen_sp);
			for (ulong b=0;b<cpu_count;b++){
				function_parameters &p=parameters[b];
				p.pt3=a;
				p.pt2=screen_sp;
				p.pt2.pixels+=p.pt2.pitch*b*division;
			}
			for (ulong b=1;b<cpu_count;b++)
#ifndef USE_THREAD_MANAGER
				threads[b].call(&parameters[b]);
#else
				threadManager.call(b-1,&parameters[b]);
#endif
			parameters.front().call();
#ifndef USE_THREAD_MANAGER
			for (ulong b=1;b<cpu_count;b++)
				threads[b].join();
#else
			threadManager.waitAll();
#endif
		}
		dst.updateWholeScreen();
		EFFECT_ITERATION_EPILOGUE;
	}
	effect_epilogue();
}

void effectSoftMask_threaded(NONS_ConstSurfaceProperties src0,NONS_SurfaceProperties src1,NONS_SurfaceProperties dst,long a){
	for (ulong y=0;y<src0.h;y++){
		for (ulong x=0;x<src0.w;x++){
			uchar *b1=src1.pixels+2;
			if ((long)*b1<=a){
				long alpha=a-(long)*b1;
				if (alpha<0)
					alpha=0;
				else if (alpha>255)
					alpha=255;
				dst.pixels[dst.offsets[0]]=(uchar)APPLY_ALPHA(src0.pixels[src0.offsets[0]],dst.pixels[dst.offsets[0]],alpha);
				dst.pixels[dst.offsets[1]]=(uchar)APPLY_ALPHA(src0.pixels[src0.offsets[1]],dst.pixels[dst.offsets[1]],alpha);
				dst.pixels[dst.offsets[2]]=(uchar)APPLY_ALPHA(src0.pixels[src0.offsets[2]],dst.pixels[dst.offsets[2]],alpha);
				dst.pixels[dst.offsets[3]]=(uchar)APPLY_ALPHA(src0.pixels[src0.offsets[3]],dst.pixels[dst.offsets[3]],alpha);
				if ((long)*b1<a-255)
					*b1=0;
			}
			src0.pixels+=4;
			src1.pixels+=4;
			dst.pixels+=4;
		}
	}
}

void NONS_GFX::effectSoftMask(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src0,dst))
		return;
	NONS_Surface dst_copy=dst.get_screen().clone(),
		mask_copy=copy_mask(src_rect,src1,dst);
#ifndef USE_THREAD_MANAGER
	std::vector<NONS_Thread> threads(cpu_count);
#endif
	BINDER_TYPEDEF_4(function_parameters,NONS_ConstSurfaceProperties,NONS_SurfaceProperties,NONS_SurfaceProperties,long);
	std::vector<function_parameters> parameters(cpu_count);
	NONS_SurfaceProperties screen_sp;
	//prepare for threading
	ulong division=ulong(float(src_rect.h)/float(cpu_count));
	src0.get_properties(parameters[0].pt0);
	mask_copy.get_properties(parameters[0].pt1);
	parameters.front().pt0.h=division;
	parameters.front().f=effectSoftMask_threaded;
	parameters.front().free_after_first_use=0;
	for (ulong a=0;a<cpu_count;a++){
		function_parameters &p=parameters[a];
		if (a>0)
			p=parameters.front();
		p.pt0.pixels+=p.pt0.pitch*a*division;
		p.pt1.pixels+=p.pt1.pitch*a*division;
	}
	parameters.back().pt0.h=src_rect.h-division*(cpu_count-1);

	EFFECT_INITIALIZE_DELAYS(512);
#ifdef BENCHMARK_EFFECTS
	ulong steps=0;
#endif
	for (long a=0;a<512;a++){
		EFFECT_ITERATION_PROLOGUE(a<511,);
		{
			NONS_Surface screen=dst.get_screen();
			screen.get_properties(screen_sp);
			for (ulong b=0;b<cpu_count;b++){
				function_parameters &p=parameters[b];
				p.pt3=a;
				p.pt2=screen_sp;
				p.pt2.pixels+=p.pt2.pitch*b*division;
			}
			screen.copy_pixels(dst_copy);
			for (ulong b=1;b<cpu_count;b++)
#ifndef USE_THREAD_MANAGER
				threads[b].call(&parameters[b]);
#else
				threadManager.call(b-1,&parameters[b]);
#endif
			parameters.front().call();
#ifndef USE_THREAD_MANAGER
			for (ulong b=1;b<cpu_count;b++)
				threads[b].join();
#else
			threadManager.waitAll();
#endif
		}
		dst.updateWholeScreen();
#ifdef BENCHMARK_EFFECTS
		steps++;
#endif

		EFFECT_ITERATION_EPILOGUE;
	}
#ifdef BENCHMARK_EFFECTS
	double speed=steps*1000.0/this->duration;
	STD_COUT <<"effectSoftMask(): "<<speed<<" steps per second.\n";
#endif
	effect_epilogue();
}

void NONS_GFX::effectMosaic(const NONS_ConstSurface &src,const NONS_ConstSurface &,NONS_VirtualScreen &dst,long start_at,long end_at,long step){
	NONS_LongRect src_rect;
	if (!effect_standard_check(src_rect,src,dst))
		return;
	EFFECT_INITIALIZE_DELAYS(10);
	for (long a=start_at;a!=end_at;a+=step){
		EFFECT_ITERATION_PROLOGUE(lastT>5 && a!=end_at-step,);
		{
			NONS_Surface screen=dst.get_screen();
			if (a==0)
				screen.copy_pixels(src);
			else{
				NONS_ConstSurfaceProperties src_sp;
				NONS_SurfaceProperties dst_sp;
				screen.get_properties(dst_sp);
				src.get_properties(src_sp);
				ulong pixelSize=1<<a;
				for (ulong y=0;y<dst_sp.h;y+=pixelSize){
					for (ulong x=0;x<dst_sp.w;x+=pixelSize){
						NONS_LongRect rect(x,y,pixelSize,pixelSize);
						const uchar *src_pixel=src_sp.pixels;
						src_pixel+=src_sp.pitch*y+4*x;
						screen.fill(rect,NONS_Color(
							src_pixel[src_sp.offsets[0]],
							src_pixel[src_sp.offsets[1]],
							src_pixel[src_sp.offsets[2]],
							src_pixel[src_sp.offsets[3]]));
					}
				}
			}
		}
		dst.updateWholeScreen();
		EFFECT_ITERATION_EPILOGUE;
	}
}

void NONS_GFX::effectMosaicIn(const NONS_ConstSurface &src,const NONS_ConstSurface &nothing,NONS_VirtualScreen &dst){
	this->effectMosaic(src,nothing,dst,9,-1,-1);
	effect_epilogue();
}

void NONS_GFX::effectMosaicOut(const NONS_ConstSurface &src,const NONS_ConstSurface &nothing,NONS_VirtualScreen &dst){
	this->effectMosaic(src,nothing,dst,0,10,1);
	dst.get_screen().copy_pixels(src);
	effect_epilogue();
}

TiXmlElement *NONS_GFX::save(const char *override_name){
	TiXmlElement *effect=new TiXmlElement(override_name?override_name:"effect");
	effect->SetAttribute("effect_no",this->effect);
	effect->SetAttribute("type",(int)this->type);
	effect->LinkEndChild(this->color.save());
	effect->SetAttribute("duration",this->duration);
	if (this->rule.size())
		effect->SetAttribute("rule",this->rule);
	return effect;
}

NONS_GFX::NONS_GFX(TiXmlElement *parent,const char *name){
	TiXmlElement *effect=parent->FirstChildElement(name?name:"effect");
	this->effect=effect->QueryIntAttribute("effect_no");
	this->type=effect->QueryIntAttribute("type");
	this->color=NONS_Color(effect->FirstChildElement("color"));
	this->duration=effect->QueryIntAttribute("duration");
	this->rule=effect->QueryWStringAttribute("rule");
	this->stored=0;
}


NONS_GFXstore::NONS_GFXstore(){
	NONS_GFX *temp[]={
		new NONS_GFX(),
		new NONS_GFX(1)
	};
	temp[0]->stored=1;
	temp[1]->stored=1;
	this->effects[0]=temp[0];
	this->effects[1]=temp[1];
}

NONS_GFXstore::~NONS_GFXstore(){
	for (std::map<ulong,NONS_GFX *>::iterator i=this->effects.begin();i!=this->effects.end();i++)
		delete i->second;
}

NONS_GFX *NONS_GFXstore::retrieve(ulong code){
	std::map<ulong,NONS_GFX *>::iterator i=this->effects.find(code);
	if (i==this->effects.end())
		return 0;
	return i->second;
}

bool NONS_GFXstore::remove(ulong code){
	std::map<ulong,NONS_GFX *>::iterator i=this->effects.find(code);
	if (i==this->effects.end())
		return 0;
	delete i->second;
	this->effects.erase(i);
	return 1;
}

NONS_GFX *NONS_GFXstore::add(ulong code,ulong effect,ulong duration,const std::wstring *rule){
	if (this->retrieve(code))
		return 0;
	NONS_GFX *res=new NONS_GFX(effect,duration,rule);
	this->effects[code]=res;
	return res;
}
