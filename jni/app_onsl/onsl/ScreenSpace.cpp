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

#include "ScreenSpace.h"
#include "IOFunctions.h"
#include "Options.h"
#include "ScriptInterpreter.h"
#include <cassert>
#include <iostream>

NONS_Layer::NONS_Layer(const NONS_LongRect &size,const NONS_Color &rgba)
		:data((ulong)size.w,(ulong)size.h),defaultShade(rgba){
	this->data.fill(rgba);
	this->fontCache=0;
	this->visible=1;
	this->useDataAsDefaultShade=0;
	this->alpha=0xFF;
	this->clip_rect=size;
}

NONS_Layer::NONS_Layer(const NONS_Surface &img,const NONS_Color &rgba)
		:data(img),defaultShade(rgba){
	this->fontCache=0;
	this->visible=1;
	this->useDataAsDefaultShade=0;
	this->alpha=0xFF;
	this->clip_rect=this->data.clip_rect();
	NONS_SurfaceProperties sp;
	img.get_properties(sp);
	this->animation.animation_length=sp.frames;
	if (this->animation.animation_length>1)
		this->animation.frame_ends.push_back(1);
}

NONS_Layer::NONS_Layer(const std::wstring *string){
	this->fontCache=0;
	this->visible=1;
	this->useDataAsDefaultShade=0;
	this->alpha=0xFF;
	this->load(string);
	this->position.x=0;
	this->position.y=0;
	this->position.w=0;
	this->position.h=0;
}

NONS_Layer::~NONS_Layer(){
	this->unload();
	if (this->fontCache)
		delete this->fontCache;
}

void NONS_Layer::MakeTextLayer(NONS_FontCache &fc,const NONS_Color &foreground){
	this->fontCache=new NONS_FontCache(fc FONTCACHE_DEBUG_PARAMETERS);
	this->fontCache->set_color(foreground);
	this->defaultShade=NONS_Color::black_transparent;
	this->data.fill(this->defaultShade);
}

void NONS_Layer::Clear(){
	if (!this->useDataAsDefaultShade){
		this->load((const std::wstring *)0);
		this->data.fill(this->defaultShade);
	}
}

void NONS_Layer::setShade(const NONS_Color &color){
	if (!this->data)
		this->data.assign((ulong)this->clip_rect.w,(ulong)this->clip_rect.h);
	this->defaultShade=color;
}

void NONS_Layer::usePicAsDefaultShade(const NONS_Surface &s){
	this->data=s;
	this->useDataAsDefaultShade=1;
}

bool NONS_Layer::load(const std::wstring *string){
	if (!string){
		NONS_LongRect lr=this->data.clip_rect();
		this->unload();
		this->data.assign(lr.w,lr.h);
		this->clip_rect=NONS_Rect(lr);
		return 1;
	}
	this->unload();
	NONS_Surface temp=*string;
	if (!temp){
		this->data.fill(this->defaultShade);
		this->clip_rect=this->data.clip_rect();
		return 0;
	}
	temp.get_optimized_updates(this->optimized_updates);
	this->data=temp;
	this->animation.parse(*string);
	this->clip_rect=this->data.clip_rect();
	/*if (this->animation.animation_length>1){
		ulong t0=clock.get();
		NONS_LongRect rect=this->getUpdateRect(0,1);
		ulong t1=clock.get();
		STD_COUT <<"completed in "<<t1-t0<<" msec."<<std::endl;
	}*/
	return 1;
}

bool NONS_Layer::load(const NONS_Surface &src){
	this->unload();
	this->data=src.clone();
	return this->data;
}

void NONS_Layer::unload(){
	assert(this!=0);
	if (!this->data)
		return;

	this->data.unbind();
	this->optimized_updates.clear();
}

bool NONS_Layer::advanceAnimation(ulong msec){
	return this->animation.advanceAnimation(msec)>=0;
}

void NONS_Layer::centerAround(int x){
	this->position.x=x-this->clip_rect.w/2;
}

void NONS_Layer::useBaseline(int y){
	this->position.y=y-this->clip_rect.h+1;
}

TiXmlElement *NONS_Layer::save(const char *override_name){
	TiXmlElement *layer=new TiXmlElement(override_name?override_name:"layer");
	layer->SetAttribute("string",this->animation.getString());
	layer->LinkEndChild(this->defaultShade.save("default_shade"));
	layer->LinkEndChild(this->clip_rect.save("clip_rect"));
	layer->LinkEndChild(this->position.save("position"));
	layer->SetAttribute("visible",this->visible);
	layer->SetAttribute("alpha",this->alpha);
	layer->SetAttribute("data_as_def_shade",this->useDataAsDefaultShade);
	return layer;
}

NONS_Layer::NONS_Layer(TiXmlElement *parent,const char *name){
	TiXmlElement *layer;
	if (!name || *name)
		layer=parent->FirstChildElement(name?name:"layer");
	else
		layer=parent;
	std::wstring string=layer->QueryWStringAttribute("string");
	this->defaultShade=NONS_Color(layer->FirstChildElement("default_shade"));
	this->clip_rect=NONS_LongRect(layer->FirstChildElement("clip_rect"));
	this->position=NONS_LongRect(layer->FirstChildElement("position"));
	this->visible=layer->QueryIntAttribute("visible");
	this->alpha=layer->QueryIntAttribute("alpha");
	if (!string.size()){
		this->setShade(this->defaultShade);
		this->Clear();
	}else
		this->load(&string);
	this->useDataAsDefaultShade=layer->QueryIntAttribute("data_as_def_shade");
	this->fontCache=0;
}

std::ofstream textDumpFile;

NONS_StandardOutput::NONS_StandardOutput(NONS_Layer *fgLayer,NONS_Layer *shadowLayer,NONS_Layer *shadeLayer){
	this->foregroundLayer=fgLayer;
	this->shadowLayer=shadowLayer;
	this->shadeLayer=shadeLayer;
	this->x=0;
	this->y=0;
	this->x0=0;
	this->y0=0;
	NONS_SurfaceProperties sp;
	fgLayer->data.get_properties(sp);
	this->w=sp.w;
	this->h=sp.h;
	this->display_speed=0;
	this->extraAdvance=0;
	this->visible=0;
	this->text_visible=1;
	this->transition=new NONS_GFX(1,0,0);
	this->log.reserve(50);
	this->horizontalCenterPolicy=0;
	this->verticalCenterPolicy=0;
	this->lastStart=-1;
	this->printingStarted=0;
	this->shadowPosX=this->shadowPosY=1;
	this->indentationLevel=0;
	this->maxLogPages=-1;
}

