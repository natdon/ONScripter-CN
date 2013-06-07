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

#ifndef NONS_SCREENSPACE_H
#define NONS_SCREENSPACE_H

#include "Common.h"
#include "ErrorCodes.h"
#include "enums.h"
#include "GUI.h"
#include "VirtualScreen.h"
#include "GFX.h"
#include "Image.h"
#include "tinyxml/tinyxml.h"
#include <map>
#include <vector>

struct NONS_Layer{
	//The actual bitmap and other stuff.
	NONS_Surface data;
	//If null, it's an image layer. Otherwise it's a text layer.
	NONS_FontCache *fontCache;
	//Um... I think this is passed to SDL_FillRect() whenever told to do a Clear().
	NONS_Color defaultShade;
	//Determines whether this layer will be included in the blend.
	bool visible;
	bool useDataAsDefaultShade;
	NONS_LongRect clip_rect,
		position;
	uchar alpha;
	optim_t optimized_updates;
	NONS_AnimationInfo animation;
	NONS_Layer(const NONS_LongRect &size,const NONS_Color &rgba);
	NONS_Layer(const NONS_Surface &img,const NONS_Color &rgba);
	NONS_Layer(const std::wstring *string);
	NONS_Layer(TiXmlElement *,const char *name=0);
	~NONS_Layer();
	void MakeTextLayer(NONS_FontCache &fc,const NONS_Color &foreground);
	bool load(const std::wstring *string);
	bool load(const NONS_Surface &src);
	//if the parameter is true and the image isn't shared, the call has no effect
	void unload();
	void usePicAsDefaultShade(const NONS_Surface &s);
	void setShade(const NONS_Color &color);
	void Clear();
	//1 if the layer should be re-blended.
	bool advanceAnimation(ulong msec);
	void centerAround(int x);
	void useBaseline(int y);
	bool animated(){
		return this->animation.frame_ends.size() && 
			this->animation.animation_time_offset<this->animation.frame_ends.back()*
			(this->animation.frame_ends.size()==1?this->animation.animation_length:1);
	}
	ulong get_frame() const{
		long r=this->animation.getCurrentAnimationFrame();
		if (r<0)
			r=0;
		return (ulong)r;
	}
	TiXmlElement *save(const char *override_name=0);
};

struct interpreter_stored_state;

struct NONS_StandardOutput{
	//Current distance (px) from the upper left corner of the cursor to the left edge of the screen.
	int x;
	//Current distance (px) from the upper left corner of the cursor to the top edge of the screen.
	int y;
	//Distance (px) from the upper left corner of the frame to the left edge of the screen.
	int x0;
	//Distance (px) from the upper left corner of the frame to the top edge of the screen.
	int y0;
	//Width of the frame.
	int w;
	//Height of the frame.
	int h;
	NONS_Layer *foregroundLayer,
		*shadowLayer,
		*shadeLayer;
	long shadowPosX,
		shadowPosY;
	ulong display_speed;
	int extraAdvance;
	bool visible,text_visible;
	NONS_GFX *transition;
	std::vector<std::wstring> log;
	std::wstring currentBuffer;
	ulong indentationLevel;
	bool indent_next;
	long maxLogPages;

