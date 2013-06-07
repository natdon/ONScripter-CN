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

#include "VirtualScreen.h"
#include "Functions.h"
#include "IOFunctions.h"
#include "ThreadManager.h"
#include "Options.h"
#include "Plugin/LibraryLoader.h"
#include <iostream>
#include <cmath>

//#define ONLY_NEAREST_NEIGHBOR
//#define BENCHMARK_INTERPOLATION

pipelineElement::pipelineElement(ulong effectNo,const NONS_Color &color,const std::wstring &rule,bool loadRule)
		:effectNo(effectNo),color(color),ruleStr(rule){
	if (rule.size() && loadRule)
		this->rule=rule;
}

void pipelineElement::operator=(const pipelineElement &o){
	this->effectNo=o.effectNo;
	this->color=o.color;
	this->rule.unbind();
	this->ruleStr=o.ruleStr;
}

TiXmlElement *pipelineElement::save(const char *override_name){
	TiXmlElement *effect=new TiXmlElement(override_name?override_name:"effect");
	effect->SetAttribute("effect_no",this->effectNo);
	effect->LinkEndChild(this->color.save());
	effect->SetAttribute("rule",(this->rule)?this->ruleStr:L"");
	return effect;
}

pipelineElement::pipelineElement(TiXmlElement *effect)
		:color(effect->FirstChildElement("color")){
	this->effectNo=effect->QueryIntAttribute("effect_no");
	this->ruleStr=effect->QueryWStringAttribute("rule");
	if (this->ruleStr.size())
		this->rule=this->ruleStr;
}

void asyncEffectThread(NONS_VirtualScreen *vs){
	ulong effectNo=vs->aeffect_no;
	ulong freq=vs->aeffect_freq;
	ulong ms=1000/freq;
	void *userData=0;
	if (vs->initializers[effectNo])
		userData=vs->initializers[effectNo](effectNo);
	asyncEffect_f effect=vs->effects[effectNo];
	NONS_Clock clock;
	while (!vs->killAsyncEffect){
		NONS_Clock::t t0=clock.get();
		{
			NONS_Surface dst=vs->get_real_screen();
			if (effect(effectNo+1,vs->get_preasync_surface(),dst,userData))
				dst.update();
		}
		NONS_Clock::t t1=clock.get();
		t1-=t0;
		if (t1<ms)
			SDL_Delay(Uint32(ms-t1));
	}
	if (vs->uninitializers[effectNo])
		vs->uninitializers[effectNo](effectNo,userData);
}

SDL_Surface *allocate_screen(ulong w,ulong h,bool fullscreen){
	SDL_Surface *r=SDL_SetVideoMode(w,h,32,SDL_SWSURFACE);
	if (!r){
		STD_CERR <<"FATAL ERROR: Could not allocate screen!\n"
			"Did you forget to install libx11-dev before building?\n"
			"Terminating.\n";
		exit(0);
	}
	return r;
}

void NONS_VirtualScreen::init_fullscreen(){
	this->fullscreen=(CLOptions.startFullscreen || settings.fullscreen.set && settings.fullscreen.data);
}

NONS_VirtualScreen::NONS_VirtualScreen(ulong w,ulong h){
	std::fill(this->usingFeature,this->usingFeature+usingFeature_s,(bool)0);
	this->init_fullscreen();
	this->screens[VIRTUAL]=
		this->screens[REAL]=NONS_Surface::assign_screen(allocate_screen(w,h,this->fullscreen));
	this->x_multiplier=0x100;
	this->y_multiplier=0x100;
	this->x_divisor=this->x_multiplier;
	this->y_divisor=this->y_multiplier;
	this->outRect=
		this->inRect=this->screens[REAL].get_dimensions<float>();
	this->normalInterpolation=0;
	this->killAsyncEffect=0;
	this->initEffectList();
	memset(this->usingFeature,0,this->usingFeature_s*sizeof(bool));
	this->printCurrentState();
}