NONS_StandardOutput::NONS_StandardOutput(NONS_FontCache &fc,const NONS_LongRect &size,const NONS_LongRect &frame,bool shadow){
	this->foregroundLayer=new NONS_Layer(size,NONS_Color::black);
	this->foregroundLayer->MakeTextLayer(fc,NONS_Color::white);
	if (shadow){
		this->shadowLayer=new NONS_Layer(size,NONS_Color::black);
		this->shadowLayer->MakeTextLayer(fc,NONS_Color::black);
	}else
		this->shadowLayer=0;
	this->shadeLayer=new NONS_Layer(size,NONS_Color(0x99,0x99,0x99));
	this->x=frame.x;
	this->y=frame.y;
	this->x0=frame.x;
	this->y0=frame.y;
	this->w=frame.w;
	this->h=frame.h;
	this->display_speed=0;
	this->extraAdvance=0;
	this->visible=0;
	this->text_visible=1;
	this->transition=new NONS_GFX(1,0,0);
	this->log.reserve(50);
	this->horizontalCenterPolicy=0;
	this->verticalCenterPolicy=0;
	this->lastStart=-1;
	this->printingStarted=0;
	this->shadowPosX=this->shadowPosY=1;
	this->indentationLevel=0;
	this->maxLogPages=-1;
}

NONS_StandardOutput::~NONS_StandardOutput(){
	this->Clear();
	delete this->foregroundLayer;
	if (this->shadowLayer)
		delete this->shadowLayer;
	delete this->shadeLayer;
	if (!this->transition->stored)
		delete this->transition;
}

#define INDENTATION_CHARACTER 0x2003

ulong NONS_StandardOutput::getIndentationSize(){
	NONS_Glyph *glyph=this->foregroundLayer->fontCache->get_glyph(INDENTATION_CHARACTER);
	long advance=glyph->get_advance();
	glyph->done();
	return this->indentationLevel*(advance+this->extraAdvance);
}

bool NONS_StandardOutput::prepareForPrinting(std::wstring str){
	long lastSpace=-1;
	int x0=this->x,y0=this->y;
	int wordL=0;
	int lineSkip=this->foregroundLayer->fontCache->line_skip;
	this->resumePrinting=0;
	bool check_at_end=1;
	ulong indentationMargin=this->x0+this->getIndentationSize();
	for (ulong a=0;a<str.size();a++){
		wchar_t character=str[a];
		NONS_Glyph *glyph=this->foregroundLayer->fontCache->get_glyph(character);
		if (character=='\n'){
			this->cachedText.push_back(character);
			if (x0+wordL>=this->w+this->x0 && lastSpace>=0){
				this->cachedText[lastSpace]='\r';
				y0+=lineSkip;
			}
			lastSpace=-1;
			x0=this->x0;
			y0+=lineSkip;
			wordL=0;
		}else if (isbreakspace(character)){
			if (x0+wordL>this->w+this->x0 && lastSpace>=0){
				this->cachedText[lastSpace]='\r';
				lastSpace=-1;
				x0=indentationMargin;
				y0+=lineSkip;
			}
			x0+=wordL;
			lastSpace=this->cachedText.size();
			wordL=glyph->get_advance()+this->extraAdvance;
			this->cachedText.push_back(character);
		}else if (character){
			wordL+=glyph->get_advance()+this->extraAdvance;
			this->cachedText.push_back(character);
		}else{
			if (x0+wordL>=this->w+this->x0 && lastSpace>=0)
				this->cachedText[lastSpace]='\r';
			check_at_end=0;
			glyph->done();
			break;
		}
		CHECK_POINTER_AND_CALL(glyph,done());
	}
	if (check_at_end && x0+wordL>=this->w+this->x0 && lastSpace>=0)
		this->cachedText[lastSpace]='\r';
	this->printingStarted=1;
	this->indent_next=0;
	if (this->verticalCenterPolicy>0 && this->currentBuffer.size()>0)
		return 1;
	NONS_LongRect frame(this->x0,this->y0,this->w,this->h);
	if (this->verticalCenterPolicy)
		this->y=this->setTextStart(&this->cachedText,&frame,this->verticalCenterPolicy);
	else if (!this->currentBuffer.size())
		this->y=this->y0;
	this->prebufferedText.append(L"<y=");
	this->prebufferedText.append(itoaw(this->y));
	this->prebufferedText.push_back('>');
	this->set_italic(this->get_italic());
	this->set_bold(this->get_bold());
	return 0;
}

void NONS_StandardOutput::set_italic(bool i){
	this->foregroundLayer->fontCache->set_italic(i);
	if (this->shadowLayer)
		this->shadowLayer->fontCache->set_italic(i);
	if (i)
		this->prebufferedText.append(L"<i>");
	else
		this->prebufferedText.append(L"<!i>");
}

void NONS_StandardOutput::set_bold(bool b){
	this->foregroundLayer->fontCache->set_bold(b);
	if (this->shadowLayer)
		this->shadowLayer->fontCache->set_bold(b);
	if (b)
		this->prebufferedText.append(L"<b>");
	else
		this->prebufferedText.append(L"<!b>");
}

void NONS_StandardOutput::set_size(ulong size){
	this->foregroundLayer->fontCache->set_size(size);
	if (this->shadowLayer)
		this->shadowLayer->fontCache->set_size(size);
}