	NONS_StandardOutput(NONS_Layer *fgLayer,NONS_Layer *shadowLayer,NONS_Layer *shadeLayer);
	NONS_StandardOutput(NONS_FontCache &fc,const NONS_LongRect &size,const NONS_LongRect &frame,bool shadow=1);
	NONS_StandardOutput(TiXmlElement *,NONS_FontCache &fc,const char *name=0);
	void Clear(bool eraseBuffer=1);
	~NONS_StandardOutput();
	void setPosition(int x,int y);
	//if return==1, there should be a call to the page cursor and a screen clear before
	//calling print().
	bool prepareForPrinting(std::wstring str);
	bool print(ulong start,ulong end,NONS_VirtualScreen *dst,ulong *printedChars=0);
	void endPrinting();
	void ephemeralOut(const std::wstring &str,const NONS_Surface &dst,bool writeToLayers,const NONS_Color *col);
	void ephemeralOut(const std::wstring &str,NONS_VirtualScreen *dst,bool update,bool writeToLayers,const NONS_Color *col);
	float getCenterPolicy(char which);
	void setCenterPolicy(char which,float val);
	void setCenterPolicy(char which,long val);
	bool NewLine();
	void set_italic(bool);
	bool get_italic(){ return this->foregroundLayer->fontCache->get_italic(); }
	void set_bold(bool);
	bool get_bold(){ return this->foregroundLayer->fontCache->get_bold(); }
	void set_size(ulong);
	TiXmlElement *save(const interpreter_stored_state &state,const char *override_name=0);
private:
	int predictLineLength(std::wstring *arr,long start,int width);
	int predictTextHeight(std::wstring *arr);
	int setLineStart(std::wstring *arr,ulong start,NONS_LongRect *frame,float center);
	int setTextStart(std::wstring *arr,NONS_LongRect *frame,float center);
	ulong getIndentationSize();
	float horizontalCenterPolicy;
	float verticalCenterPolicy;
	long lastStart;
	std::wstring cachedText;
	std::wstring prebufferedText;
	bool resumePrinting;
	bool printingStarted;
	ulong resumePrintingWhere;
};

struct NONS_GraphicBar{
	ulong current_value,
		total_value;
	NONS_Rect rect;
	NONS_Color color;
};

struct NONS_ScreenSpace{
	std::vector<NONS_Layer *> layerStack;
	NONS_Layer *Background,
		*leftChar,
		*rightChar,
		*centerChar,
		**characters[3],
		*cursor;
	NONS_VirtualScreen *screen;
	NONS_Surface screenBuffer;
	NONS_StandardOutput *output;
	NONS_GFXstore *gfx_store;
	bool apply_monochrome_first;
	std::vector<pipelineElement> filterPipeline;
	ulong sprite_priority;
	NONS_Lookback *lookback;
	ulong char_baseline;
	bool blendSprites;
	std::vector<ulong> charactersBlendOrder;
	std::map<long,NONS_GraphicBar> bars;

	NONS_ScreenSpace(int framesize,NONS_FontCache &fc);
	NONS_ScreenSpace(const NONS_LongRect &window,const NONS_LongRect &frame,NONS_FontCache &fc,bool shadow);
	~NONS_ScreenSpace();

	ErrorCode BlendAll(ulong effect);
	ErrorCode BlendAll(ulong effect,long timing,const std::wstring *rule);
	void BlendOptimized(std::vector<NONS_LongRect> &rects);
	ErrorCode BlendNoCursor(ulong effect);
	ErrorCode BlendNoCursor(ulong effect,long timing,const std::wstring *rule);
	ErrorCode BlendNoText(ulong effect);
	ErrorCode BlendNoText(ulong effect,long timing,const std::wstring *rule);
	ErrorCode BlendOnlyBG(ulong effect);
	ErrorCode BlendOnlyBG(ulong effect,long timing,const std::wstring *rule);
	void copyBufferToScreenWithoutUpdating();
	void clearText();
	void hideText();
	void showText();
	void hideTextWindow();
	void showTextWindow();
	void resetParameters(const NONS_LongRect &window,const NONS_LongRect &frame,NONS_FontCache &fc,bool shadow);
	void clear();
	ErrorCode loadSprite(ulong n,const std::wstring &string,long x,long y,uchar alpha,bool visibility);
	bool advanceAnimations(ulong msecs,std::vector<NONS_LongRect> &rects);

	void addBar(long barNo,ulong current_value,long x,long y,ulong w,ulong h,ulong total_value,const NONS_Color &color);
	void clearBars();

	TiXmlElement *save(const interpreter_stored_state &state);
	void load(TiXmlElement *,NONS_FontCache &fc);
	void load_async_effect(TiXmlElement *);
	void load_filters(TiXmlElement *);
private:
	void alloc_screen();

	TiXmlElement *save_characters();
	void load_characters(TiXmlElement *);
	TiXmlElement *save_sprites();
	void load_sprites(TiXmlElement *);
	TiXmlElement *save_filters();
};
#endif