NONS_VirtualScreen::NONS_VirtualScreen(ulong iw,ulong ih,ulong ow,ulong oh){
	std::fill(this->usingFeature,this->usingFeature+usingFeature_s,(bool)0);
	this->init_fullscreen();
	this->screens[VIRTUAL]=
		this->screens[REAL]=NONS_Surface::assign_screen(allocate_screen(ow,oh,this->fullscreen));
	if (iw==ow && ih==oh){
		this->x_multiplier=1.f;
		this->y_multiplier=1.f;
		this->outRect=
			this->inRect=this->screens[REAL].get_dimensions<float>();
		this->normalInterpolation=0;
	}else{
		this->usingFeature[INTERPOLATION]=1;
		this->pre_inter=VIRTUAL;
		this->post_inter=REAL;
		{
			NONS_Surface surface(iw,ih);
			this->screens[VIRTUAL]=surface;
			this->screens[VIRTUAL].strong_bind();
		}
		float ratio0=float(iw)/float(ih),
			ratio1=float(ow)/float(oh);
		this->normalInterpolation=&NONS_Surface::bilinear_interpolation;
		this->inRect=this->screens[VIRTUAL].get_dimensions<float>();
		if (fabs(ratio0-ratio1)<.00001f){
			this->x_multiplier=float(ow)/float(iw);
			this->y_multiplier=float(oh)/float(ih);
			if (x_multiplier<1.f || y_multiplier<1.f)
				this->normalInterpolation=&NONS_Surface::bilinear_interpolation2;
			this->outRect=this->screens[REAL].get_dimensions<float>();
		}else if (ratio0>ratio1){
			float h=float(ow)/float(ratio0);
			this->outRect.x=0;
			this->outRect.y=(oh-h)/2.f;
			this->outRect.w=(float)ow;
			this->outRect.h=(float)h;
			this->x_multiplier=float(ow)/float(iw);
			this->y_multiplier=h/float(ih);
			if (this->x_multiplier<1.f || h/float(ih)<1.f)
				this->normalInterpolation=&NONS_Surface::bilinear_interpolation2;
		}else{
			float w=float(oh)*float(ratio0);
			this->outRect.x=(ow-w)/2.0f;
			this->outRect.y=0;
			this->outRect.w=w;
			this->outRect.h=(float)oh;
			this->x_multiplier=w/float(iw);
			this->y_multiplier=float(oh)/float(ih);
			if (this->y_multiplier<1.f || w/float(iw)<1.f)
				this->normalInterpolation=&NONS_Surface::bilinear_interpolation2;
		}
		this->x_divisor=1.f/this->x_multiplier;
		this->y_divisor=1.f/this->y_multiplier;
	}
	this->killAsyncEffect=0;
	this->initEffectList();
	this->printCurrentState();
}

NONS_VirtualScreen::~NONS_VirtualScreen(){
	this->stopEffect();
	this->applyFilter(0,NONS_Color::black,L"");
	settings.fullscreen.data=this->fullscreen;
	settings.fullscreen.set=1;
}

void NONS_VirtualScreen::blitToScreen(NONS_Surface &src,NONS_LongRect *srcrect,NONS_LongRect *dstrect){
	this->get_screen().over(src,dstrect,srcrect);
}

NONS_DECLSPEC void NONS_VirtualScreen::updateScreen(ulong x,ulong y,ulong w,ulong h,bool fast){
	if (this->usingFeature[OVERALL_FILTER]){
		NONS_Surface src=this->screens[VIRTUAL],
			dst=this->screens[this->post_filter];
		for (ulong a=0;a<this->filterPipeline.size();a++){
			if (a==1)
				src=this->screens[this->post_filter];
			pipelineElement &el=this->filterPipeline[a];
			this->filters[el.effectNo](el.effectNo+1,el.color,src,el.rule,dst,NONS_LongRect(x,y,w,h));
		}
	}
	if (this->usingFeature[INTERPOLATION]){
		NONS_Rect d(
				floor(this->convertX(x)+.5f),
				floor(this->convertY(y)+.5f),
				floor(this->convertW(w)+.5f),
				floor(this->convertH(h)+.5f)
			),
			s(
				this->unconvertX(d.x),
				this->unconvertY(d.y),
				this->unconvertW(d.w),
				this->unconvertH(d.h)
			);
		if (d.x+d.w>this->outRect.w+this->outRect.x)
			d.w=(long)this->outRect.w-d.x+(long)this->outRect.x;
		if (d.y+d.h>this->outRect.h+this->outRect.y)
			d.h=(long)this->outRect.h-d.y+(long)this->outRect.y;
#ifdef ONLY_NEAREST_NEIGHBOR
		fast=1;
#endif
		NONS_Surface::public_interpolation_f f=(fast)?&NONS_Surface::NN_interpolation:this->normalInterpolation;
		{
			NONS_Surface dst=this->screens[this->post_inter];
			(dst.*f)(
				this->screens[this->pre_inter],
				d,
				s,
				this->x_multiplier,
				this->y_multiplier
			);
		}
		if (!this->usingFeature[ASYNC_EFFECT])
			NONS_Surface(this->screens[REAL]).update((ulong)d.x,(ulong)d.y,(ulong)d.w,(ulong)d.h);
	}else if (!this->usingFeature[ASYNC_EFFECT])
		NONS_Surface(this->screens[REAL]).update(x,y,w,h);
}