bool NONS_StandardOutput::print(ulong start,ulong end,NONS_VirtualScreen *dst,ulong *printedChars){
	if (start>=this->cachedText.size())
		return 0;
	NONS_EventQueue queue;
	bool enterPressed=0;
	int x0,
		y0=this->y;
	NONS_LongRect frame(this->x0,this->y0,this->w,this->h);
	ulong indentationMargin=this->x0+this->getIndentationSize();
	if (this->x==this->x0){
		x0=this->setLineStart(&this->cachedText,start,&frame,this->horizontalCenterPolicy);
		if (this->indent_next && !this->horizontalCenterPolicy)
			x0=indentationMargin;
		if (x0!=this->lastStart){
			this->prebufferedText.append(L"<x=");
			this->prebufferedText.append(itoaw(x0));
			this->prebufferedText.push_back('>');
			this->lastStart=x0;
		}
	}else
		x0=this->x;
	y0=this->y;
	int lineSkip=this->foregroundLayer->fontCache->line_skip;
	NONS_Clock clock;
	NONS_Clock::t t0,t1;
	if (this->resumePrinting)
		start=this->resumePrintingWhere;
	for (ulong a=start;a<end && a<this->cachedText.size();a++){
		t0=clock.get();
		wchar_t character=this->cachedText[a];
		NONS_Glyph *glyph=this->foregroundLayer->fontCache->get_glyph(character);
		NONS_Glyph *glyph2=(this->shadowLayer)?this->shadowLayer->fontCache->get_glyph(character):0;
		if (!glyph){
			if (y0+lineSkip>=this->h+this->y0){
				this->resumePrinting=1;
				this->x=x0;
				this->y=y0;
				this->resumePrinting=1;
				this->resumePrintingWhere=a+1;
				this->currentBuffer.append(this->prebufferedText);
				this->prebufferedText.clear();
				this->indent_next=(character=='\r');
				return 1;
			}
			if (a<this->cachedText.size()-1){
				x0=this->setLineStart(&this->cachedText,a+1,&frame,this->horizontalCenterPolicy);
				if (character=='\r' && !this->horizontalCenterPolicy)
					x0=indentationMargin;
				if (x0!=this->lastStart){
					this->prebufferedText.append(L"<x=");
					this->prebufferedText.append(itoaw(x0));
					this->prebufferedText.push_back('>');
					this->lastStart=x0;
				}
			}else
				x0=this->x0;
			y0+=lineSkip;
			this->prebufferedText.push_back('\n');
			continue;
		}
		int advance=glyph->get_advance()+this->extraAdvance;
		if (x0+advance>this->w+this->x0){
			if (y0+lineSkip>=this->h+this->y0){
				this->resumePrinting=1;
				this->x=x0;
				this->y=y0;
				this->resumePrinting=1;
				this->resumePrintingWhere=isbreakspace(glyph->get_codepoint())?a+1:a;
				this->currentBuffer.append(this->prebufferedText);
				this->prebufferedText.clear();
				this->indent_next=1;
				glyph->done();
				CHECK_POINTER_AND_CALL(glyph2,done());
				return 1;
			}else{
				x0=this->setLineStart(&this->cachedText,a,&frame,this->horizontalCenterPolicy);
				if (!this->horizontalCenterPolicy)
					x0=indentationMargin;
				if (x0!=this->lastStart){
					this->prebufferedText.append(L"<x=");
					this->prebufferedText.append(itoaw(x0));
					this->prebufferedText.push_back('>');
					this->lastStart=x0;
				}
				y0+=lineSkip;
				this->prebufferedText.push_back('\n');
			}
		}
		switch (glyph->get_codepoint()){
			case '\\':
				this->prebufferedText.append(L"\\\\");
				break;
			case '<':
				this->prebufferedText.append(L"\\<");
				break;
			case '>':
				this->prebufferedText.append(L"\\>");
				break;
			default:
				this->prebufferedText.push_back(glyph->get_codepoint());
		}
		{
			NONS_Surface screen=dst->get_screen();
			if (glyph2){
				glyph2->put(
					this->shadowLayer->data,
					x0+this->shadowPosX-(int)this->shadowLayer->clip_rect.x,
					y0+this->shadowPosY-(int)this->shadowLayer->clip_rect.y
				);
				glyph2->put(
					screen,
					x0+this->shadowPosX,
					y0+this->shadowPosY
				);
				glyph2->done();
			}
			glyph->put(
				this->foregroundLayer->data,
				x0-(int)this->foregroundLayer->clip_rect.x,
				y0-(int)this->foregroundLayer->clip_rect.y
			);
			glyph->put(screen,x0,y0);
			glyph->done();
		}
		NONS_LongRect r=glyph->get_put_bounding_box((Sint16)x0,(Sint16)y0);
		if (glyph2){
			long tempX=(this->shadowPosX<=0)?0:this->shadowPosX,
				tempY=(this->shadowPosY<=0)?0:this->shadowPosY;
			r.w+=(Uint16)tempX;
			r.h+=(Uint16)tempY;
		}
		dst->updateScreen(r.x,r.y,r.w,r.h);
		if (printedChars)
			(*printedChars)++;
		x0+=advance;
		while (!CURRENTLYSKIPPING && !enterPressed && !queue.empty()){
			SDL_Event event=queue.pop();
			if (event.type==SDL_KEYDOWN && (event.key.keysym.sym==SDLK_RETURN || event.key.keysym.sym==SDLK_SPACE))
				enterPressed=1;
		}
		t1=clock.get();
		if (!CURRENTLYSKIPPING && !enterPressed && this->display_speed>ulong(t1-t0))
			SDL_Delay(this->display_speed-long(t1-t0));
	}
	this->x=x0;
	this->y=y0;
	this->resumePrinting=0;
	this->resumePrintingWhere=0;
	return 0;
}

void NONS_StandardOutput::endPrinting(){
	if (this->printingStarted)
		this->currentBuffer.append(this->prebufferedText);
	this->prebufferedText.clear();
	this->cachedText.clear();
	this->printingStarted=0;
}

void NONS_StandardOutput::ephemeralOut(const std::wstring &str,const NONS_Surface &dst,bool writeToLayers,const NONS_Color *col){
	int x=this->x0,
		y=this->y0;
	int lineSkip=this->foregroundLayer->fontCache->line_skip;
	if (writeToLayers){
		this->foregroundLayer->Clear();
		if (this->shadowLayer)
			this->shadowLayer->Clear();
	}
	long lastStart=this->x0;
	NONS_FontCache cache(*this->foregroundLayer->fontCache FONTCACHE_DEBUG_PARAMETERS),
		*shadow=(this->shadowLayer)?new NONS_FontCache(*this->shadowLayer->fontCache FONTCACHE_DEBUG_PARAMETERS):0;
	cache.set_to_normal();
	if (shadow)
		shadow->set_to_normal();
	if (col)
		cache.set_color(*col);
	for (ulong a=0;a<str.size();a++){
		wchar_t character=str[a];
		if (character=='<'){
			std::wstring tagname=tag_name(str,a);
			if (tagname.size()){
				if (tagname==L"x"){
					std::wstring tagvalue=tag_value(str,a);
					if (tagvalue.size())
						lastStart=x=atol(tagvalue);
				}else if (tagname==L"y"){
					std::wstring tagvalue=tag_value(str,a);
					if (tagvalue.size())
						y=atol(tagvalue);
				}else if (tagname==L"i"){
					cache.set_italic(1);
					if (shadow)
						shadow->set_italic(1);
				}else if (tagname==L"!i"){
					cache.set_italic(0);
					if (shadow)
						shadow->set_italic(0);
				}else if (tagname==L"b"){
					cache.set_bold(1);
					if (shadow)
						shadow->set_bold(1);
				}else if (tagname==L"!b"){
					cache.set_bold(0);
					if (shadow)
						shadow->set_bold(0);
				}
				a=str.find('>',a);
			}
			continue;
		}
		if (character=='\\')
			character=str[++a];
		NONS_Glyph *glyph=cache.get_glyph(character);
		NONS_Glyph *glyph2=(this->shadowLayer)?shadow->get_glyph(character):0;
		if (character=='\n'){
			x=lastStart;
			y+=lineSkip;
		}else{
			if (writeToLayers){
				CHECK_POINTER_AND_CALL(glyph2,put(this->shadowLayer->data,x+1,y+1));
				glyph->put(this->foregroundLayer->data,x,y);
			}
			if (dst){
				CHECK_POINTER_AND_CALL(glyph2,put(dst,x+1,y+1));
				glyph->put(dst,x,y);
			}
			x+=glyph->get_advance();
			glyph->done();
			CHECK_POINTER_AND_CALL(glyph2,done());
		}
	}
	if (shadow)
		delete shadow;
}

