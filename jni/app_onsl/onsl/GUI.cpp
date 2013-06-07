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

#include "GUI.h"
#include "ScreenSpace.h"
#include "ScriptInterpreter.h"
#include "IOFunctions.h"
#include "Options.h"
#include <iostream>
#include <cassert>
#include <freetype/ftoutln.h>
#include <freetype/ftstroke.h>

NONS_DebuggingConsole *console=0;

template <typename T>
inline T FT_FLOOR(T x){
	return (x&-64)/64;
}

template <typename T>
inline T FT_CEIL(T x){
	return ((x+63)&-64)/64;
}

NONS_Lookback::NONS_Lookback(NONS_StandardOutput *output,const NONS_Color &color)
		:foreground(color){
	this->output=output;
	this->up=0;
	this->down=0;
	this->resetButtons();
}

void NONS_Lookback::resetButtons(){
	NONS_LongRect temp=this->output->foregroundLayer->clip_rect;
	temp.y=temp.h;
	temp.h/=3;
	temp.y-=temp.h;
	NONS_GraphicButton *up=new NONS_GraphicButton(NONS_Surface::null,0,0,(int)temp.w,(int)temp.h,0,0),
		*down=new NONS_GraphicButton(NONS_Surface::null,0,(int)temp.y,(int)temp.w,(int)temp.h,0,0);
	delete this->up;
	delete this->down;
	this->up=up;
	this->down=down;
}

NONS_Lookback::~NONS_Lookback(){
	delete this->up;
	delete this->down;
}

bool NONS_Lookback::setUpButtons(const std::wstring &upon,const std::wstring &upoff,const std::wstring &downon,const std::wstring &downoff){
	const std::wstring *srcs[4]={&upon,&upoff,&downon,&downoff};
	for (int a=0;a<4;a++){
		this->surfaces[a]=*srcs[a];
		if (!this->surfaces[a]){
			for (;a>=0;a--)
				this->surfaces[a].unbind();
			return 0;
		}
	}
	this->setUpButtons();
	return 1;
}

bool NONS_Lookback::setUpButtons(ulong up,ulong down,NONS_ScreenSpace *screen){
	delete this->up;
	delete this->down;
	this->up=new NONS_SpriteButton(up,screen);
	this->down=new NONS_SpriteButton(down,screen);
	return 1;
}

void NONS_Lookback::setUpButtons(){
	NONS_GraphicButton *up=dynamic_cast<NONS_GraphicButton *>(this->up),
		*down=dynamic_cast<NONS_GraphicButton *>(this->down);
	assert(up!=0);
	up->allocateLayer(up->setOffLayer(),NONS_Surface::null,0,0,(int)up->getBox().w,(int)up->getBox().h,0,0);
	down->allocateLayer(down->setOffLayer(),NONS_Surface::null,0,0,(int)down->getBox().w,(int)down->getBox().h,0,0);
	NONS_LongRect srcRect=this->surfaces[0].clip_rect(),
		dstRect[4];
	NONS_Layer *dst[]={
		up->setOnLayer(),
		up->setOffLayer(),
		down->setOnLayer(),
		down->setOffLayer()
	};
	for (int a=0;a<4;a++){
		NONS_LongRect temp=this->surfaces[a].clip_rect();
		dstRect[a].x=dst[a]->clip_rect.w-temp.w;
		if (a<2)
			continue;
		dstRect[a].y=dst[a]->clip_rect.h-temp.h;
	}
	for (int a=0;a<4;a++)
		dst[a]->data.over(this->surfaces[a],dstRect+a,&srcRect);
}

void NONS_Lookback::reset(NONS_StandardOutput *output){
	this->output=output;
	if (this->up && !dynamic_cast<NONS_SurfaceButton *>(this->up)){
		this->resetButtons();
		if (!this->surfaces[0])
			return;
		this->setUpButtons();
	}
}