bool NONS_VirtualScreen::toggleFullscreen(uchar mode){
	NONS_Surface screen=this->screens[REAL];
	if (mode==2)
		this->fullscreen=!this->fullscreen;
	else
		this->fullscreen=mode&1;
	NONS_SurfaceProperties sp;
	screen.get_properties(sp);
	NONS_Surface tempCopy;
	if (!this->usingFeature[INTERPOLATION] && !this->usingFeature[ASYNC_EFFECT])
		tempCopy=screen.clone();
	this->screens[REAL]=
		screen=NONS_Surface::assign_screen(allocate_screen(sp.w,sp.h,this->fullscreen));
	if (tempCopy){
		if (!this->usingFeature[OVERALL_FILTER])
			this->screens[VIRTUAL]=this->screens[REAL];
		screen.copy_pixels(tempCopy);
	}
	this->updateWholeScreen();
	return this->fullscreen;
}

SDL_Surface *NONS_VirtualScreen::toggleFullscreenFromVideo(const NONS_Surface &s){
	this->fullscreen=!this->fullscreen;
	ulong w,h;
	this->screens[REAL].get_dimensions(w,h);
	bool tempCopy=0;
	if (!this->usingFeature[INTERPOLATION] && !this->usingFeature[ASYNC_EFFECT])
		tempCopy=1;
	NONS_Surface::assign_screen(allocate_screen(w,h,this->fullscreen),0);
	const NONS_Surface &screen=s;
	this->screens[REAL]=NONS_CrippledSurface(screen);
	if (tempCopy){
		if (!this->usingFeature[OVERALL_FILTER])
			this->screens[VIRTUAL]=this->screens[REAL];
	}
	return screen.get_SDL_screen();
}

std::wstring NONS_VirtualScreen::takeScreenshot(const std::wstring &name){
	std::wstring filename;
	if (!name.size())
		filename=generate_filename<wchar_t>();
	else
		filename=name;
	NONS_Surface(this->screens[REAL]).save_bitmap(filename);
	return filename;
}

void NONS_VirtualScreen::initEffectList(){
	this->initializers.clear();
	this->effects.clear();
	this->uninitializers.clear();
	this->filters.clear();
	this->filters.push_back(effectMonochrome);
	this->filters.push_back(effectNegative);
	return;
	/*pluginLibraryFunction fp=(pluginLibraryFunction)pluginLibrary.getFunction("getFunctionPointers");
	if (!fp)
		return;
	{
		//1=Get filter effect function pointers.
		std::vector<filterFX_f> vec=*(std::vector<filterFX_f> *)fp((void *)1);
		this->filters.insert(this->filters.end(),vec.begin(),vec.end());
	}
	{
		//2=Get asynchronous effect function pointers.
		std::vector<asyncFXfunctionSet> vec=*(std::vector<asyncFXfunctionSet> *)fp((void *)2);
		for (ulong a=0;a<vec.size();a++){
			this->initializers.push_back(vec[a].initializer);
			this->effects.push_back(vec[a].effect);
			this->uninitializers.push_back(vec[a].uninitializer);
		}
	}*/
}

ErrorCode NONS_VirtualScreen::callEffect(ulong effectNo,ulong frequency){
	if (effectNo>=this->effects.size())
		return NONS_NO_EFFECT;
	this->stopEffect();
	{
		NONS_MutexLocker ml(this->mutex);
		this->changeState(1,0);
	}
	this->aeffect_no=effectNo;
	this->aeffect_freq=frequency;
	this->asyncEffectThread.call(bind(::asyncEffectThread,this));
	return NONS_NO_ERROR;
}