void NONS_StandardOutput::ephemeralOut(const std::wstring &str,NONS_VirtualScreen *dst,bool update,bool writeToLayers,const NONS_Color *col){
	if (dst){
		this->ephemeralOut(str,dst->get_screen(),writeToLayers,col);
		if (update)
			dst->updateWholeScreen();
	}else
		this->ephemeralOut(str,NONS_Surface(),writeToLayers,col);
}

int NONS_StandardOutput::setLineStart(std::wstring *arr,ulong start,NONS_LongRect *frame,float center){
	while (start<arr->size() && !(*arr)[start])
		start++;
	int width=this->predictLineLength(arr,start,frame->w);
	float factor=(center<=0.5f)?center:1.0f-center;
	int pixelcenter=int(float(frame->w)*factor);
	//Magic formula. Don't mess with it.
	return int((width/2.0f>pixelcenter)?frame->x+(frame->w-width)*(center>0.5f):frame->x+frame->w*center-width/2.0f);
}

int NONS_StandardOutput::predictLineLength(std::wstring *arr,long start,int width){
	int res=0;
	for (ulong a=start;a<arr->size() && (*arr)[a];a++){
		NONS_Glyph *glyph=this->foregroundLayer->fontCache->get_glyph((*arr)[a]);
		if (!glyph || res+glyph->get_advance()+this->extraAdvance>=width){
			CHECK_POINTER_AND_CALL(glyph,done());
			break;
		}
		res+=glyph->get_advance()+this->extraAdvance;
		glyph->done();
	}
	return res;
}

int NONS_StandardOutput::predictTextHeight(std::wstring *arr){
	int lines=1;
	for (ulong a=0;a<arr->size() && (*arr)[a];a++){
		wchar_t char0=(*arr)[a];
		if (char0==10 || char0==13)
			lines++;
	}
	if (!arr->size() || (*arr)[arr->size()-1]==13 || (*arr)[arr->size()-1]==10)
		lines--;
	return this->foregroundLayer->fontCache->line_skip*lines;
}

int NONS_StandardOutput::setTextStart(std::wstring *arr,NONS_LongRect *frame,float center){
	int height=this->predictTextHeight(arr);
	float factor=(center<=0.5f)?center:1.0f-center;
	int pixelcenter=int(float(frame->h)*factor);
	return int((height/2.0f>pixelcenter)?frame->y+(frame->h-height)*(center>0.5f):frame->y+frame->h*center-height/2.0f);
}

void NONS_StandardOutput::Clear(bool eraseBuffer){
	this->foregroundLayer->Clear();
	if (this->shadowLayer)
		this->shadowLayer->Clear();
	this->x=this->x0;
	this->y=this->y0;
	if (eraseBuffer){
		if (this->printingStarted){
			this->currentBuffer.append(this->prebufferedText);
			this->prebufferedText.clear();
		}
		if (this->currentBuffer.size()>0){
			if (textDumpFile.is_open()){
				textDumpFile <<UniToUTF8(remove_tags(this->currentBuffer))<<std::endl;
				textDumpFile.flush();
			}
			if (this->maxLogPages){
				this->log.push_back(this->currentBuffer);
				if (this->maxLogPages>0 && this->log.size()>(ulong)this->maxLogPages && !CLOptions.never_clear_log)
					this->log.erase(this->log.begin());
			}
			this->currentBuffer.clear();
		}
	}
	if (this->verticalCenterPolicy>0 && this->cachedText.size()){
		NONS_LongRect frame(this->x0,this->y0,this->w,this->h);
		this->y=this->setTextStart(&this->cachedText,&frame,this->verticalCenterPolicy);
		this->prebufferedText.append(L"<y=");
		this->prebufferedText.append(itoaw(this->y));
		this->prebufferedText.push_back('>');
	}
}

void NONS_StandardOutput::setPosition(int x,int y){
	this->x=this->x0+x;
	this->y=this->y0+y;
	this->currentBuffer.append(L"<x=");
	this->currentBuffer.append(itoaw(this->x));
	this->currentBuffer.append(L"><y=");
	this->currentBuffer.append(itoaw(this->y));
	this->currentBuffer.push_back('>');
}

float NONS_StandardOutput::getCenterPolicy(char which){
	which=tolower(which);
	return (which=='v')?this->verticalCenterPolicy:this->horizontalCenterPolicy;
}

void NONS_StandardOutput::setCenterPolicy(char which,float val){
	which=tolower(which);
	if (val<0)
		val=-val;
	if (val>1){
		ulong val2=(ulong)val;
		val-=val2;
	}
	if (which=='v')
		this->verticalCenterPolicy=val;
	else
		this->horizontalCenterPolicy=val;
}

void NONS_StandardOutput::setCenterPolicy(char which,long val){
	this->setCenterPolicy(which,float(val)/100);
}

bool NONS_StandardOutput::NewLine(){
	int skip=this->foregroundLayer->fontCache->line_skip;
	if (this->y+skip>=this->y0+this->h)
		return 1;
	this->y+=skip;
	this->x=this->x0;
	if (this->printingStarted)
		this->prebufferedText.append(L"\n");
	else
		this->currentBuffer.append(L"\n");
	return 0;
}

TiXmlElement *NONS_StandardOutput::save(const interpreter_stored_state &state,const char *override_name){
	TiXmlElement *output=new TiXmlElement(override_name?override_name:"window");
	output->LinkEndChild(this->shadeLayer->clip_rect.save("window_rect"));
	output->LinkEndChild(NONS_LongRect(this->x0,this->y0,this->w,this->h).save("frame_rect"));
	output->LinkEndChild(this->shadeLayer->defaultShade.save("window_shade"));
	output->LinkEndChild(this->transition->save("transition_effect"));
	{
		TiXmlElement *font=new TiXmlElement("font");
		output->LinkEndChild(font);
		font->SetAttribute("size",this->foregroundLayer->fontCache->get_size());
		font->LinkEndChild(this->foregroundLayer->fontCache->get_color().save());
		font->SetAttribute("bold",state.bold);
		font->SetAttribute("italic",state.italic);
		font->SetAttribute("spacing",this->foregroundLayer->fontCache->spacing);
		font->SetAttribute("line_skip",this->foregroundLayer->fontCache->line_skip);
	}
	output->SetAttribute("text_speed",this->display_speed);
	output->SetAttribute("shadow",(int)!!this->shadeLayer);
	output->SetAttribute("shadow_x",(int)!!this->shadowPosX);
	output->SetAttribute("shadow_y",(int)!!this->shadowPosY);
	{
		TiXmlElement *log=new TiXmlElement("log");
		output->LinkEndChild(log);
		for (ulong a=0;a<this->log.size();a++){
			TiXmlElement *page=new TiXmlElement("page");
			log->LinkEndChild(page);
			page->SetAttribute("data",this->log[a]);
		}
		log->SetAttribute("current_buffer",this->currentBuffer);
	}
	output->SetAttribute("indentation",this->indentationLevel);
	output->SetAttribute("text_position_x",state.textX);
	output->SetAttribute("text_position_y",state.textY);
	output->SetAttribute("visible",this->visible);
	output->SetAttribute("text_visible",this->text_visible);
	output->SetAttribute("extra_advance",this->extraAdvance);
	output->SetAttribute("center_x",itoac(this->horizontalCenterPolicy));
	output->SetAttribute("center_y",itoac(this->verticalCenterPolicy));
	output->SetAttribute("max_log_pages",this->maxLogPages);
	return output;
}