int NONS_Lookback::display(NONS_VirtualScreen *dst){
	int ret=0;
	if (!this->output->log.size())
		return ret;
	NONS_EventQueue queue;
	NONS_Surface dst_copy=dst->get_screen().clone();
	long end=this->output->log.size(),
		currentPage=end-1;
	this->output->ephemeralOut(this->output->log[currentPage],dst,0,0,&this->foreground);
	NONS_Surface before_blit=dst->get_screen().clone();
	int mouseOver=-1;
	int x,y;
	uchar visibility=(!!currentPage)<<1;
	if (visibility){
		getCorrectedMousePosition(dst,&x,&y);
		if (this->up->MouseOver(x,y)){
			mouseOver=0;
			this->up->mergeWithoutUpdate(dst,before_blit,1,1);
		}else{
			mouseOver=-1;
			this->up->mergeWithoutUpdate(dst,before_blit,0,1);
		}
	}else
		mouseOver=-1;
	dst->updateWholeScreen();
	while (1){
		queue.WaitForEvent(10);
		while (!queue.empty()){
			SDL_Event event=queue.pop();
			switch (event.type){
				case SDL_QUIT:
					ret=INT_MIN;
					goto callLookback_000;
				case SDL_MOUSEMOTION:
					{
						if (visibility){
							if (visibility&2 && this->up->MouseOver(&event)){
								if (visibility&1)
									this->down->merge(dst,before_blit,0);
								mouseOver=0;
								this->up->merge(dst,before_blit,1);
							}else{
								if (visibility&2)
									this->up->merge(dst,before_blit,0);
								if (visibility&1 && this->down->MouseOver(&event)){
									mouseOver=1;
									this->down->merge(dst,before_blit,1);
								}else{
									mouseOver=-1;
									if (visibility&1)
										this->down->merge(dst,before_blit,0);
								}
							}
						}
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					{
						if (event.button.button==SDL_BUTTON_LEFT){
							if (mouseOver<0 || !visibility)
								break;
							dst->get_screen().copy_pixels(dst_copy);
							int dir;
							if (!mouseOver)
								dir=-1;
							else
								dir=1;
							if (!this->changePage(dir,currentPage,dst_copy,dst,before_blit,visibility,mouseOver))
								goto callLookback_000;
						}else if (event.button.button==SDL_BUTTON_WHEELUP || event.button.button==SDL_BUTTON_WHEELDOWN){
							int dir;
							if (event.button.button==SDL_BUTTON_WHEELUP)
								dir=-1;
							else
								dir=1;
							if (!this->changePage(dir,currentPage,dst_copy,dst,before_blit,visibility,mouseOver))
								goto callLookback_000;
						}
					}
					break;
				case SDL_KEYDOWN:
					{
						int dir=0;
						switch (event.key.keysym.sym){
							case SDLK_UP:
							case SDLK_PAGEUP:
								dir=-1;
								break;
							case SDLK_DOWN:
							case SDLK_PAGEDOWN:
								dir=1;
								break;
							case SDLK_ESCAPE:
								dir=end-currentPage;
							default:
								break;
						}
						if (!this->changePage(dir,currentPage,dst_copy,dst,before_blit,visibility,mouseOver))
							goto callLookback_000;
					}
					break;
			}
		}
		SDL_Delay(10);
	}
callLookback_000:
	return ret;
}

bool NONS_Lookback::changePage(
		int dir,
		long &currentPage,
		const NONS_ConstSurface &copyDst,
		NONS_VirtualScreen *dst,
		NONS_Surface &preBlit,
		uchar &visibility,
		int &mouseOver){
	long end=this->output->log.size();
	if (!dir || -dir>currentPage)
		return 1;
	dst->get_screen().over(copyDst);
	currentPage+=dir;
	if (currentPage==end)
		return 0;
	this->output->ephemeralOut(this->output->log[currentPage],dst,0,0,&this->foreground);
	preBlit.copy_pixels(dst->get_screen());
	bool visibilitya[]={
		!!currentPage,
		currentPage!=end-1
	};
	visibility=(visibilitya[0]<<1)|int(visibilitya[1]);
	NONS_Button *buttons[]={
		(NONS_Button *)this->up,
		(NONS_Button *)this->down
	};
	int x,y;
	getCorrectedMousePosition(dst,&x,&y);
	mouseOver=-1;
	if (visibilitya[0] || visibilitya[1]){
		for (int a=0;a<2;a++){
			if (mouseOver<0 && buttons[a]->MouseOver(x,y))
				mouseOver=a;
			if (visibilitya[a])
				buttons[a]->mergeWithoutUpdate(dst,preBlit,mouseOver==a,1);
		}
	}
	dst->updateWholeScreen();
	return 1;
}

NONS_Cursor::NONS_Cursor(NONS_ScreenSpace *screen){
	this->data=0;
	this->xpos=0;
	this->ypos=0;
	this->absolute=0;
	this->screen=screen;
}

NONS_Cursor::NONS_Cursor(const std::wstring &str,long x,long y,long absolute,NONS_ScreenSpace *screen){
	this->data=0;
	this->xpos=x;
	this->ypos=y;
	this->absolute=!!absolute;
	this->screen=screen;
	this->data=new NONS_Layer(&str);
}

NONS_Cursor::NONS_Cursor(TiXmlElement *parent,NONS_ScreenSpace *screen,const char *name){
	TiXmlElement *cursor=parent->FirstChildElement(name?name:0);
	this->xpos=cursor->QueryIntAttribute("x");
	this->ypos=cursor->QueryIntAttribute("y");
	this->absolute=!!cursor->QueryIntAttribute("absolute");
	this->screen=screen;
	std::wstring str=cursor->QueryWStringAttribute("string");
	this->data=new NONS_Layer(&str);
}

NONS_Cursor::~NONS_Cursor(){
	if (this->data)
		delete this->data;
}

int NONS_Cursor::animate(NONS_Menu *menu,ulong expiration){
	if (CURRENTLYSKIPPING)
		return 0;
	NONS_Clock clock;
	this->screen->cursor=this->data;
	this->data->position.x=this->xpos+((!this->absolute)?this->screen->output->x:0);
	this->data->position.y=this->ypos+((!this->absolute)?this->screen->output->y:0);
	bool done=0;
	NONS_EventQueue queue;
	const long delayadvance=25;
	ulong expire=expiration?expiration:ULONG_MAX;
	int ret=0;
	std::vector<NONS_LongRect> rects;
	if (this->data->animated())
		this->screen->BlendAll(1);
	while (!done && !CURRENTLYSKIPPING && expire>0){
		for (ulong a=0;!done && !CURRENTLYSKIPPING && expire>0;a+=delayadvance){
			while (!queue.empty()){
				SDL_Event event=queue.pop();
				switch (event.type){
					case SDL_QUIT:
						ret=-1;
						goto animate_000;
					case SDL_KEYDOWN:
						if (event.key.keysym.sym==SDLK_PAUSE){
							if (console){
								console->enter(this->screen);
								if (queue.emptify()){
									ret=-1;
									goto animate_000;
								}
							}
							break;
						}
						if (!menu)
							break;
						switch (event.key.keysym.sym){
							case SDLK_ESCAPE:
								if (!this->callMenu(menu,&queue)){
									ret=-1;
									goto animate_000;
								}
								break;
							case SDLK_UP:
							case SDLK_PAGEUP:
								if (!this->callLookback(&queue)){
									ret=-1;
									goto animate_000;
								}
								break;
							case SDLK_MENU:
								break;
							default:
								goto animate_000;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
						if (event.button.button==SDL_BUTTON_RIGHT){
							if (!this->callMenu(menu,&queue)){
								ret=-1;
								goto animate_000;
							}
						}else if (event.button.button==SDL_BUTTON_WHEELUP && !this->callLookback(&queue)){
							ret=-1;
							goto animate_000;
						}else
							done=1;
						break;
					default:
						break;
				}
			}
			NONS_Clock::t t0=clock.get();
			if (!!this->screen && this->screen->advanceAnimations(delayadvance,rects))
				this->screen->BlendOptimized(rects);
			NONS_Clock::t t1=clock.get()-t0,
				delay=delayadvance-t1;
			if (delay>0)
				SDL_Delay((Uint32)delay);
			expire-=delayadvance;
		}
	}
animate_000:
	if (ret!=-1){
		this->screen->BlendNoCursor(1);
		this->screen->cursor=0;
	}
	return ret;
}

TiXmlElement *NONS_Cursor::save(const char *override_name){
	if (!this->data)
		return 0;
	TiXmlElement *cursor=new TiXmlElement(override_name?override_name:"cursor");
	cursor->SetAttribute("string",this->data->animation.getString());
	cursor->SetAttribute("x",this->xpos);
	cursor->SetAttribute("y",this->ypos);
	cursor->SetAttribute("absolute",this->absolute);
	return cursor;
}

bool NONS_Cursor::callMenu(NONS_Menu *menu,NONS_EventQueue *queue){
	if (menu && menu->rightClickMode==1 && menu->buttons){
		//this->screen->BlendNoText(1);
		int ret=menu->callMenu();
		if (ret==-1 || ret==INT_MIN || queue->emptify())
			return 0;
		if (this->data->animated())
			this->screen->BlendAll(1);
		else
			this->screen->BlendNoCursor(1);
	}
	return 1;
}

bool NONS_Cursor::callLookback(NONS_EventQueue *queue){
	this->screen->BlendNoText(0);
	{
		NONS_Surface screen=this->screen->screen->get_screen();
		screen.over(this->screen->screenBuffer);
		screen.multiply(this->screen->output->shadeLayer->data,&this->screen->output->shadeLayer->clip_rect);
	}
	if (this->screen->lookback->display(this->screen->screen)==INT_MIN || queue->emptify())
		return 0;
	if (this->data->animated())
		this->screen->BlendAll(1);
	else
		this->screen->BlendNoCursor(1);
	return 1;
}

#define NONS_BUTTON_MERGE_COMMON                                \
	if (!force && this->status==status)                         \
		return;                                                 \
	NONS_LongRect srcRect=nrect,                                \
		dstRect=srcRect;                                        \
	dstRect.x=posx;                                             \
	dstRect.y=posy;                                             \
	this->status=status;                                        \
	{                                                           \
		NONS_Surface screen=dst->get_screen();                  \
		screen.over(original,&dstRect,&dstRect);                \
		this->mergeWithoutUpdate_inner(screen,dstRect,srcRect); \
	}

bool NONS_Button::MouseOver(SDL_Event *event){
	if (event->type!=SDL_MOUSEMOTION)
		return 0;
	int x=event->motion.x,
		y=event->motion.y;
	return this->MouseOver(x,y);
}

void NONS_Button::merge(
		NONS_VirtualScreen *dst,
		const NONS_LongRect &nrect,
		int posx,
		int posy,
		const NONS_ConstSurface &original,
		bool status,
		bool force){
	NONS_BUTTON_MERGE_COMMON;
	NONS_LongRect rect((NONS_LongRect)dst->inRect);
	long w=(dstRect.w+dstRect.x>rect.w)?rect.w-dstRect.x:dstRect.w,
	     h=(dstRect.h+dstRect.y>rect.h)?rect.h-dstRect.y:dstRect.h;
	dst->updateScreen(dstRect.x,dstRect.y,w,h);
}

void NONS_Button::mergeWithoutUpdate(
		NONS_VirtualScreen *dst,
		const NONS_LongRect &nrect,
		int posx,
		int posy,
		const NONS_ConstSurface &original,
		bool status,
		bool force){
	NONS_BUTTON_MERGE_COMMON;
}

NONS_SurfaceButton::~NONS_SurfaceButton(){
	delete this->getOffLayer();
	delete this->getOnLayer();
}

void NONS_SurfaceButton::mergeWithoutUpdate(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force){
	NONS_Button::mergeWithoutUpdate(dst,this->box,this->posx,this->posy,original,status,force);
}

NONS_LongRect NONS_SurfaceButton::get_dimensions(){
	NONS_LongRect rect=this->box;
	rect.x=(long)posx;
	rect.y=(long)posy;
	return rect;
}

void NONS_SurfaceButton::mergeWithoutUpdate_inner(NONS_Surface &dst,const NONS_LongRect &dstRect,const NONS_LongRect &srcRect){
	NONS_TextButton *_this=dynamic_cast<NONS_TextButton *>(this);
	if (_this && _this->getShadowLayer()){
		NONS_LongRect temp=dstRect;
		temp.x++;
		temp.y++;
		dst.over(_this->setShadowLayer()->data,&temp);
	}
	if (this->status)
		dst.over(this->getOnLayer()->data,&dstRect);
	else if (this->getOffLayer())
		dst.over(this->getOffLayer()->data,&dstRect);
}

void NONS_SurfaceButton::merge(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force){
	NONS_Button::merge(dst,this->box,this->posx,this->posy,original,status,force);
}

NONS_GraphicButton::NONS_GraphicButton(const NONS_ConstSurface &src,int posx,int posy,int width,int height,int originX,int originY)
		:NONS_SurfaceButton(){
	this->allocateLayer(this->setOnLayer(),src,posx,posy,width,height,originX,originY);
	this->posx=posx;
	this->posy=posy;
	this->box.w=width;
	this->box.h=height;
}

void NONS_GraphicButton::allocateLayer(
		NONS_Layer *&layer,
		const NONS_ConstSurface &src,
		int posx,
		int posy,
		int width,
		int height,
		int originX,
		int originY){
	NONS_LongRect dst(0,0,width,height),
		srcRect(originX,originY,width,height);
	delete layer;
	layer=new NONS_Layer(dst,NONS_Color::black_transparent);
	layer->data.over(src,&dst,&srcRect);
}

NONS_TextButton::NONS_TextButton(
		const std::wstring &text,
		const NONS_FontCache &fc,
		float center,
		const NONS_Color &on,
		const NONS_Color &off,
		bool shadow,
		int limitX,
		int limitY)
		:NONS_SurfaceButton(),font_cache(fc FONTCACHE_DEBUG_PARAMETERS){
	this->limitX=limitX;
	this->limitY=limitY;
	NONS_LongRect limit(0,0,limitX,limitY);
	this->font_cache.set_color(on);
	NONS_Surface text_surface(text,this->font_cache,limit,center,0);
	this->box=text_surface.clip_rect();
	this->setOnLayer()=new NONS_Layer(text_surface,on);
	text_surface=text_surface.clone();
	text_surface.color(off);
	this->setOffLayer()=new NONS_Layer(text_surface,off);
	if (shadow){
		text_surface=text_surface.clone();
		text_surface.color(NONS_Color::black);
		this->setShadowLayer()=new NONS_Layer(text_surface,NONS_Color::black);
		this->box.w++;
		this->box.h++;
	}else
		this->setShadowLayer()=0;
}

NONS_TextButton::~NONS_TextButton(){
	delete this->getShadowLayer();
}

void NONS_SpriteButton::mergeWithoutUpdate_inner(NONS_Surface &dst,const NONS_LongRect &dstRect,const NONS_LongRect &srcRect){
	NONS_LongRect temp=srcRect;
	temp.x=(!this->status)?0:temp.w;
	NONS_Layer *layer=this->screen->layerStack[this->sprite];
	dst.over_with_alpha(layer->data,&dstRect,&temp,layer->alpha);
}

void NONS_SpriteButton::mergeWithoutUpdate(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force){
	NONS_Layer *layer=this->screen->layerStack[this->sprite];
	NONS_Button::mergeWithoutUpdate(dst,layer->clip_rect,(int)layer->position.x,(int)layer->position.y,original,status,force);
}

void NONS_SpriteButton::merge(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force){
	NONS_Layer *layer=this->screen->layerStack[this->sprite];
	NONS_Button::merge(dst,layer->clip_rect,(int)layer->position.x,(int)layer->position.y,original,status,force);
}

bool NONS_SpriteButton::MouseOver(int x,int y){
	NONS_Layer *layer=this->screen->layerStack[this->sprite];
	return MOUSE_OVER(x,y,layer->position.x,layer->position.y,layer->clip_rect.w,layer->clip_rect.h);
}

NONS_LongRect NONS_SpriteButton::get_dimensions(){
	NONS_Layer *layer=this->screen->layerStack[this->sprite];
	return NONS_LongRect(
		layer->position.x,
		layer->position.y,
		layer->clip_rect.w,
		layer->clip_rect.h
	);
}

NONS_ButtonLayer::NONS_ButtonLayer(const NONS_FontCache &fc,NONS_ScreenSpace *screen,bool exitable,NONS_Menu *menu){
	this->font_cache=new NONS_FontCache(fc FONTCACHE_DEBUG_PARAMETERS);
	this->screen=screen;
	this->exitable=exitable;
	this->menu=menu;
	this->return_on_down=0;
}

NONS_ButtonLayer::NONS_ButtonLayer(const NONS_Surface &img,NONS_ScreenSpace *screen)
		:loadedGraphic(img){
	this->font_cache=0;
	this->screen=screen;
	this->return_on_down=0;
}

NONS_ButtonLayer::~NONS_ButtonLayer(){
	for (map_t::iterator i=this->buttons.begin(),e=this->buttons.end();i!=e;++i)
		delete i->second;
	delete this->font_cache;
}

void NONS_ButtonLayer::makeTextButtons(const std::vector<std::wstring> &arr,
		const NONS_Color &on,
		const NONS_Color &off,
		bool shadow,
		std::wstring *entry,
		std::wstring *mouseover,
		std::wstring *click,
		NONS_Audio *audio,
		int width,
		int height){
	if (!this->font_cache)
		return;
	for (map_t::iterator i=this->buttons.begin(),e=this->buttons.end();i!=e;++i)
		delete i->second;
	this->buttons.clear();
	this->audio=audio;
	if (entry)
		this->voiceEntry=*entry;
	if (click)
		this->voiceClick=*click;
	if (mouseover)
		this->voiceMouseOver=*mouseover;
	this->boundingBox.x=0;
	this->boundingBox.y=0;
	this->boundingBox.w=0;
	this->boundingBox.h=0;
	for (ulong a=0;a<arr.size();a++){
		NONS_TextButton *button=new NONS_TextButton(arr[a],*this->font_cache,0,on,off,shadow,width,height);
		this->buttons[a]=button;
		this->boundingBox.h+=(Uint16)button->getBox().h;
		if (button->getBox().w>this->boundingBox.w)
			this->boundingBox.w=(Uint16)button->getBox().w;
	}
}

int NONS_ButtonLayer::getUserInput(int x,int y,bool override_placement){
	if (!this->buttons.size())
		return -1;
	if (override_placement){
		for (map_t::iterator i=this->buttons.begin(),e=this->buttons.end();i!=e;++i){
			NONS_TextButton *cB=(NONS_TextButton *)i->second;
			cB->setPosx()=x;
			cB->setPosy()=y;
			y+=int(cB->getBox().y+cB->getBox().h);
		}
		if (y>this->screen->output->y0+this->screen->output->h)
			return -2;
	}
	if (this->voiceEntry.size())
		this->audio->play_sound_once(this->voiceEntry);
	NONS_ScopedAudioStream voiceMouseOver_channel(this->audio,this->voiceMouseOver);
	NONS_EventQueue queue;
	NONS_Surface screenCopy=this->screen->screen->get_screen().clone();
	map_t::iterator mouseOver=this->buttons.end();
	getCorrectedMousePosition(this->screen->screen,&x,&y);
	map_t::iterator e=this->buttons.end();
	for (map_t::iterator i=this->buttons.begin();i!=e;++i){
		NONS_Button *b=i->second;
		if (!b)
			continue;
		bool state=(b->MouseOver(x,y) && mouseOver==e);
		if (state)
			mouseOver=i;
		b->mergeWithoutUpdate(this->screen->screen,screenCopy,state,1);
	}
	this->screen->screen->updateWholeScreen();
	while (1){
		queue.WaitForEvent(10);
		SDL_Event event=queue.pop();
		//Handle entering to lookback.
		if (event.type==SDL_KEYDOWN && (/*event.key.keysym.sym==SDLK_UP || */event.key.keysym.sym==SDLK_PAGEUP) ||
				event.type==SDL_MOUSEBUTTONDOWN && (event.button.button==SDL_BUTTON_WHEELUP || event.button.button==SDL_BUTTON_WHEELDOWN)){
			this->screen->BlendNoText(0);
			this->screen->screen->blitToScreen(this->screen->screenBuffer,0,0);
			{
				NONS_Surface screen=this->screen->screen->get_screen();
				screen.multiply(this->screen->output->shadeLayer->data,0,&this->screen->output->shadeLayer->clip_rect);
				if (this->screen->lookback->display(this->screen->screen)==INT_MIN || queue.emptify())
					return INT_MIN;
				screen.copy_pixels(screenCopy);
			}
			getCorrectedMousePosition(this->screen->screen,&x,&y);
			for (map_t::iterator i=this->buttons.begin(),e=this->buttons.end();i!=e;++i){
				NONS_Button *b=i->second;
				if (!b)
					continue;
				bool state=(b->MouseOver(x,y) && mouseOver==e);
				if (state)
					mouseOver=i;
				b->mergeWithoutUpdate(this->screen->screen,screenCopy,state,1);
			}
			this->screen->screen->updateWholeScreen();
			continue;
		}
		switch (event.type){
			case SDL_QUIT:
				return INT_MIN;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym){
					case SDLK_ESCAPE:
						if (this->exitable){
							this->screen->screen->blitToScreen(screenCopy,0,0);
							this->screen->screen->updateWholeScreen();
							return -1;
						}else if (this->menu){
							this->screen->screen->blitToScreen(screenCopy,0,0);
							int ret=this->menu->callMenu();
							if (ret<0)
								return (ret==INT_MIN)?INT_MIN:-3;
							if (queue.emptify())
								return INT_MIN;
							this->screen->screen->blitToScreen(screenCopy,0,0);
							getCorrectedMousePosition(this->screen->screen,&x,&y);
							for (map_t::iterator i=this->buttons.begin(),e=this->buttons.end();i!=e;++i){
								NONS_Button *b=i->second;
								if (!b)
									continue;
								bool state=(b->MouseOver(x,y) && mouseOver==e);
								if (state)
									mouseOver=i;
								b->mergeWithoutUpdate(this->screen->screen,screenCopy,state,1);
							}
							this->screen->screen->updateWholeScreen();
						}
						break;
					case SDLK_UP:
					case SDLK_DOWN:
						this->react_to_updown(mouseOver,event.key.keysym.sym,screenCopy);
						voiceMouseOver_channel.play(0);
						break;
					case SDLK_RETURN:
						if (this->react_to_click(mouseOver,screenCopy))
							return mouseOver->first;
						break;
					case SDLK_PAUSE:
						if (console){
							console->enter(this->screen);
							if (queue.emptify())

								return INT_MIN;
						}
						break;
					default:
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				if (this->react_to_movement(mouseOver,&event,screenCopy) && this->voiceMouseOver.size())
					voiceMouseOver_channel.play(0);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (this->react_to_click(mouseOver,screenCopy))
					return mouseOver->first;
				break;
		}
	}
}

bool NONS_ButtonLayer::react_to_movement(map_t::iterator &mouseOver,SDL_Event *event,const NONS_ConstSurface &screenCopy){
	map_t::iterator e=this->buttons.end(),
		tempMO=e;
	if (mouseOver!=e && mouseOver->second->MouseOver(event))
		return 0;
	for (map_t::iterator i=this->buttons.begin();i!=e && tempMO==e;++i)
		if (i->second->MouseOver(event))
			tempMO=i;
	if (tempMO==e){
		if (mouseOver!=e)
			mouseOver->second->merge(this->screen->screen,screenCopy,0);
		mouseOver=e;
		return 0;
	}
	if (mouseOver!=e)
		mouseOver->second->merge(this->screen->screen,screenCopy,0);
	mouseOver=tempMO;
	mouseOver->second->merge(this->screen->screen,screenCopy,1);
	return 1;
}

void NONS_ButtonLayer::react_to_updown(map_t::iterator &mouseOver,SDLKey key,const NONS_ConstSurface &screenCopy){
	map_t::iterator e=this->buttons.end();
	if (mouseOver!=e)
		mouseOver->second->merge(this->screen->screen,screenCopy,0);
	mouseOver=advance_iterator(mouseOver,(key==SDLK_UP)?-1:1,this->buttons);
	NONS_Button *button=mouseOver->second;
	if (!button)
		return;
	button->merge(this->screen->screen,screenCopy,1);
	NONS_LongRect rect=button->get_dimensions();
	rect.x=rect.x+rect.w/2;
	rect.y=rect.y+rect.h/2;
	SDL_WarpMouse((Uint16)this->screen->screen->convertX((float)rect.x),(Uint16)this->screen->screen->convertY((float)rect.y));
}

bool NONS_ButtonLayer::react_to_click(map_t::iterator &mouseOver,const NONS_ConstSurface &screenCopy){
	map_t::iterator e=this->buttons.end();
	if (mouseOver==e)
		return 0;
	if (this->voiceClick.size())
		this->audio->play_sound_once(this->voiceClick);
	this->screen->screen->get_screen().copy_pixels(screenCopy);
	this->screen->screen->updateWholeScreen();
	return 1;
}

void NONS_ButtonLayer::addImageButton(ulong index,int posx,int posy,int width,int height,int originX,int originY){
	delete this->buttons[index];
	this->buttons[index]=new NONS_GraphicButton(this->loadedGraphic,posx,posy,width,height,originX,originY);
}

void NONS_ButtonLayer::addSpriteButton(ulong index,ulong sprite){
	delete this->buttons[index];
	this->buttons[index]=new NONS_SpriteButton(sprite,this->screen);
}

int NONS_ButtonLayer::getUserInput(ulong expiration){
	if (!this->buttons.size()){
		NONS_LongRect rect=this->loadedGraphic.clip_rect();
		this->addImageButton(0,0,0,rect.w,rect.h,0,0);
		//return LONG_MIN;
	}
	NONS_EventQueue queue;
	this->screen->BlendNoText(0);
	this->screen->copyBufferToScreenWithoutUpdating();
	NONS_Surface screenCopy=this->screen->screen->get_screen().clone();
	map_t::iterator e=this->buttons.end(),
		mouseOver=e;
	int x,y;
	getCorrectedMousePosition(this->screen->screen,&x,&y);
	for (map_t::iterator i=this->buttons.begin();i!=e;++i){
		NONS_Button *b=i->second;
		if (!b)
			continue;
		bool state=(b->MouseOver(x,y) && mouseOver==e);
		if (state)
			mouseOver=i;
		b->mergeWithoutUpdate(this->screen->screen,screenCopy,state,1);
	}
	this->screen->screen->updateWholeScreen();
	long expire=expiration;


	std::map<SDLKey,std::pair<int,bool *> > key_bool_map;
	key_bool_map[SDLK_ESCAPE]=  std::make_pair(-10,&this->inputOptions.EscapeSpace);
	key_bool_map[SDLK_SPACE]=   std::make_pair(-11,&this->inputOptions.EscapeSpace);
	key_bool_map[SDLK_PAGEUP]=  std::make_pair(-12,&this->inputOptions.PageUpDown);
	key_bool_map[SDLK_PAGEDOWN]=std::make_pair(-13,&this->inputOptions.PageUpDown);
	key_bool_map[SDLK_RETURN]=  std::make_pair(-19,&this->inputOptions.Enter);
	key_bool_map[SDLK_TAB]=     std::make_pair(-20,&this->inputOptions.Tab);
	key_bool_map[SDLK_F1]=      std::make_pair(-21,&this->inputOptions.Function);
	key_bool_map[SDLK_F2]=      std::make_pair(-22,&this->inputOptions.Function);
	key_bool_map[SDLK_F3]=      std::make_pair(-23,&this->inputOptions.Function);
	key_bool_map[SDLK_F4]=      std::make_pair(-24,&this->inputOptions.Function);
	key_bool_map[SDLK_F5]=      std::make_pair(-25,&this->inputOptions.Function);
	key_bool_map[SDLK_F6]=      std::make_pair(-26,&this->inputOptions.Function);
	key_bool_map[SDLK_F7]=      std::make_pair(-27,&this->inputOptions.Function);
	key_bool_map[SDLK_F8]=      std::make_pair(-28,&this->inputOptions.Function);
	key_bool_map[SDLK_F9]=      std::make_pair(-29,&this->inputOptions.Function);
	key_bool_map[SDLK_F10]=     std::make_pair(-30,&this->inputOptions.Function);
	key_bool_map[SDLK_F11]=     std::make_pair(-31,&this->inputOptions.Function);
	key_bool_map[SDLK_F12]=     std::make_pair(-32,&this->inputOptions.Function);
	key_bool_map[SDLK_UP]=      std::make_pair(-40,&this->inputOptions.Cursor);
	key_bool_map[SDLK_RIGHT]=   std::make_pair(-41,&this->inputOptions.Cursor);
	key_bool_map[SDLK_DOWN]=    std::make_pair(-42,&this->inputOptions.Cursor);
	key_bool_map[SDLK_LEFT]=    std::make_pair(-43,&this->inputOptions.Cursor);
	key_bool_map[SDLK_INSERT]=  std::make_pair(-50,&this->inputOptions.Insert);
	key_bool_map[SDLK_z]=       std::make_pair(-51,&this->inputOptions.ZXC);
	key_bool_map[SDLK_x]=       std::make_pair(-52,&this->inputOptions.ZXC);
	key_bool_map[SDLK_c]=       std::make_pair(-53,&this->inputOptions.ZXC);

	while (expiration && expire>0 || !expiration){
		while (!queue.empty()){
			SDL_Event event=queue.pop();
			switch (event.type){
				case SDL_QUIT:
					return INT_MIN;
				case SDL_MOUSEMOTION:
					this->react_to_movement(mouseOver,&event,screenCopy);
					break;
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
					if (!this->return_on_down && event.type==SDL_MOUSEBUTTONUP || this->return_on_down && event.type==SDL_MOUSEBUTTONDOWN){
						int button;
						switch (event.button.button){
							case SDL_BUTTON_LEFT:
								button=1;
								break;
							case SDL_BUTTON_RIGHT:
								button=2;
								break;
							case SDL_BUTTON_WHEELUP:
								button=3;
								break;
							case SDL_BUTTON_WHEELDOWN:
								button=4;
								break;
							default:
								button=-1;
						}
						if (button<0)
							break;
						this->screen->screen->updateWholeScreen();
						if (button==1)
							return (mouseOver==e)?-1:mouseOver->first;
						else
							return -button;
					}
					break;
				case SDL_KEYDOWN:
					{
						SDLKey key=event.key.keysym.sym;
						switch (key){
							case SDLK_PAUSE:
								if (console){
									console->enter(this->screen);
									if (queue.emptify())
										return INT_MIN;
								}
								break;
							case SDLK_UP:
							case SDLK_DOWN:
								if (!*key_bool_map[key].second){
									this->react_to_updown(mouseOver,key,screenCopy);
									break;
								}
							case SDLK_RETURN:
								if (key==SDLK_RETURN && !*key_bool_map[key].second){
									this->screen->screen->updateWholeScreen();
									return (mouseOver==e)?-1:mouseOver->first;
								}
							default:
								{
									std::map<SDLKey,std::pair<int,bool *> >::iterator i=key_bool_map.find(key);
									int ret=0;
									if (this->inputOptions.space_returns_left_click && key==SDLK_SPACE)
										ret=1;
									else if (i!=key_bool_map.end()){
										if (*(i->second.second))
											ret=i->second.first-1;
										else if (key==SDLK_ESCAPE)
											ret=-2;
									}
									if (ret){
										this->screen->screen->updateWholeScreen();
										return ret;
									}
								}
						}
					}
					break;
			}
		}
		SDL_Delay(10);
		expire-=10;
	}
	return (this->inputOptions.Wheel)?-5:-2;
}

NONS_Menu::NONS_Menu(NONS_ScriptInterpreter *interpreter)
		:off(0xAA,0xAA,0xAA),on(0xFF,0xFF,0xFF){
	this->interpreter=interpreter;
	this->nofile=this->off;
	this->shadow=1;
	this->font_cache=0;
	this->default_font_cache=interpreter->font_cache;
	this->buttons=0;
	NONS_ScreenSpace *scr=interpreter->screen;
	this->shade=new NONS_Layer(scr->screen->get_screen().clip_rect(),NONS_Color(0xCC,0xCC,0xCC));
	this->slots=10;
	this->audio=interpreter->audio;
	this->rightClickMode=1;
}

NONS_Menu::NONS_Menu(std::vector<std::wstring> *options,NONS_ScriptInterpreter *interpreter)
		:off(0xA9,0xA9,0xA9),on(0xFF,0xFF,0xFF){
	this->interpreter=interpreter;
	for (ulong a=0;a<options->size();a++){
		this->strings.push_back((*options)[a++]);
		this->commands.push_back((*options)[a]);
	}
	this->shadow=1;
	this->font_cache=0;
	NONS_ScreenSpace *scr=interpreter->screen;
	this->default_font_cache=interpreter->font_cache;
	this->buttons=new NONS_ButtonLayer(*this->default_font_cache,scr,1,0);
	int w,h;
	{
		NONS_LongRect rect=scr->screen->get_screen().clip_rect();
		w=rect.w;
		h=rect.h;
	}
	this->audio=interpreter->audio;
	this->buttons->makeTextButtons(
		this->strings,
		this->on,
		this->off,
		this->shadow,
		&this->voiceEntry,
		&this->voiceMO,
		&this->voiceClick,
		this->audio,
		w,
		h);
	this->x=(w-this->buttons->boundingBox.w)/2;
	this->y=(h-this->buttons->boundingBox.h)/2;
	this->shade=new NONS_Layer(scr->screen->get_screen().clip_rect(),NONS_Color(0xCC,0xCC,0xCC));
	this->rightClickMode=1;
}

NONS_Menu::~NONS_Menu(){
	delete this->buttons;
	delete this->font_cache;
	delete this->shade;
}

int NONS_Menu::callMenu(){
	this->interpreter->screen->BlendNoText(0);
	NONS_Surface &s=this->interpreter->screen->screenBuffer;
	s.multiply(this->shade->data);
	this->interpreter->screen->screen->get_screen().over(s);
	int choice=this->buttons->getUserInput(this->x,this->y);
	if (choice<0){
		if (choice!=INT_MIN && this->voiceCancel.size())
			this->audio->play_sound_once(this->voiceCancel);
		return 0;
	}
	return this->call(this->commands[choice]);
}

void NONS_Menu::reset(){
	delete this->buttons;
	delete this->font_cache;
	this->font_cache=new NONS_FontCache(*this->default_font_cache FONTCACHE_DEBUG_PARAMETERS);
	this->font_cache->spacing=this->spacing;
	this->font_cache->line_skip=this->lineskip;
	this->shade->setShade(this->shadeColor);
	NONS_ScreenSpace *scr=this->interpreter->screen;
	this->buttons=new NONS_ButtonLayer(*this->font_cache,scr,1,0);
	int w,h;
	{
		NONS_LongRect rect=scr->screen->get_screen().clip_rect();
		w=rect.w;
		h=rect.h;
	}
	this->buttons->makeTextButtons(
		this->strings,
		this->on,
		this->off,
		this->shadow,
		0,0,0,0,
		w,h);
	this->x=(w-this->buttons->boundingBox.w)/2;
	this->y=(h-this->buttons->boundingBox.h)/2;
}

void NONS_Menu::resetStrings(std::vector<std::wstring> *options){
	if (this->buttons)
		delete this->buttons;
	this->strings.clear();
	this->commands.clear();
	for (ulong a=0;a<options->size();a++){
		this->strings.push_back((*options)[a++]);
		this->commands.push_back((*options)[a]);
	}
	NONS_ScreenSpace *scr=interpreter->screen;
	this->buttons=new NONS_ButtonLayer(this->get_font_cache(),scr,1,0);
	int w,h;
	{
		NONS_LongRect rect=scr->screen->get_screen().clip_rect();
		w=rect.w;
		h=rect.h;
	}
	this->buttons->makeTextButtons(
		this->strings,
		this->on,
		this->off,
		this->shadow,
		0,0,0,0,
		w,h);
	this->x=(w-this->buttons->boundingBox.w)/2;
	this->y=(h-this->buttons->boundingBox.h)/2;
}

int NONS_Menu::write(const std::wstring &txt,int y){
	NONS_FontCache tempCacheForeground(this->get_font_cache() FONTCACHE_DEBUG_PARAMETERS),
		tempCacheShadow(tempCacheForeground FONTCACHE_DEBUG_PARAMETERS);
	if (this->shadow)
		tempCacheShadow.set_color(NONS_Color::black);
	
	std::vector<NONS_Glyph *> outputBuffer;
	std::vector<NONS_Glyph *> outputBuffer2;
	ulong width=0;
	for (size_t a=0;a<txt.size();a++){
		NONS_Glyph *glyph=tempCacheForeground.get_glyph(txt[a]);
		width+=glyph->get_advance();
		outputBuffer.push_back(glyph);
		if (this->shadow)
			outputBuffer2.push_back(tempCacheShadow.get_glyph(txt[a]));
	}
	NONS_ScreenSpace *scr=interpreter->screen;
	int w=scr->screen->get_screen().clip_rect().w;
	int x=(w-width)/2;
	for (ulong a=0;a<outputBuffer.size();a++){
		int advance=outputBuffer[a]->get_advance();
		{
			NONS_Surface screen=scr->screen->get_screen();
			if (this->shadow){
				outputBuffer2[a]->put(screen,x+1,y+1);
				outputBuffer2[a]->put(screen,x+1,y+1);
				outputBuffer2[a]->done();
			}
			outputBuffer[a]->put(screen,x,y);
			outputBuffer[a]->done();
		}
		x+=advance;
	}
	return this->get_font_cache().line_skip;
}

extern std::wstring save_directory;

int NONS_Menu::save(){
	int y0;
	if (this->stringSave.size())
		y0=this->write(this->stringSave,20);
	else
		y0=this->write(L"~~ Save File ~~",20);
	NONS_ScreenSpace *scr=interpreter->screen;
	NONS_ButtonLayer files(this->get_font_cache(),scr,1,0);
	std::vector<tm *> times=existing_files(save_directory);
	int choice,
		ret;
	while (1){
		std::vector<std::wstring> strings;
		std::wstring pusher;
		for (ulong a=0;a<slots;a++){
			tm *t=times[a];
			if (this->stringSlot.size())
				pusher=this->stringSlot;
			else
				pusher=L"Slot";
			pusher+=L" "+itoaw(a+1,2)+L"    ";
			if (t)
				pusher.append(getTimeString<wchar_t>(t));
			else
				pusher+=L"-------------------";
			strings.push_back(pusher);
		}
		int w,h;
		{
			NONS_LongRect rect=scr->screen->get_screen().clip_rect();
			w=rect.w;
			h=rect.h;
		}
		files.makeTextButtons(
			strings,
			this->on,
			this->off,
			this->shadow,
			&this->voiceEntry,
			&this->voiceMO,
			&this->voiceClick,
			this->audio,
			w,h);
		choice=files.getUserInput((w-files.boundingBox.w)/2,y0*2+20);
		if (choice==INT_MIN)
			ret=INT_MIN;
		else{
			if (choice==-2){
				this->slots--;
				continue;
			}
			if (choice<0 && this->voiceCancel.size())
				this->audio->play_sound_once(this->voiceCancel);
			ret=choice+1;
		}
		break;
	}
	for (ulong a=0;a<times.size();a++)
		if (times[a])
			delete times[a];
	return ret;
}

int NONS_Menu::load(){
	int y0;
	if (this->stringLoad.size())
		y0=this->write(this->stringLoad,20);
	else
		y0=this->write(L"~~ Load File ~~",20);
	NONS_ScreenSpace *scr=interpreter->screen;
	NONS_ButtonLayer files(this->get_font_cache(),scr,1,0);
	std::vector<tm *> times=existing_files(save_directory);
	int choice,
		ret;
	std::vector<std::wstring> strings;
	{
		std::wstring pusher;
		for (ulong a=0;a<slots;a++){
			tm *t=times[a];
			if (this->stringSlot.size())
				pusher=this->stringSlot;
			else
				pusher=L"Slot";
			pusher+=L" "+itoaw(a+1,2)+L"    ";
			if (t)
				pusher.append(getTimeString<wchar_t>(t));
			else
				pusher+=L"-------------------";
			strings.push_back(pusher);
		}
	}
	while (1){
		int w,h;
		{
			NONS_LongRect rect=scr->screen->get_screen().clip_rect();
			w=rect.w;
			h=rect.h;
		}
		files.makeTextButtons(
			strings,
			this->on,
			this->off,
			this->shadow,
			&this->voiceEntry,
			&this->voiceMO,
			&this->voiceClick,
			this->audio,
			w,h);
		choice=files.getUserInput((w-files.boundingBox.w)/2,y0*2+20);
		if (choice==INT_MIN)
			ret=INT_MIN;
		else{
			if (choice==-2){
				this->slots--;
				strings.pop_back();
				continue;
			}
			if (choice<0 && this->voiceCancel.size())
				this->audio->play_sound_once(this->voiceCancel);
			ret=choice+1;
		}
		break;
	}
	for (ulong a=0;a<times.size();a++)
		if (times[a])
			delete times[a];
	return ret;
}

int NONS_Menu::windowerase(){
	this->interpreter->screen->BlendNoText(1);
	NONS_EventQueue queue;
	while (1){
		queue.WaitForEvent();
		while (!queue.empty()){
			SDL_Event event=queue.pop();
			if (event.type==SDL_KEYDOWN || event.type==SDL_MOUSEBUTTONDOWN)
				return 0;
			if (event.type==SDL_QUIT)
				return INT_MIN;
		}
	}
}

int NONS_Menu::skip(){
	ctrlIsPressed=1;
	return 0;
}

int NONS_Menu::call(const std::wstring &string){
	int ret=0;
	if (string==L"reset")
		ret=-1;
	if (string==L"save"){
		int save=this->save();
		if (save>0){
			this->interpreter->save(save);
			//ret=-1;
		}else if (save==INT_MIN)
			ret=INT_MIN;
	}else if (string==L"load"){
		int load=this->load();
		if (load>0 && this->interpreter->load(load))
			ret=-1;
	}else if (string==L"windowerase"){
		ret=this->windowerase();
	}else if (string==L"lookback"){
		NONS_ScreenSpace *scr=this->interpreter->screen;
		scr->BlendNoText(0);
		{
			NONS_Surface screen=scr->screen->get_screen();
			screen.multiply(scr->output->shadeLayer->data,0,&scr->output->shadeLayer->clip_rect);
		}
		ret=scr->lookback->display(scr->screen);
	}else if (string==L"skip"){
		this->skip();
	}else{
		ErrorCode error=this->interpreter->interpretString(string,0,0);
		if (error==NONS_END)
			ret=INT_MIN;
		else if (error!=NONS_NO_ERROR)
			handleErrors(error,-1,"NONS_Menu::call",1);
	}
	return ret;
}

NONS_FreeType_Lib NONS_FreeType_Lib::instance;

NONS_FreeType_Lib::NONS_FreeType_Lib(){
	if (FT_Init_FreeType(&this->library)){
		//throw std::string("FT_Init_FreeType() has failed!");
		exit(1);
	}
}

NONS_FreeType_Lib::~NONS_FreeType_Lib(){
	FT_Done_FreeType(this->library);
}

ulong NONS_FT_Stream_IoFunc(FT_Stream s,ulong offset,uchar *buffer,ulong count){
	NONS_DataStream *stream=(NONS_DataStream *)(s->descriptor.pointer);
	if (!count)
		return 0;
	ulong o=(ulong)stream->seek(offset,1);
	size_t a=0;
	//if (count){
		a=(size_t)count;
		if (!stream->read(buffer,a,a))
			a=0;
		o+=a;
	//}
	//s->pos=o;
	return (ulong)a;
}

void NONS_FT_Stream_CloseFunc(FT_Stream s){}

NONS_Font::NONS_Font(const std::wstring &filename){
	this->error=1;
	this->stream=0;
	NONS_DataStream *stream=general_archive.open(filename);
	if (!stream)
		return;
	this->stream=new FT_StreamRec;
	memset(this->stream,0,sizeof(*this->stream));
	this->stream->descriptor.pointer=stream;
	this->stream->read=NONS_FT_Stream_IoFunc;
	this->stream->close=NONS_FT_Stream_CloseFunc;
	this->stream->size=(size_t)stream->get_size();
	FT_Open_Args args;
	args.flags=FT_OPEN_STREAM;
	args.stream=this->stream;
	this->error=FT_Open_Face(NONS_FreeType_Lib::instance.get_lib(),&args,0,&this->ft_font);
	if (!this->good())
		general_archive.close(stream);
	this->size=0;
}

NONS_Font::~NONS_Font(){
	if (this->good())
		FT_Done_Face(this->ft_font);
	delete this->stream;
}

void NONS_Font::set_size(ulong size){
	if (this->size==size)
		return;
	this->size=size;
	FT_Set_Pixel_Sizes(this->ft_font,0,size);
	FT_Fixed scale=this->ft_font->size->metrics.y_scale;
	this->ascent=FT_CEIL(FT_MulFix(this->ft_font->ascender,scale));
	this->line_skip=FT_CEIL(FT_MulFix(this->ft_font->height,scale));
}

bool NONS_Font::is_monospace() const{
	return this->check_flag(FT_FACE_FLAG_FIXED_WIDTH);
}

FT_GlyphSlot NONS_Font::get_glyph(wchar_t codepoint,bool italic,bool bold) const{
	FT_Load_Glyph(this->ft_font,FT_Get_Char_Index(this->ft_font,codepoint),FT_LOAD_FORCE_AUTOHINT);
	if (italic){
		FT_Matrix shear;
		shear.xx=0x10000;
		shear.xy=0x34FE; //~0.207
		shear.yx=0;
		shear.yy=0x10000;
		FT_Outline_Transform(&this->ft_font->glyph->outline,&shear);
	}
	if (bold)
		FT_Outline_Embolden(&this->ft_font->glyph->outline,FT_Pos(this->size*177/100)); //32 for every 18 of this->size
	return this->ft_font->glyph;
}

FT_GlyphSlot NONS_Font::render_glyph(wchar_t codepoint,bool italic,bool bold) const{
	return this->render_glyph(this->get_glyph(codepoint,italic,bold));
}

FT_GlyphSlot NONS_Font::render_glyph(FT_GlyphSlot slot) const{
	FT_Render_Glyph(slot,FT_RENDER_MODE_LIGHT);
	return this->ft_font->glyph;
}

struct Span{
	int x, y, w, alpha;
	Span(){}
	Span(int x,int y,int w,int coverage):x(x),y(y),w(w),alpha(coverage){}
};

typedef std::vector<Span> Spans;

void RasterCallback(int y,int count,const FT_Span *spans,void *user) {
	for (int i=0;i<count;i++){
		Span s(spans[i].x,y,spans[i].len,spans[i].coverage);
		((Spans *)user)->push_back(s);
	}
}

void RenderSpans(FT_Library library,FT_Outline *outline,Spans *spans){
	FT_Raster_Params params;
	memset(&params,0,sizeof(params));
	params.flags=FT_RASTER_FLAG_AA|FT_RASTER_FLAG_DIRECT;
	params.gray_spans=RasterCallback;
	params.user=spans;
	FT_Outline_Render(library,outline,&params);
}

uchar *render_glyph(NONS_LongRect &box,FT_Glyph &glyph,int ascent,float outline_size){
	FT_Stroker stroker;
	FT_Stroker_New(NONS_FreeType_Lib::instance.get_lib(),&stroker);
	FT_Stroker_Set(stroker,FT_Fixed(outline_size)*64,FT_STROKER_LINECAP_ROUND,FT_STROKER_LINEJOIN_BEVEL,0);
	FT_Glyph_StrokeBorder(&glyph,stroker,0,1);

	Spans outlineSpans;
	RenderSpans(NONS_FreeType_Lib::instance.get_lib(),&FT_OutlineGlyph(glyph)->outline,&outlineSpans);
	FT_Stroker_Done(stroker);

	if (!outlineSpans.size())
		return 0;
	int minx=outlineSpans.front().x,
		miny=outlineSpans.front().y,
		maxx=outlineSpans.front().x+outlineSpans.front().w,
		maxy=outlineSpans.front().y;
	for (size_t a=1;a<outlineSpans.size();a++){
		minx=std::min(minx,outlineSpans[a].x);
		miny=std::min(miny,outlineSpans[a].y);
		maxx=std::max(maxx,outlineSpans[a].x+outlineSpans[a].w);
		maxy=std::max(maxy,outlineSpans[a].y);
	}
	box.x=minx;
	box.y=ascent-maxy-1;
	box.w=maxx-minx+1;
	box.h=maxy-miny+1;
	size_t bmsize=box.w*box.h;
	if (!bmsize)
		return 0;
	uchar *bitmap=new uchar[bmsize];
	memset(bitmap,0,bmsize);
	for (size_t a=0;a<outlineSpans.size();a++){
		Span &span=outlineSpans[a];
		ulong element=span.x-minx+(box.h-(span.y-miny)-1)*box.w;
		uchar *dst=bitmap+element;
		for (int b=0;b<span.w;b++)
			*dst++=(uchar)span.alpha;
	}
	return bitmap;
}

NONS_Glyph::NONS_Glyph(
		NONS_FontCache &fc,
		wchar_t codepoint,
		ulong size,
		const NONS_Color &color,
		bool italic,
		bool bold,
		ulong outline_size,
		const NONS_Color &outline_color)
			:fc(fc){
	this->codepoint=codepoint;
	this->size=size;
	this->outline_size=outline_size;
	this->color=color;
	this->outline_color=outline_color;
	this->refCount=0;
	this->italic=italic;
	this->bold=bold;
	NONS_Font &font=fc.get_font();
	font.set_size(size);

	FT_GlyphSlot glyph_slot;
	glyph_slot=font.get_glyph(codepoint,italic,bold);
	this->real_outline_size=0;
	if (outline_size){
		FT_Glyph glyph;
		FT_Get_Glyph(glyph_slot,&glyph);
		float temp_os=float(outline_size)/18.f*float(size);
		this->real_outline_size=(ulong)ceil(temp_os);
		this->outline_base_bitmap=render_glyph(this->outline_bounding_box,glyph,fc.ascent,temp_os);
		FT_Done_Glyph(glyph);
	}else
		this->outline_base_bitmap=0;
	
	glyph_slot=font.render_glyph(glyph_slot);
	FT_Bitmap &bitmap=glyph_slot->bitmap;
	ulong width=bitmap.width,
		height=bitmap.rows;
	uchar *dst=this->base_bitmap=new uchar[1+width*height];
	for (ulong y=0;y<(ulong)bitmap.rows;y++){
		uchar *src=bitmap.buffer+y*bitmap.pitch;
		for (ulong x=0;x<(ulong)bitmap.width;x++)
			*dst++=*src++;
	}

	this->bounding_box.x=glyph_slot->bitmap_left;
	this->bounding_box.y=Sint16(font.ascent-glyph_slot->bitmap_top);
	this->bounding_box.w=(Uint16)width;
	this->bounding_box.h=(Uint16)height;
	this->advance=FT_CEIL(glyph_slot->metrics.horiAdvance);

	if (outline_size && !this->outline_base_bitmap){
		this->outline_bounding_box=this->bounding_box;
		size_t buffer_size=this->outline_bounding_box.w*this->outline_bounding_box.h+1;
		this->outline_base_bitmap=new uchar[buffer_size];
		memset(this->outline_base_bitmap,0,buffer_size);
	}
}

NONS_Glyph::~NONS_Glyph(){
	delete[] this->base_bitmap;
	delete[] this->outline_base_bitmap;
}

bool NONS_Glyph::needs_redraw(ulong size,bool italic,bool bold,ulong outline_size) const{
	return (this->size!=size || this->italic!=italic || this->bold!=bold || this->outline_size!=outline_size);
}

long NONS_Glyph::get_advance() const{
	return long(this->advance)+this->fc.spacing;
}

extern uchar integer_division_lookup[0x10000];

void put_glyph(const NONS_Surface &dst,int x,int y,uchar alpha,uchar *src,const NONS_LongRect &box,const NONS_Color color){
	x+=box.x;
	y+=box.y;
	int x0=0,
		y0=0;
	if (x<0){
		x0=-x;
		x=0;
	}
	if (y<0){
		y0=-y;
		y=0;
	}

	NONS_SurfaceProperties sp;
	dst.get_properties(sp);
	uchar src_rgba[4]={
		color.rgba[0],
		color.rgba[1],
		color.rgba[2],
		0
	};
	for (long src_y=y0,dst_y=y;src_y<box.h && dst_y<(long)sp.h;src_y++,dst_y++){
		uchar *pixel=sp.pixels+dst_y*sp.pitch+x*4;
		src+=x0;
		for (long src_x=x0,dst_x=x;src_x<box.w && dst_x<(long)sp.w;src_x++,dst_x++){
			uchar *dst_rgba[4]={
				pixel+sp.offsets[0],
				pixel+sp.offsets[1],
				pixel+sp.offsets[2],
				pixel+sp.offsets[3]
			};
			src_rgba[3]=(alpha==0xFF)?*src:FAST_INTEGER_MULTIPLICATION(*src,alpha);
			ulong bottom_alpha=*dst_rgba[3]=~(uchar)FAST_INTEGER_MULTIPLICATION(src_rgba[3]^0xFF,*dst_rgba[3]^0xFF);
			ulong composite=integer_division_lookup[src_rgba[3]+(bottom_alpha<<8)];
			if (composite){
#define put_glyph_APPLY_ALPHA(x) *dst_rgba[x]=(uchar)APPLY_ALPHA(src_rgba[x],*dst_rgba[x],composite)
				put_glyph_APPLY_ALPHA(0);
				put_glyph_APPLY_ALPHA(1);
				put_glyph_APPLY_ALPHA(2);
			}
			src++;
			pixel+=4;
		}
	}
}

void NONS_Glyph::put(const NONS_Surface &dst,int x,int y,uchar alpha){
	if (this->outline_base_bitmap)
		put_glyph(dst,x,y,alpha,this->outline_base_bitmap,this->outline_bounding_box,this->outline_color);
	if (this->base_bitmap)
		put_glyph(dst,x,y,alpha,this->base_bitmap,this->bounding_box,this->color);
}

void NONS_Glyph::done(){
	this->fc.done(this);
}

#ifndef _DEBUG
NONS_FontCache::NONS_FontCache(NONS_Font &f,ulong size,const NONS_Color &color,bool italic,bool bold,ulong outline_size,const NONS_Color &outline_color):font(f){
#else
NONS_FontCache::NONS_FontCache(NONS_Font &f,ulong size,const NONS_Color &color,bool italic,bool bold,ulong outline_size,const NONS_Color &outline_color,const char *file,ulong line):font(f){
	this->declared_in=file;
	this->line=line;
#endif
	this->set_color(color);
	this->set_outline_color(outline_color);
	this->reset_style(size,italic,bold,outline_size);
	this->spacing=0;
}

#ifndef _DEBUG
NONS_FontCache::NONS_FontCache(const NONS_FontCache &fc):font(fc.font){
#else
NONS_FontCache::NONS_FontCache(const NONS_FontCache &fc,const char *file,ulong line):font(fc.font){
	this->declared_in=file;
	this->line=line;
#endif
	this->set_color(fc.color);
	this->set_outline_color(fc.outline_color);
	this->reset_style(fc.size,fc.italic,fc.bold,fc.outline_size);
	this->spacing=fc.spacing;
	this->line_skip=fc.line_skip;
	this->font_line_skip=fc.font_line_skip;
	this->ascent=fc.ascent;
}

NONS_FontCache::~NONS_FontCache(){
	ulong count=0;
	for (std::map<wchar_t,NONS_Glyph *>::iterator i=this->glyphs.begin(),end=this->glyphs.end();i!=end;i++){
		count+=i->second->refCount;
		delete i->second;
	}
	for (std::set<NONS_Glyph *>::iterator i=this->garbage.begin(),end=this->garbage.end();i!=end;i++){
		count+=(*i)->refCount;
		delete *i;
	}
	if (count){
		o_stderr <<"NONS_FontCache::~NONS_FontCache(): Warning: "<<count<<" possible dangling references.\n";
#ifdef _DEBUG
		o_stderr <<"The cache was created in "<<this->declared_in<<", line "<<this->line<<"\n";
#endif
	}
}

void NONS_FontCache::set_size(ulong size){
	this->size=size;
	this->font.set_size(size);
	this->font_line_skip=this->line_skip=this->font.line_skip;
	this->ascent=this->font.ascent;
}

void NONS_FontCache::reset_style(ulong size,bool italic,bool bold,ulong outline_size){
	this->set_size(size);
	this->set_italic(italic);
	this->set_bold(bold);
	this->set_outline_size(outline_size);
}

#define INDENTATION_CHARACTER 0x2003

NONS_Glyph *NONS_FontCache::get_glyph(wchar_t c){
	if (c=='\t')
		c=INDENTATION_CHARACTER;
	NONS_CommandLineOptions::replaceArray_t &array=CLOptions.replaceArray;
	NONS_CommandLineOptions::replaceArray_t::iterator i=array.size()?array.find(c):array.end();
	if (i!=array.end())
		c=i->second;
	if (c<32)
		return 0;
	bool must_render=(this->glyphs.find(c)==this->glyphs.end());
	NONS_Glyph *&g=this->glyphs[c];
	if (!must_render && g->needs_redraw(this->size,this->italic,this->bold,this->outline_size)){
		if (!g->refCount)
			delete g;
		else
			this->garbage.insert(g);
		must_render=1;
	}
	if (must_render)
		g=new NONS_Glyph(*this,c,this->size,this->color,this->italic,this->bold,this->outline_size,this->outline_color);
	g->setColor(this->color);
	g->setOutlineColor(this->outline_color);
	g->refCount++;
	return g;
}

void NONS_FontCache::done(NONS_Glyph *g){
	if (!g)
		return;
	std::map<wchar_t,NONS_Glyph *>::iterator i=this->glyphs.find(g->get_codepoint());
	if (i!=this->glyphs.end()){
		if (i->second!=g){
			std::set<NONS_Glyph *>::iterator i2=this->garbage.find(g);
			if (i2!=this->garbage.end()){
				(*i2)->refCount--;
				if (!(*i2)->refCount){
					delete *i2;
					this->garbage.erase(i2);
				}
			}
		}else
			i->second->refCount--;
	}
	//otherwise, the glyph doesn't belong to this cache
}

NONS_Font *init_font(const std::wstring &filename){
	NONS_Font *font=new NONS_Font(filename);
	if (!font->good()){
		delete font;
		return 0;
	}
	return font;
}

NONS_DebuggingConsole::NONS_DebuggingConsole():font(0),cache(0),print_prompt(1){}

NONS_DebuggingConsole::~NONS_DebuggingConsole(){
	if (this->font)
		delete this->font;
}

ulong getGlyphWidth(NONS_FontCache *cache){
	ulong res=0;
	for (wchar_t a='A';a<='Z';a++){
		NONS_Glyph *g=cache->get_glyph(a);
		ulong w=g->get_advance();
		g->done();
		if (w>res)
			res=w;
	}
	return res;
}

void NONS_DebuggingConsole::init(){
	if (!this->font){
		const std::wstring &font=CLOptions.console_font;
		this->font=init_font(font);
		if (!this->font){
			o_stderr <<"The font \""<<font<<"\" could not be found. The debugging console will not be available.\n";
		}else{
			NONS_Color color(0xFF,0xFF,0xFF);
			this->cache=new NONS_FontCache(*this->font,15,color,0,0,0,color FONTCACHE_DEBUG_PARAMETERS);
			this->screenW=this->screenH=0;
		}
	}
	this->autocompleteVector.push_back(L"quit");
	this->autocompleteVector.push_back(L"_get");
	gScriptInterpreter->getCommandListing(this->autocompleteVector);
}

extern bool useDebugMode;

void NONS_DebuggingConsole::enter(NONS_ScreenSpace *dst){
	if (!this->font)
		return;
	NONS_Surface dstCopy;
	{
		NONS_Surface screen=dst->screen->get_screen();
		NONS_LongRect rect=screen.clip_rect();
		if (!this->screenW){
			this->characterWidth=getGlyphWidth(this->cache);
			this->characterHeight=this->font->line_skip;
			this->screenW=rect.w/this->characterWidth;
			this->screenH=rect.h/this->characterHeight;
			this->screen.resize(this->screenW*this->screenH);
			this->cursorX=this->cursorY=0;
		}
		dstCopy=screen.clone();
		screen.fill(NONS_Color::black);
		gScriptInterpreter->getSymbolListing(this->autocompleteVector);
		std::sort(this->autocompleteVector.begin(),this->autocompleteVector.end());
	}
	dst->screen->updateWholeScreen();
	std::wstring inputLine=this->partial;
	while (this->input(inputLine,dst)){
		if (!stdStrCmpCI(inputLine,L"quit")){
			gScriptInterpreter->stop();
			break;
		}
		this->output(inputLine+L"\n",dst);
		this->pastInputs.push_back(inputLine);
		if (firstcharsCI(inputLine,0,L"_get"))
			this->output(gScriptInterpreter->getValue(inputLine.substr(4))+L"\n",dst);
		else if (!stdStrCmpCI(inputLine,L"cls")){
			this->screen.clear();
			this->screen.resize(this->screenH*this->screenW);
			this->cursorX=0;
			this->cursorY=0;
		}else
			this->output(gScriptInterpreter->interpretFromConsole(inputLine)+L"\n",dst);
		inputLine.clear();
		this->print_prompt=1;
	}
	this->partial=inputLine;
	
	dst->screen->blitToScreen(dstCopy,0,0);
	dst->screen->updateWholeScreen();
}

void NONS_DebuggingConsole::output(const std::wstring &str,NONS_ScreenSpace *dst){
	ulong lastY=this->cursorY;
	for (ulong a=0;a<str.size();a++){
		switch (str[a]){
			case '\n':
				this->cursorX=0;
				this->cursorY++;
				break;
			case '\t':
				this->cursorX+=4-this->cursorX%4;
				if (this->cursorX>=this->screenW){
					this->cursorX=0;
					this->cursorY++;
				}
				break;
			default:
				this->locate(this->cursorX,this->cursorY)=str[a];
				this->cursorX++;
				if (this->cursorX>=this->screenW){
					this->cursorX=0;
					this->cursorY++;
				}
		}
		while (this->cursorY*this->screenW>=this->screen.size())
			this->screen.resize(this->screen.size()+this->screenW);
		if (dst && this->cursorY!=lastY){
			lastY=this->cursorY;
			this->redraw(dst,0,0);
		}
	}
}

void NONS_DebuggingConsole::autocomplete(std::vector<std::wstring> &dst,const std::wstring &line,std::wstring &suggestion,ulong cursor,ulong &space){
	dst.clear();
	std::wstring first,
		second,
		third=line.substr(cursor);
	ulong cutoff=find_last_not_of_id(line,cursor);
	if (cutoff==line.npos)
		cutoff=0;
	else
		cutoff++;
	space=cutoff;
	first=line.substr(0,cutoff);
	second=line.substr(cutoff,cursor-cutoff);
	for (ulong a=0;a<this->autocompleteVector.size();a++)
		if (firstcharsCI(this->autocompleteVector[a],0,second))
			dst.push_back(this->autocompleteVector[a]);
	if (!dst.size())
		return;
	ulong max;
	for (max=0;max<dst.front().size();max++){
		wchar_t c=dst.front()[max];
		bool _break=0;
		for (ulong a=0;a<dst.size() && !_break;a++)
			if (dst[a][max]!=c)
				_break=1;
		if (_break)
			break;
	}
	suggestion=dst.front().substr(0,max);
}

void NONS_DebuggingConsole::outputMatches(const std::vector<std::wstring> &matches,NONS_ScreenSpace *dst/*,long startFromLine,ulong cursor,const std::wstring &line*/){
	this->output(L"\n",dst);
	for (ulong a=0;a<matches.size();a++)
		this->output(matches[a]+L"\n",dst);
}

bool NONS_DebuggingConsole::input(std::wstring &input,NONS_ScreenSpace *dst){
	const std::wstring prompt=L"input:>";
	if (this->print_prompt){
		this->output(prompt,dst);
		print_prompt=0;
	}
	ulong cursor=input.size();
	this->redraw(dst,0,cursor,input);
	bool ret=1;
	useDebugMode=1;
	std::vector<std::wstring> inputs=this->pastInputs;
	std::wstring inputLine=input;
	inputs.push_back(inputLine);
	ulong currentlyEditing=inputs.size()-1;
	NONS_EventQueue queue;
	bool _break=0;
	long screenOffset=0;
	while (!_break){
		queue.WaitForEvent(10);
		while (!queue.empty() && !_break){
			SDL_Event event=queue.pop();
			bool refresh=0;
			switch (event.type){
				case SDL_QUIT:
					ret=0;
					_break=1;
					break;
				case SDL_KEYDOWN:
					{
						switch (event.key.keysym.sym){
							case SDLK_UP:
								if (!(event.key.keysym.mod&KMOD_CTRL)){
									if (currentlyEditing){
										inputs[currentlyEditing--]=inputLine;
										inputLine=inputs[currentlyEditing];
										cursor=inputLine.size();
										refresh=1;
									}
								}else if (this->screen.size()/this->screenW-this->screenH+screenOffset>0){
									screenOffset--;
									refresh=1;
								}
								break;
							case SDLK_DOWN:
								if (!(event.key.keysym.mod&KMOD_CTRL)){
									if (currentlyEditing<inputs.size()-1){
										inputs[currentlyEditing++]=inputLine;
										inputLine=inputs[currentlyEditing];
										cursor=inputLine.size();
										refresh=1;
									}
								}else if (screenOffset<0){
									screenOffset++;
									refresh=1;
								}
								break;
							case SDLK_LEFT:
								if (cursor){
									cursor--;
									refresh=1;
								}
								break;
							case SDLK_RIGHT:
								if (cursor<inputLine.size()){
									cursor++;
									refresh=1;
								}
								break;
							case SDLK_HOME:
								if (cursor){
									cursor=0;
									refresh=1;
								}
								break;
							case SDLK_END:
								if (cursor<inputLine.size()){
									cursor=inputLine.size();
									refresh=1;
								}
								break;
							case SDLK_RETURN:
							case SDLK_KP_ENTER:
								_break=1;
								break;
							case SDLK_PAUSE:
							case SDLK_ESCAPE:
								ret=0;
								_break=1;
								break;
							case SDLK_BACKSPACE:
								if (inputLine.size() && cursor){
									inputLine.erase(cursor-1,1);
									cursor--;
									refresh=1;
								}
								break;
							case SDLK_DELETE:
								if (cursor<inputLine.size()){
									inputLine.erase(cursor,1);
									refresh=1;
								}
								break;
							case SDLK_TAB:
								if (inputLine.size()){
									std::vector<std::wstring> matches;
									std::wstring suggestion;
									ulong space;
									this->autocomplete(matches,inputLine,suggestion,cursor,space);
									if (matches.size()==1){
										inputLine=inputLine.substr(0,space)+matches.front()+L" "+inputLine.substr(cursor);
										cursor=space+matches.front().size()+1;
										refresh=1;
									}else if (matches.size()>1){
										inputLine=inputLine.substr(0,space)+suggestion+inputLine.substr(cursor);
										cursor=space+suggestion.size();
										this->outputMatches(matches,dst);
										this->output(prompt,dst);
										refresh=1;
									}
								}
								break;
							default:
								{
									wchar_t c=event.key.keysym.unicode;
									if (c<32)
										break;
									inputLine.insert(inputLine.begin()+cursor++,c);
									refresh=1;
								}
						}
						break;
					}
				default:
					break;
			}
			if (refresh)
				this->redraw(dst,screenOffset,cursor,inputLine);
		}
	}
	useDebugMode=0;
	input=inputLine;
	return ret;
}

void NONS_DebuggingConsole::redraw(NONS_ScreenSpace *dst,long startFromLine,ulong lineHeight){
	dst->screen->get_screen().fill(NONS_Color::black);
	long startAt=this->screen.size()/this->screenW-this->screenH;
	if (this->cursorY+lineHeight>=this->screenH)
		startAt+=startFromLine+lineHeight;
	if (startAt<0)
		startAt=0;
	this->cache->set_color(NONS_Color::white);
	for (ulong y=0;y<this->screenH;y++){
		for (ulong x=0;x<this->screenW;x++){
			if (CONLOCATE(x,y+startAt)>=this->screen.size())
				continue;
			wchar_t c=this->locate(x,y+startAt);
			if (!c)
				continue;
			NONS_Glyph *g=this->cache->get_glyph(c);
			g->put(dst->screen->get_screen(),x*this->characterWidth,y*this->characterHeight);
			g->done();
		}
	}
}

void NONS_DebuggingConsole::redraw(NONS_ScreenSpace *dst,long startFromLine,ulong cursor,const std::wstring &line){
	NONS_Surface screen=dst->screen->get_screen();
	ulong lineHeight=(line.size()+this->cursorX)/this->screenW;
	this->redraw(dst,startFromLine,lineHeight);
	long cursorX=this->cursorX,
		cursorY=this->cursorY;
	if (cursorY+lineHeight>=this->screenH)
		cursorY=this->screenH-1-lineHeight-startFromLine;
	//cursorY-=startFromLine;
	if (cursor<line.size()){
		for (ulong a=0;a<line.size();a++){
			if (cursorY<(long)this->screenH){
				if (a==cursor){
					NONS_LongRect rect(
						cursorX*this->characterWidth,
						cursorY*this->characterHeight,
						this->characterWidth,
						this->characterHeight
					);
					screen.fill(rect,NONS_Color::white);
				}
				this->cache->set_color((a!=cursor)?NONS_Color::white:NONS_Color::black);
				NONS_Glyph *g=this->cache->get_glyph(line[a]);
				g->put(screen,cursorX*this->characterWidth,cursorY*this->characterHeight);
				g->done();
			}
			cursorX++;
			if (cursorX>=(long)this->screenW){
				cursorX=0;
				cursorY++;
			}
		}
	}else{
		this->cache->set_color(NONS_Color::white);
		for (ulong a=0;a<line.size();a++){
			if (cursorY<(long)this->screenH){
				NONS_Glyph *g=this->cache->get_glyph(line[a]);
				g->put(screen,cursorX*this->characterWidth,cursorY*this->characterHeight);
				g->done();
			}
			cursorX++;
			if (cursorX>=(long)this->screenW){
				cursorX=0;
				cursorY++;
			}
		}
		NONS_LongRect rect(
			cursorX*this->characterWidth,
			cursorY*this->characterHeight,
			this->characterWidth,
			this->characterHeight
		);
		screen.fill(rect,NONS_Color::white);
	}
	screen.unbind();
	dst->screen->updateWholeScreen();
}