void NONS_VirtualScreen::stopEffect(){
	this->killAsyncEffect=1;
	this->asyncEffectThread.join();
	this->killAsyncEffect=0;
	NONS_MutexLocker ml(this->mutex);
	if (this->usingFeature[ASYNC_EFFECT])
		this->changeState(1,0);
}

ErrorCode NONS_VirtualScreen::applyFilter(ulong effectNo,const NONS_Color &color,const std::wstring &rule){
	if (!effectNo){
		if (this->usingFeature[OVERALL_FILTER]){
			this->changeState(0,1);
			this->filterPipeline.clear();
		}
		return NONS_NO_ERROR;
	}
	effectNo--;
	if (this->filters.size()<=effectNo)
		return NONS_NO_EFFECT;
	if (!this->usingFeature[OVERALL_FILTER])
		this->changeState(0,1);
	this->filterPipeline.push_back(pipelineElement(effectNo,color,rule,1));
	return NONS_NO_ERROR;
}

void NONS_VirtualScreen::changeState(bool switchAsyncState,bool switchFilterState){
	ulong originalState=(ulong)this->usingFeature[OVERALL_FILTER]|((ulong)this->usingFeature[ASYNC_EFFECT]<<1),
		newState=(ulong)(this->usingFeature[OVERALL_FILTER]^switchFilterState)|((ulong)(this->usingFeature[ASYNC_EFFECT]^switchAsyncState)<<1),
		transition=(originalState<<2)|(newState);
	bool interpolation=this->usingFeature[INTERPOLATION];
	switch (transition){
		case 0:  // ignore changing from one state to itself
		case 5:
		case 10:
		case 15:
		case 3:	 // ignore changing more than one state at a time
		case 6:
		case 9:
		case 12:
			break;
		case 1: //turn filter on, effect remains off
			if (interpolation){
				this->post_filter=1;
				this->pre_inter=1;
				this->screens[this->pre_inter]=this->screens[VIRTUAL];
			}else
				this->post_filter=3;
			NONS_CrippledSurface::copy_surface(this->screens[VIRTUAL],NONS_Surface(this->screens[VIRTUAL]));
			break;
		case 2: //turn effect on, filter remains off
			NONS_CrippledSurface::copy_surface(this->screens[PRE_ASYNC],NONS_Surface(this->screens[REAL]));
			if (interpolation)
				this->post_inter=2;
			else
				this->screens[VIRTUAL]=this->screens[PRE_ASYNC];
			break;
		case 4: //turn filter off, effect remains off
			if (interpolation)
				this->screens[this->pre_inter].unbind();
			else
				this->screens[VIRTUAL]=this->screens[REAL];
			break;
		case 7: //turn effect on, filter remains on
			if (interpolation)
				this->post_inter=2;
			else
				this->post_filter=2;
			NONS_CrippledSurface::copy_surface(this->screens[PRE_ASYNC],NONS_Surface(this->screens[REAL]));
			break;
		case 8: //turn effect off, filter remains off
			this->screens[PRE_ASYNC].unbind();
			if (interpolation)
				this->post_inter=3;
			else
				this->screens[VIRTUAL]=this->screens[REAL];
			break;
		case 11: //turn filter on, effect remains on
			if (interpolation){
				this->post_filter=1;
				this->pre_inter=1;
				NONS_CrippledSurface::copy_surface(this->screens[this->pre_inter],NONS_Surface(this->screens[VIRTUAL]));
			}else{
				this->post_filter=2;
				NONS_CrippledSurface::copy_surface(this->screens[VIRTUAL],NONS_Surface(this->screens[VIRTUAL]));
			}
			break;
		case 13: //turn effect off, filter remains on
			this->screens[PRE_ASYNC].unbind();
			if (interpolation)
				this->post_inter=3;
			else
				this->post_filter=3;
			break;
		case 14: //turn filter off, effect remains on
			if (interpolation){
				this->screens[this->pre_inter].unbind();
				this->pre_inter=0;
			}else{
				this->screens[VIRTUAL].unbind();
				this->screens[VIRTUAL]=this->screens[PRE_ASYNC];
			}
			break;
	}
	this->usingFeature[OVERALL_FILTER]^=switchFilterState;
	this->usingFeature[ASYNC_EFFECT]^=switchAsyncState;
	this->printCurrentState();
}