NONS_StandardOutput::NONS_StandardOutput(TiXmlElement *parent,NONS_FontCache &fc,const char *name){
	TiXmlElement *output=parent->FirstChildElement(name?name:"window");
	TiXmlElement *font=output->FirstChildElement("font");
	TiXmlElement *log=output->FirstChildElement("log");
	NONS_LongRect window(output->FirstChildElement("window_rect"));
	NONS_LongRect frame(output->FirstChildElement("frame_rect"));
	this->shadeLayer=new NONS_Layer(window,NONS_Color(output->FirstChildElement("window_shade")));
	this->transition=new NONS_GFX(output,"transition_effect");
	fc.set_size(font->QueryIntAttribute("size"));
	fc.spacing=font->QueryIntAttribute("spacing");
	fc.line_skip=font->QueryIntAttribute("line_skip");
	this->foregroundLayer=new NONS_Layer(window,NONS_Color::black);
	this->foregroundLayer->MakeTextLayer(fc,NONS_Color(font->FirstChildElement("color")));
	if (font->QueryIntAttribute("shadow")){
		this->shadowLayer=new NONS_Layer(window,NONS_Color::black);
		this->shadowLayer->MakeTextLayer(fc,NONS_Color::black);
	}else
		this->shadowLayer=0;
	this->set_bold(font->QueryIntAttribute("bold"));
	this->set_italic(font->QueryIntAttribute("italic"));
	this->x0=frame.x;
	this->y0=frame.y;
	this->w=frame.w;
	this->h=frame.h;
	this->display_speed=output->QueryIntAttribute("text_speed");
	this->shadowPosX=output->QueryIntAttribute("shadow_x");
	this->shadowPosY=output->QueryIntAttribute("shadow_y");
	this->log.reserve(50);
	for (TiXmlElement *i=log->FirstChildElement();i;i=i->NextSiblingElement())
		this->log.push_back(i->QueryWStringAttribute("data"));
	this->currentBuffer=log->QueryWStringAttribute("current_buffer");
	this->ephemeralOut(this->currentBuffer,0,0,1,0);
	this->indentationLevel=output->QueryIntAttribute("indentation");
	this->x=output->QueryIntAttribute("text_position_x");
	this->y=output->QueryIntAttribute("text_position_y");

	this->visible=output->QueryIntAttribute("visible");
	this->text_visible=output->QueryIntAttribute("text_visible");
	this->extraAdvance=output->QueryIntAttribute("extra_advance");
	this->horizontalCenterPolicy=output->QueryFloatAttribute("center_x");
	this->verticalCenterPolicy=output->QueryFloatAttribute("center_y");
	this->maxLogPages=output->QueryIntAttribute("max_log_pages");

	this->printingStarted=0;
	this->lastStart=-1;
}

void NONS_ScreenSpace::alloc_screen(){
	int w,h;
	if (settings.resolution.set){
		w=settings.resolution.data.w;
		h=settings.resolution.data.h;
	}else{
		w=CLOptions.virtualWidth;
		h=CLOptions.virtualHeight;
		if (CLOptions.resolution_set){
			settings.resolution.set=1;
			settings.resolution.data.w=w;
			settings.resolution.data.h=h;
		}
	}
	this->screen=new NONS_VirtualScreen(CLOptions.virtualWidth,CLOptions.virtualHeight,w,h);
}

NONS_ScreenSpace::NONS_ScreenSpace(int framesize,NONS_FontCache &fc){
	this->alloc_screen();
	NONS_LongRect size(0,0,CLOptions.virtualWidth,CLOptions.virtualHeight);
	if (!CLOptions.play.size()){
		this->output=new NONS_StandardOutput(
			fc,size,
			NONS_LongRect(
				framesize,framesize,
				CLOptions.virtualWidth-framesize*2,
				CLOptions.virtualHeight-framesize*2
			)
		);
		this->output->visible=0;
	}else{
		this->output=0;
		this->lookback=0;
	}
	this->layerStack.resize(1000,0);
	this->Background=new NONS_Layer(size,NONS_Color::black);
	this->leftChar=0;
	this->centerChar=0;
	this->rightChar=0;
	this->char_baseline=(ulong)this->screen->inRect.h-1;
	this->screenBuffer.assign((ulong)this->screen->inRect.w,(ulong)this->screen->inRect.h);
	this->gfx_store=new NONS_GFXstore();
	this->sprite_priority=25;
	if (!CLOptions.play.size()){
		assert(!!this->output);
		this->lookback=new NONS_Lookback(this->output,this->output->foregroundLayer->fontCache->get_color());
	}
	this->cursor=0;
	this->blendSprites=1;

	this->characters[0]=&this->leftChar;
	this->characters[1]=&this->centerChar;
	this->characters[2]=&this->rightChar;
	this->charactersBlendOrder.push_back(0);
	this->charactersBlendOrder.push_back(1);
	this->charactersBlendOrder.push_back(2);
	this->apply_monochrome_first=0;
}

NONS_ScreenSpace::NONS_ScreenSpace(const NONS_LongRect &window,const NONS_LongRect &frame,NONS_FontCache &fc,bool shadow){
	this->alloc_screen();
	if (!CLOptions.play.size()){
		this->output=new NONS_StandardOutput(fc,window,frame);
		this->output->visible=0;
	}else{
		this->output=0;
		this->lookback=0;
	}
	this->layerStack.resize(1000,0);
	this->Background=new NONS_Layer(
		NONS_LongRect(0,0,CLOptions.virtualWidth,CLOptions.virtualHeight),
		NONS_Color::black
	);
	this->leftChar=0;
	this->centerChar=0;
	this->rightChar=0;
	this->char_baseline=(ulong)this->screen->inRect.h-1;
	this->screenBuffer.assign((ulong)this->screen->inRect.w,(ulong)this->screen->inRect.h);
	this->gfx_store=new NONS_GFXstore;
	this->sprite_priority=25;
	if (!CLOptions.play.size()){
		assert(!!this->output);
		this->lookback=new NONS_Lookback(this->output,this->output->foregroundLayer->fontCache->get_color());
	}
	this->cursor=0;
	this->blendSprites=1;

	this->characters[0]=&this->leftChar;
	this->characters[1]=&this->centerChar;
	this->characters[2]=&this->rightChar;
	this->charactersBlendOrder.push_back(0);
	this->charactersBlendOrder.push_back(1);
	this->charactersBlendOrder.push_back(2);
	this->apply_monochrome_first=0;
}