void NONS_VirtualScreen::printCurrentState(){
	STD_COUT <<"Processing pipeline: ";
	for (ulong a=0;a<3;a++){
		STD_COUT <<a;
		ulong pointsTo=a;
		for (ulong b=a+1;b<4 && pointsTo==a;b++)
			if (this->screens[a]==this->screens[b])
				pointsTo=b;
		if (pointsTo!=a){
			STD_COUT <<"->";
			a=pointsTo-1;
			continue;
		}
		STD_COUT <<"*-(";
		if (a<1 && this->usingFeature[OVERALL_FILTER]){
			a=this->post_filter-1;
			STD_COUT <<"filter";
		}else if (a<2 && this->usingFeature[INTERPOLATION]){
			a=this->post_inter-1;
			STD_COUT <<"interpolation";
		}else if (a<3 && this->usingFeature[ASYNC_EFFECT]){
			a=REAL-1;
			STD_COUT <<"async effect";
		}
		STD_COUT <<")->";
	}
	STD_COUT <<"3\n";
}

TiXmlElement *NONS_VirtualScreen::save_async_fx(const char *override_name){
	TiXmlElement *async_fx=new TiXmlElement("async_fx");
	if (this->usingFeature[ASYNC_EFFECT]){
		async_fx->SetAttribute("effect_no",this->aeffect_no);
		async_fx->SetAttribute("frequency",this->aeffect_freq);
	}
	return async_fx;
}

void NONS_VirtualScreen::load_async_fx(TiXmlElement *parent,const char *name){
	TiXmlElement *async_fx=parent->FirstChildElement(name?name:"async_fx");
	this->callEffect(async_fx->QueryIntAttribute("effect_no"),async_fx->QueryIntAttribute("frequency"));
}

TiXmlElement *NONS_VirtualScreen::save_filter_pipeline(const char *override_name){
	TiXmlElement *filter_pipeline=new TiXmlElement(override_name?override_name:"filter_pipeline");
	for (size_t a=0;a<this->filterPipeline.size();a++)
		filter_pipeline->LinkEndChild(this->filterPipeline[a].save());
	return filter_pipeline;
}

void NONS_VirtualScreen::load_filter_pipeline(TiXmlElement *parent,const char *name){
	TiXmlElement *filter_pipeline=parent->FirstChildElement(name?name:"filter_pipeline");
	this->applyFilter(0,NONS_Color::black_transparent,L"");
	for (TiXmlElement *i=filter_pipeline->FirstChildElement();i;i=i->NextSiblingElement()){
		pipelineElement pe(i);
		this->applyFilter(pe.effectNo+1,pe.color,pe.ruleStr);
	}
}

#define RED_MONOCHROME(x) ((x)*3/10)
#define GREEN_MONOCHROME(x) ((x)*59/100)
#define BLUE_MONOCHROME(x) ((x)*11/100)

void effectMonochrome_threaded(const NONS_Surface *dst,NONS_ConstSurface src,NONS_LongRect rect,NONS_Color color){
	NONS_ConstSurfaceProperties src_p;
	NONS_SurfaceProperties dst_p;
	src.get_properties(src_p);
	dst->get_properties(dst_p);
	long w=(long)rect.w,
		h=(long)rect.h;
	src_p.pixels+=long(rect.x)*4+long(rect.y)*src_p.pitch;
	dst_p.pixels+=long(rect.x)*4+long(rect.y)*dst_p.pitch;
	for (long y=0;y<h;y++){
		const uchar *pix0=src_p.pixels;
		uchar *pix1=dst_p.pixels;
		for (long x0=0;x0<w;x0++){
			ulong c0[]={
				RED_MONOCHROME(ulong(pix0[src_p.offsets[0]])),
				GREEN_MONOCHROME(ulong(pix0[src_p.offsets[1]])),
				BLUE_MONOCHROME(ulong(pix0[src_p.offsets[2]]))
			};
			for (int a=0;a<3;a++){
				ulong b=c0[0]*color.rgba[a]
				       +c0[1]*color.rgba[a]
				       +c0[2]*color.rgba[a];
				b/=255;
				pix1[dst_p.offsets[a]]=(uchar)b;
			}
			pix0+=4;
			pix1+=4;
		}
		src_p.pixels+=src_p.pitch;
		dst_p.pixels+=dst_p.pitch;
	}
}

FILTER_EFFECT_F(effectMonochrome){
	if (!dst)
		return;
	NONS_LongRect &dstRect=area;
	if (cpu_count==1){
		effectMonochrome_threaded(&dst,src,dstRect,color);
		return;
	}
#ifndef USE_THREAD_MANAGER
	std::vector<NONS_Thread> threads(cpu_count);
#endif
	BINDER_TYPEDEF_4(monochrome_parameters,const NONS_Surface *,NONS_ConstSurface,NONS_LongRect,NONS_Color);
	std::vector<monochrome_parameters> parameters(cpu_count);
	ulong division=ulong(float(dstRect.h)/float(cpu_count)),
		total=0;
	parameters.front().f=effectMonochrome_threaded;
	parameters.front().free_after_first_use=0;
	parameters.front().p=4;
	for (ushort a=0;a<cpu_count;a++){
		monochrome_parameters &p=parameters[a];
		p=parameters.front();
		p.pt0=&dst;
		p.pt1=src;
		p.pt2=dstRect;
		p.pt2.y+=a*division;
		p.pt2.h=division;
		total+=division;
		p.pt3=color;
	}
	parameters.back().pt2.h+=dstRect.h-total;
	for (ushort a=1;a<cpu_count;a++)
#ifndef USE_THREAD_MANAGER
		threads[a].call(&parameters[a]);
#else
		threadManager.call(a-1,&parameters[a]);
#endif
	parameters.front().call();
#ifndef USE_THREAD_MANAGER
	for (ushort a=1;a<cpu_count;a++)
		threads[a].join();
#else
	threadManager.waitAll();
#endif
}
                            
void effectNegative_threaded(const NONS_Surface *dst,NONS_ConstSurface src,NONS_LongRect rect){
	NONS_ConstSurfaceProperties src_p;
	NONS_SurfaceProperties dst_p;
	src.get_properties(src_p);
	dst->get_properties(dst_p);
	long w=(long)rect.w,
		h=(long)rect.h;
	src_p.pixels+=long(rect.x)*4+long(rect.y)*src_p.pitch;
	dst_p.pixels+=long(rect.x)*4+long(rect.y)*dst_p.pitch;
	for (long y=0;y<h;y++){
		const uchar *pix0=src_p.pixels;
		uchar *pix1=dst_p.pixels;
		for (long x0=0;x0<w;x0++){
			pix1[dst_p.offsets[0]]=~pix0[src_p.offsets[0]];
			pix1[dst_p.offsets[1]]=~pix0[src_p.offsets[1]];
			pix1[dst_p.offsets[2]]=~pix0[src_p.offsets[2]];
			pix1[dst_p.offsets[3]]=pix0[src_p.offsets[3]];
			pix1+=4;
			pix0+=4;
		}
		src_p.pixels+=src_p.pitch;
		dst_p.pixels+=dst_p.pitch;
	}
}

FILTER_EFFECT_F(effectNegative){
	if (!dst)
		return;
	NONS_LongRect &dstRect=area;
	if (cpu_count==1){
		effectMonochrome_threaded(&dst,src,dstRect,color);
		return;
	}
#ifndef USE_THREAD_MANAGER
	std::vector<NONS_Thread> threads(cpu_count);
#endif
	BINDER_TYPEDEF_3(negative_parameters,const NONS_Surface *,NONS_ConstSurface,NONS_LongRect);
	std::vector<negative_parameters> parameters(cpu_count);
	ulong division=ulong(float(dstRect.h)/float(cpu_count)),
		total=0;
	parameters.front().f=effectNegative_threaded;
	parameters.front().free_after_first_use=0;
	parameters.front().p=3;
	for (ushort a=0;a<cpu_count;a++){
		negative_parameters &p=parameters[a];
		p=parameters.front();
		p.pt0=&dst;
		p.pt1=src;
		p.pt2=dstRect;
		p.pt2.y+=a*division;
		p.pt2.h=division;
		total+=division;
	}
	parameters.back().pt2.h+=dstRect.h-total;
	for (ushort a=1;a<cpu_count;a++)
#ifndef USE_THREAD_MANAGER
		threads[a].call(&parameters[a]);
#else
		threadManager.call(a-1,&parameters[a]);
#endif
	parameters.front().call();
#ifndef USE_THREAD_MANAGER
	for (ushort a=1;a<cpu_count;a++)
		threads[a].join();
#else
	threadManager.waitAll();
#endif
}