NONS_ScreenSpace::~NONS_ScreenSpace(){
	delete this->output;
	for (ulong a=0;a<this->layerStack.size();a++)
		if (this->layerStack[a])
			delete this->layerStack[a];
	delete this->leftChar;
	delete this->rightChar;
	delete this->centerChar;
	delete this->Background;
	delete this->screen;
	delete this->gfx_store;
	delete this->lookback;
}

template <typename T>
void blend_optimized(const NONS_Layer *p,const NONS_LongRect &refresh_area,NONS_Surface &screenBuffer,T function){
	if (!p || !p->data || !p->visible)
		return;
	NONS_LongRect src(
		refresh_area.x-p->position.x+p->clip_rect.x,
		refresh_area.y-p->position.y+p->clip_rect.y,
		std::max(refresh_area.w,(long)p->clip_rect.w),
		std::max(refresh_area.h,(long)p->clip_rect.h)
	);
	if (src.x<p->clip_rect.x)
		src.x=p->clip_rect.x;
	if (src.y<p->clip_rect.y)
		src.y=p->clip_rect.y;
	NONS_LongRect dst=refresh_area;
	if (dst.x<p->position.x)
		dst.x=p->position.x;
	if (dst.y<p->position.y)
		dst.y=p->position.y;
	(screenBuffer.*function)(p->data,p->get_frame(),&dst,&src);
}

void NONS_ScreenSpace::BlendOptimized(std::vector<NONS_LongRect> &rects){
	if (!rects.size())
		return;
////////////////////////////////////////////////////////////////////////////////
#define BLEND_OPTIM(p,function) blend_optimized((p),refresh_area,this->screenBuffer,&NONS_Surface::function##_frame)
////////////////////////////////////////////////////////////////////////////////
	ulong minx=rects[0].x,
		maxx=minx+rects[0].w,
		miny=rects[0].y,
		maxy=miny+rects[0].h;
	for (ulong a=1;a<rects.size();a++){
		ulong x0=rects[a].x,
			x1=x0+rects[a].w,
			y0=rects[a].y,
			y1=y0+rects[a].h;
		if (x0<minx)
			minx=x0;
		if (x1>maxx)
			maxx=x1;
		if (y0<miny)
			miny=y0;
		if (y1>maxy)
			maxy=y1;
	}
	NONS_LongRect refresh_area(minx,miny,maxx-minx,maxy-miny);
	refresh_area=refresh_area.intersect(NONS_LongRect(this->screen->inRect));
	if (!(refresh_area.w*refresh_area.h))
		return;
	this->screenBuffer.fill(refresh_area,NONS_Color::black_transparent);
	BLEND_OPTIM(this->Background,over);
	for (ulong a=this->layerStack.size()-1;a>this->sprite_priority;a--){
		NONS_Layer *p=this->layerStack[a];
		BLEND_OPTIM(p,over);
	}
	for (ulong a=0;a<this->charactersBlendOrder.size();a++){
		NONS_Layer *lay=*this->characters[charactersBlendOrder[a]];
		BLEND_OPTIM(lay,over);
	}
	for (long a=this->sprite_priority;a>=0;a--){
		NONS_Layer *p=this->layerStack[a];
		BLEND_OPTIM(p,over);
	}
	for (ulong a=0;a<this->filterPipeline.size();a++){
		pipelineElement &el=this->filterPipeline[a];
		NONS_GFX::callFilter(el.effectNo,el.color,el.ruleStr,this->screenBuffer,this->screenBuffer);
	}
	if (this->output->visible){
		if (!this->output->shadeLayer->useDataAsDefaultShade){
			BLEND_OPTIM(this->output->shadeLayer,multiply);
		}else{
			BLEND_OPTIM(this->output->shadeLayer,over);
		}
		BLEND_OPTIM(this->output->shadowLayer,over);
		BLEND_OPTIM(this->output->foregroundLayer,over);
	}
	BLEND_OPTIM(this->cursor,over);
	this->screen->get_screen().over(this->screenBuffer,&refresh_area,&refresh_area);
	this->screen->updateScreen(refresh_area.x,refresh_area.y,refresh_area.w,refresh_area.h);
}

ErrorCode NONS_ScreenSpace::BlendAll(ulong effect){
	this->BlendNoCursor(0);
	if (this->cursor && this->cursor->data)
		this->screenBuffer.over_frame(
			this->cursor->data,
			this->cursor->get_frame(),
			&this->cursor->position,
			&this->cursor->clip_rect
		);
	if (effect){
		NONS_GFX *e=this->gfx_store->retrieve(effect);
		if (!e)
			return NONS_UNDEFINED_EFFECT;
		return e->call(this->screenBuffer,NONS_Surface::null,this->screen);
	}
	return NONS_NO_ERROR;
}

ErrorCode NONS_ScreenSpace::BlendAll(ulong effect,long timing,const std::wstring *rule){
	this->BlendAll(0);
	return NONS_GFX::callEffect(effect,timing,rule,this->screenBuffer,NONS_Surface::null,*this->screen);
}

ErrorCode NONS_ScreenSpace::BlendNoCursor(ulong effect){
	this->BlendNoText(0);
	if (this->output->visible){
#define BlendNoCursor_A(f) \
	this->screenBuffer.f(this->output->shadeLayer->data,&this->output->shadeLayer->clip_rect)
		if (!this->output->shadeLayer->useDataAsDefaultShade)
			BlendNoCursor_A(multiply);
		else
			BlendNoCursor_A(over);
#define BlendNoCursor_B(dst) \
	this->screenBuffer.over_with_alpha(this->output->dst->data,&this->output->dst->clip_rect,0,this->output->dst->alpha)
		if (this->output->shadowLayer)
			BlendNoCursor_B(shadowLayer);
		BlendNoCursor_B(foregroundLayer);
	}
	if (effect){
		NONS_GFX *e=this->gfx_store->retrieve(effect);
		if (!e)
			return NONS_UNDEFINED_EFFECT;
		return e->call(this->screenBuffer,NONS_Surface::null,this->screen);
	}
	return NONS_NO_ERROR;
}

ErrorCode NONS_ScreenSpace::BlendNoCursor(ulong effect,long timing,const std::wstring *rule){
	this->BlendNoCursor(0);
	return NONS_GFX::callEffect(effect,timing,rule,this->screenBuffer,NONS_Surface::null,*this->screen);
}

ErrorCode NONS_ScreenSpace::BlendNoText(ulong effect){
	this->BlendOnlyBG(0);
	if (this->blendSprites){
		for (ulong a=this->layerStack.size()-1;a>this->sprite_priority;a--){
			NONS_Layer *l=this->layerStack[a];
			if (l && l->visible && l->data)
				this->screenBuffer.over_frame_with_alpha(l->data,l->get_frame(),&l->position,&l->clip_rect,l->alpha);
		}
	}
	for (ulong a=0;a<this->charactersBlendOrder.size();a++){
		NONS_Layer *l=*this->characters[charactersBlendOrder[a]];
		if (l && l->data)
			this->screenBuffer.over_frame_with_alpha(l->data,l->get_frame(),&l->position,&l->clip_rect,l->alpha);
	}
	if (this->blendSprites){
		for (long a=this->sprite_priority;a>=0;a--){
			NONS_Layer *l=this->layerStack[a];
			if (l && l->visible && l->data)
				this->screenBuffer.over_frame_with_alpha(l->data,l->get_frame(),&l->position,&l->clip_rect,l->alpha);
		}
	}
	for (ulong a=0;a<this->filterPipeline.size();a++){
		pipelineElement &el=this->filterPipeline[a];
		NONS_GFX::callFilter(el.effectNo,el.color,el.ruleStr,this->screenBuffer,this->screenBuffer);
	}
	for (std::map<long,NONS_GraphicBar>::iterator i=this->bars.begin(),e=this->bars.end();i!=e;++i){
		NONS_GraphicBar &bar=i->second;
		NONS_Rect rect=bar.rect;
		rect.w*=float(bar.current_value)/float(bar.total_value);
		this->screenBuffer.fill(NONS_LongRect(rect),bar.color);
	}
	if (effect){
		NONS_GFX *e=this->gfx_store->retrieve(effect);
		if (!e)
			return NONS_UNDEFINED_EFFECT;
		return e->call(this->screenBuffer,NONS_Surface::null,this->screen);
	}
	return NONS_NO_ERROR;
}

ErrorCode NONS_ScreenSpace::BlendNoText(ulong effect,long timing,const std::wstring *rule){
	this->BlendNoText(0);
	return NONS_GFX::callEffect(effect,timing,rule,this->screenBuffer,NONS_Surface::null,*this->screen);
}

ErrorCode NONS_ScreenSpace::BlendOnlyBG(ulong effect){
	this->screenBuffer.fill(NONS_Color::black);
	if (this->Background && this->Background->data)
		this->screenBuffer.copy_pixels(this->Background->data,&this->Background->position,&this->Background->clip_rect);
	if (effect){
		NONS_GFX *e=this->gfx_store->retrieve(effect);
		if (!e)
			return NONS_UNDEFINED_EFFECT;
		return e->call(this->screenBuffer,NONS_Surface::null,this->screen);
	}
	return NONS_NO_ERROR;
}

ErrorCode NONS_ScreenSpace::BlendOnlyBG(ulong effect,long timing,const std::wstring *rule){
	this->BlendOnlyBG(0);
	return NONS_GFX::callEffect(effect,timing,rule,this->screenBuffer,NONS_Surface::null,*this->screen);
}

void NONS_ScreenSpace::copyBufferToScreenWithoutUpdating(){
	this->screen->blitToScreen(this->screenBuffer,0,0);
}

void NONS_ScreenSpace::clearText(){
	this->output->Clear();
	this->BlendNoCursor(1);
	//SDL_UpdateRect(this->screen,0,0,0,0);
}

void NONS_ScreenSpace::hideText(){
	if (!this->output->text_visible)
		return;
	this->output->text_visible=0;
	this->BlendNoCursor(0);
	this->output->transition->call(this->screenBuffer,NONS_Surface::null,this->screen);
}

void NONS_ScreenSpace::showText(){
	if (this->output->text_visible)
		return;
	this->output->text_visible=1;
	this->BlendNoCursor(0);
	this->output->transition->call(this->screenBuffer,NONS_Surface::null,this->screen);
}

void NONS_ScreenSpace::hideTextWindow(){
	if (!this->output->visible)
		return;
	this->output->visible=0;
	this->BlendNoCursor(0);
	this->output->transition->call(this->screenBuffer,NONS_Surface::null,this->screen);
}

void NONS_ScreenSpace::showTextWindow(){
	if (this->output->visible)
		return;
	this->output->visible=1;
	this->BlendNoCursor(0);
	this->output->transition->call(this->screenBuffer,NONS_Surface::null,this->screen);
}

void NONS_ScreenSpace::resetParameters(const NONS_LongRect &window,const NONS_LongRect &frame,NONS_FontCache &fc,bool shadow){
	NONS_GFX *temp;
	bool a=this->output->transition->stored;
	if (a)
		temp=this->output->transition;
	else
		temp=new NONS_GFX(*this->output->transition);
	std::vector<std::wstring> temp_vector;
	if (CLOptions.never_clear_log)
		temp_vector=this->output->log;
	delete this->output;
	this->output=new NONS_StandardOutput(fc,window,frame,shadow);
	if (CLOptions.never_clear_log)
		this->output->log=temp_vector;
	delete this->output->transition;
	this->output->transition=temp;
	this->lookback->reset(this->output);
}

ErrorCode NONS_ScreenSpace::loadSprite(ulong n,const std::wstring &string,long x,long y,uchar alpha,bool visibility){
	if (!string[0])
		return NONS_EMPTY_STRING;
	if (n>this->layerStack.size())
		return NONS_INVALID_RUN_TIME_PARAMETER_VALUE;
	if (!this->layerStack[n])
		this->layerStack[n]=new NONS_Layer(&string);
	else
		this->layerStack[n]->load(&string);
	if (!this->layerStack[n]->data)
		return NONS_UNDEFINED_ERROR;
	this->layerStack[n]->position.x=(Sint16)x;
	this->layerStack[n]->position.y=(Sint16)y;
	this->layerStack[n]->visible=visibility;
	this->layerStack[n]->alpha=alpha;
	return NONS_NO_ERROR;
}

void NONS_ScreenSpace::clear(){
#define CHECK_AND_DELETE(p) if (p){ (p)->unload(); delete p; p=0; }
	CHECK_AND_DELETE(this->Background);
	CHECK_AND_DELETE(this->leftChar);
	CHECK_AND_DELETE(this->rightChar);
	CHECK_AND_DELETE(this->centerChar);
	for (ulong a=0;a<this->layerStack.size();a++)
		CHECK_AND_DELETE(this->layerStack[a]);
	this->clearText();
	this->BlendNoCursor(1);
}

bool NONS_ScreenSpace::advanceAnimations(ulong msecs,std::vector<NONS_LongRect> &rects){
	rects.clear();
	bool requireRefresh=0;
	std::vector<NONS_Layer *> arr;
	arr.reserve(5+this->layerStack.size());
	arr.push_back(this->Background);
	arr.push_back(this->leftChar);
	arr.push_back(this->rightChar);
	arr.push_back(this->centerChar);
	for (ulong a=0;a<this->layerStack.size();a++)
		arr.push_back(this->layerStack[a]);
	arr.push_back(this->cursor);
	for (ulong a=0;a<arr.size();a++){
		NONS_Layer *p=arr[a];
		if (!p || !p->data)
			continue;
		ulong first=p->animation.getCurrentAnimationFrame();
		if (p->advanceAnimation(msecs)){
			requireRefresh=1;
			ulong second=p->animation.getCurrentAnimationFrame();
			NONS_LongRect push=p->optimized_updates[std::pair<ulong,ulong>(first,second)];
			push.x+=p->position.x;
			push.y+=p->position.y;
			rects.push_back(push);
		}
	}
	return requireRefresh;
}

void NONS_ScreenSpace::addBar(long barNo,ulong current_value,long x,long y,ulong w,ulong h,ulong total_value,const NONS_Color &color){
	NONS_GraphicBar bar;
	bar.current_value=current_value;
	bar.total_value=total_value;
	bar.rect=NONS_Rect((float)x,(float)y,(float)w,(float)h);
	bar.color=color;
	this->bars[barNo]=bar;
}

void NONS_ScreenSpace::clearBars(){
	this->bars.clear();
}

TiXmlElement *NONS_ScreenSpace::save_characters(){
	TiXmlElement *characters=new TiXmlElement("characters");
	characters->SetAttribute("baseline",this->char_baseline);
	for (int a=0;a<3;a++){
		TiXmlElement *character=
			(!*this->characters[a] || !(*this->characters[a])->data)?
			new TiXmlElement("character"):
			(*this->characters[a])->save("character");
		characters->LinkEndChild(character);
		character->SetAttribute("no",this->charactersBlendOrder[a]);
	}
	return characters;
}

void NONS_ScreenSpace::load_characters(TiXmlElement *parent){
	TiXmlElement *characters=parent->FirstChildElement("characters");
	this->char_baseline=characters->QueryIntAttribute("baseline");
	for (int a=0;a<3;a++){
		delete *this->characters[a];
		*this->characters[a]=0;
	}
	this->charactersBlendOrder.clear();
	for (TiXmlElement *i=characters->FirstChildElement();i;i=i->NextSiblingElement()){
		NONS_Layer *new_character;
		if (i->FirstAttribute()->NameTStr()=="no")
			new_character=0;
		else
			new_character=new NONS_Layer(i,"");
		int order=i->QueryIntAttribute("no");
		this->charactersBlendOrder.push_back(order);
		*this->characters[order]=new_character;
	}
}

TiXmlElement *NONS_ScreenSpace::save_sprites(){
	TiXmlElement *sprites=new TiXmlElement("sprites");
	for (size_t a=0;a<this->layerStack.size();a++){
		NONS_Layer *layer=this->layerStack[a];
		if (!layer || !layer->data)
			continue;
		TiXmlElement *sprite=layer->save();
		sprites->LinkEndChild(sprite);
		sprite->SetAttribute("no",a);
	}
	sprites->SetAttribute("blend",this->blendSprites);
	sprites->SetAttribute("priority",this->sprite_priority);
	return sprites;
}

void NONS_ScreenSpace::load_sprites(TiXmlElement *parent){
	TiXmlElement *sprites=parent->FirstChildElement("sprites");
	for (size_t a=0;a<this->layerStack.size();a++){
		delete this->layerStack[a];
		this->layerStack[a]=0;
	}
	for (TiXmlElement *i=sprites->FirstChildElement();i;i=i->NextSiblingElement())
		this->layerStack[i->QueryIntAttribute("no")]=new NONS_Layer(i,"");
	this->blendSprites=sprites->QueryIntAttribute("blend");
	this->sprite_priority=sprites->QueryIntAttribute("priority");
}

TiXmlElement *NONS_ScreenSpace::save_filters(){
	TiXmlElement *filters=new TiXmlElement("filters");
	filters->SetAttribute("monochrome_first",this->apply_monochrome_first);
	{
		TiXmlElement *pipeline=new TiXmlElement("pipeline_A");
		filters->LinkEndChild(pipeline);
		for (size_t a=0;a<this->filterPipeline.size();a++)
			pipeline->LinkEndChild(this->filterPipeline[a].save());
	}
	filters->LinkEndChild(this->screen->save_async_fx());
	filters->LinkEndChild(this->screen->save_filter_pipeline("pipeline_B"));
	return filters;
}

void NONS_ScreenSpace::load_filters(TiXmlElement *grandfather){
	TiXmlElement *screen=grandfather->FirstChildElement("screen");
	TiXmlElement *filters=screen->FirstChildElement("filters");
	this->apply_monochrome_first=filters->QueryIntAttribute("monochrome_first");
	this->filterPipeline.clear();
	for (TiXmlElement *i=filters->FirstChildElement("pipeline_A")->FirstChildElement();i;i=i->NextSiblingElement())
		this->filterPipeline.push_back(i);
	this->screen->load_filter_pipeline(filters,"pipeline_B");
}

TiXmlElement *NONS_ScreenSpace::save(const interpreter_stored_state &state){
	TiXmlElement *screen=new TiXmlElement("screen");
	screen->LinkEndChild(this->output->save(state));
	if (this->Background)
		screen->LinkEndChild(this->Background->save("background"));
	screen->LinkEndChild(this->save_characters());
	screen->LinkEndChild(this->save_sprites());
	screen->LinkEndChild(this->save_filters());
	return screen;
}

void NONS_ScreenSpace::load(TiXmlElement *parent,NONS_FontCache &fc){
	TiXmlElement *screen=parent->FirstChildElement("screen");
	delete this->output;
	this->output=new NONS_StandardOutput(screen,fc);
	delete this->Background;
	this->Background=(!screen->FirstChildElement("background"))?0:new NONS_Layer(screen,"background");
	this->load_characters(screen);
	this->load_sprites(screen);
}

void NONS_ScreenSpace::load_async_effect(TiXmlElement *grandfather){
	TiXmlElement *screen=grandfather->FirstChildElement("screen");
	TiXmlElement *filters=screen->FirstChildElement("filters");
	this->screen->load_async_fx(filters);
}
