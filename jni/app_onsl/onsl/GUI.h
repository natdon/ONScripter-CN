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

#ifndef NONS_GUI_H
#define NONS_GUI_H

#include "Common.h"
#include "VirtualScreen.h"
#include "Audio.h"
#include "Archive.h"
#include "SDLhead.h"
#include <string>
#include <set>
#include <ft2build.h>
#include <freetype/freetype.h>

class NONS_FreeType_Lib{
	FT_Library library;
	NONS_FreeType_Lib();
	NONS_FreeType_Lib(const NONS_FreeType_Lib &){}
	~NONS_FreeType_Lib();
public:
	static NONS_FreeType_Lib instance;
	FT_Library get_lib() const{ return this->library; }
};

class NONS_Font{
	FT_Face ft_font;
	FT_Stream stream;
	FT_Error error;
	ulong size;

	NONS_Font(const NONS_Font &){}
	void operator=(const NONS_Font &){}
public:
	ulong ascent,
		line_skip;

	NONS_Font(const std::wstring &filename);
	~NONS_Font();
	bool good() const{ return !this->error; }
	FT_Error get_error() const{ return this->error; }
	bool check_flag(unsigned flag) const{ return (this->ft_font->face_flags&flag)==flag; }
	bool is_monospace() const;
	void set_size(ulong size);
	FT_GlyphSlot get_glyph(wchar_t codepoint,bool italic,bool bold) const;
	FT_GlyphSlot render_glyph(wchar_t codepoint,bool italic,bool bold) const;
	FT_GlyphSlot render_glyph(FT_GlyphSlot) const;
	FT_Face get_font() const{ return this->ft_font; }
};

class NONS_FontCache;

class NONS_Glyph{
	NONS_FontCache &fc;
	wchar_t codepoint;
	//style properties:
	ulong size,
		outline_size;
	NONS_Color color,
		outline_color;
	bool italic,
		bold;
	//~style properties
	uchar *base_bitmap,
		*outline_base_bitmap;
	NONS_LongRect bounding_box,
		outline_bounding_box;
	ulong advance;
public:
	ulong refCount,
		real_outline_size;
	NONS_Glyph(
		NONS_FontCache &fc,
		wchar_t codepoint,
		ulong size,
		const NONS_Color &color,
		bool italic,
		bool bold,
		ulong outline_size,
		const NONS_Color &outline_color
	);
	~NONS_Glyph();
	bool operator<(const NONS_Glyph &b) const{ return this->codepoint<b.codepoint; }
	void setColor(const NONS_Color &color){ this->color=color; }
	void setOutlineColor(const NONS_Color &color){ this->outline_color=color; }
	ulong get_advance_fixed() const{ return this->advance; }
	const NONS_LongRect &get_bounding_box() const{ return this->bounding_box; }
	NONS_LongRect get_put_bounding_box(long x,long y) const{
		NONS_LongRect ret=(!this->outline_base_bitmap)?this->bounding_box:this->outline_bounding_box;
		ret.x+=x;
		ret.y+=y;
		return ret;
	}
	wchar_t get_codepoint() const{ return this->codepoint; }
	bool needs_redraw(ulong size,bool italic,bool bold,ulong outline_size) const;
	long get_advance() const;
	void put(const NONS_Surface &dst,int x,int y,uchar alpha=255);
	const NONS_FontCache &get_cache() const{ return this->fc; }
	NONS_FontCache &get_cache(){ return this->fc; }
	void done();
	ulong type(){ return 0; }
};

#ifdef _DEBUG
#define FONTCACHE_DEBUG_PARAMETERS , __FILE__ , __LINE__
#else
#define FONTCACHE_DEBUG_PARAMETERS
#endif

class NONS_FontCache{
	std::map<wchar_t,NONS_Glyph *> glyphs;
	NONS_Font &font;
	ulong size,
		outline_size;
	NONS_Color color,
		outline_color;
	bool italic,
		bold;
	std::set<NONS_Glyph *> garbage;
public:
	long spacing;
	ulong line_skip,
		font_line_skip,
		ascent;
#ifdef _DEBUG
	std::string declared_in;
	ulong line;
	NONS_FontCache(NONS_Font &font,ulong size,const NONS_Color &color,bool italic,bool bold,ulong outline_size,const NONS_Color &outline_color,const char *file,ulong line);
	NONS_FontCache(const NONS_FontCache &fc,const char *file,ulong line);
private:
	NONS_FontCache(const NONS_FontCache &fc);
public:
#else
	NONS_FontCache(NONS_Font &font,ulong size,const NONS_Color &color,bool italic,bool bold,ulong outline_size,const NONS_Color &outline_color);
	NONS_FontCache(const NONS_FontCache &fc);
#endif
	~NONS_FontCache();
	void reset_style(ulong size,bool italic,bool bold,ulong outline_size);
	void set_outline_size(ulong size){ this->outline_size=size; }
	void set_size(ulong size);
	void set_to_normal(){
		this->italic=0;
		this->bold=0;
	}
	void set_italic(bool i){ this->italic=i; }
	void set_bold(bool b){ this->bold=b; }
	void set_color(const NONS_Color &color){ this->color=color; }
	void set_outline_color(const NONS_Color &color){ this->outline_color=color; }
	NONS_Glyph *get_glyph(wchar_t c);
	void done(NONS_Glyph *g);
	const NONS_Font &get_font() const{ return this->font; }
	NONS_Font &get_font(){ return this->font; }
	ulong get_size() const{ return this->size; }
	const NONS_Color &get_color() const{ return this->color; }
	bool get_italic() const{ return this->italic; }
	bool get_bold() const{ return this->bold; }
};

class NONS_AutoGlyph{
	NONS_FontCache &cache;
	NONS_Glyph &glyph;
public:
	NONS_AutoGlyph(NONS_FontCache &fc,NONS_Glyph &g):cache(fc),glyph(g){}
	~NONS_AutoGlyph(){ this->cache.done(&this->glyph); }
};

NONS_Font *init_font(const std::wstring &filename);

struct NONS_StandardOutput;
struct NONS_ScreenSpace;
class NONS_ScriptInterpreter;
struct NONS_Menu;
struct NONS_Layer;
class NONS_EventQueue;

class NONS_Button{
	//friend struct NONS_ButtonLayer;
	//friend struct NONS_Lookback;
protected:
	bool status;
public:
	NONS_Button():status(0){}
	virtual ~NONS_Button(){}
	virtual void merge(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force=0)=0;
	virtual void mergeWithoutUpdate(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force=0)=0;
	bool MouseOver(SDL_Event *event);
	virtual bool MouseOver(int x,int y)=0;
	virtual NONS_LongRect get_dimensions()=0;
protected:
	void merge(
		NONS_VirtualScreen *dst,
		const NONS_LongRect &rect,
		int posx,
		int posy,
		const NONS_ConstSurface &original,
		bool status,
		bool force=0
	);
	void mergeWithoutUpdate(
		NONS_VirtualScreen *dst,
		const NONS_LongRect &rect,
		int posx,
		int posy,
		const NONS_ConstSurface &original,
		bool status,
		bool force=0
	);
	virtual void mergeWithoutUpdate_inner(NONS_Surface &dst,const NONS_LongRect &dstRect,const NONS_LongRect &srcRect)=0;
};

#define MOUSE_OVER(x,y,posx,posy,w,h) ((x)>=(posx) && (x)<=(posx)+(w) && (y)>=(posy) && (y)<=(posy)+(h))

class NONS_SurfaceButton:public NONS_Button{
protected:
	NONS_Layer *offLayer,
		*onLayer;
	NONS_LongRect box;
	int posx,posy,
		limitX,limitY;
public:
	NONS_SurfaceButton():NONS_Button(),offLayer(0),onLayer(0),posx(0),posy(0){}
	~NONS_SurfaceButton();
	void merge(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force=0);
	void mergeWithoutUpdate(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force=0);
	bool MouseOver(int x,int y){
		float posx=float(this->posx+this->box.x),
			posy=float(this->posy+this->box.y);
		return MOUSE_OVER(x,y,posx,posy,this->box.w,this->box.h);
	}
	virtual void dummy()=0;
	const NONS_Layer *getOffLayer() const{ return this->offLayer; }
	const NONS_Layer *getOnLayer() const{ return this->onLayer; }
	NONS_Layer *&setOffLayer(){ return this->offLayer; }
	NONS_Layer *&setOnLayer(){ return this->onLayer; }
	const NONS_LongRect &getBox() const{ return this->box; }
	int getPosx(){ return this->posx; }
	int getPosy(){ return this->posy; }
	int &setPosx(){ return this->posx; }
	int &setPosy(){ return this->posy; }
	NONS_LongRect get_dimensions();
protected:
	void mergeWithoutUpdate_inner(NONS_Surface &dst,const NONS_LongRect &dstRect,const NONS_LongRect &srcRect);
};

class NONS_GraphicButton:public NONS_SurfaceButton{
public:
	NONS_GraphicButton(const NONS_ConstSurface &src,int posx,int posy,int width,int height,int originX,int originY);
	void allocateLayer(NONS_Layer *&layer,const NONS_ConstSurface &src,int posx,int posy,int width,int height,int originX,int originY);
	void dummy(){}
};

class NONS_TextButton:public NONS_SurfaceButton{
protected:
	NONS_Layer *shadowLayer;
	NONS_FontCache font_cache;
public:
	NONS_TextButton(
		const std::wstring &text,
		const NONS_FontCache &fc,
		float center,
		const NONS_Color &on,
		const NONS_Color &off,
		bool shadow,
		int limitX=INT_MAX,
		int limitY=INT_MAX
	);
	~NONS_TextButton();
	const NONS_Layer *getShadowLayer() const{ return this->shadowLayer; }
	NONS_Layer *&setShadowLayer(){ return this->shadowLayer; }
	void dummy(){}
};

struct NONS_SpriteButton:public NONS_Button{
protected:
	ulong sprite;
	NONS_ScreenSpace *screen;
public:
	NONS_SpriteButton(ulong sprite,NONS_ScreenSpace *screen):sprite(sprite),screen(screen){}
	void merge(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force=0);
	void mergeWithoutUpdate(NONS_VirtualScreen *dst,const NONS_ConstSurface &original,bool status,bool force=0);
	bool MouseOver(int x,int y);
	NONS_LongRect get_dimensions();
protected:
	void mergeWithoutUpdate_inner(NONS_Surface &dst,const NONS_LongRect &dstRect,const NONS_LongRect &srcRect);
};

struct NONS_ButtonLayer{
	typedef std::map<ulong,NONS_Button *> map_t;
	map_t buttons;
	NONS_FontCache *font_cache;
	NONS_ScreenSpace *screen;
	std::wstring voiceEntry;
	std::wstring voiceMouseOver;
	std::wstring voiceClick;
	NONS_Audio *audio;
	NONS_LongRect boundingBox;
	bool exitable;
	NONS_Menu *menu;
	NONS_Surface loadedGraphic;
	struct anonymous_struct{
		bool Wheel,
			btnArea,
			EscapeSpace,
			PageUpDown,
			Enter,
			Tab,
			Function,
			Cursor,
			Insert,
			ZXC,
			space_returns_left_click;
		anonymous_struct()
			:Wheel(0),
			btnArea(0),
			EscapeSpace(0),
			PageUpDown(0),
			Enter(0),
			Tab(0),
			Function(0),
			Cursor(0),
			Insert(0),
			ZXC(0),
			space_returns_left_click(0){}
	} inputOptions;
	bool return_on_down;
	NONS_ButtonLayer(const NONS_Surface &img,NONS_ScreenSpace *screen);
	NONS_ButtonLayer(const NONS_FontCache &fc,NONS_ScreenSpace *screen,bool exitable,NONS_Menu *menu);
	~NONS_ButtonLayer();
	void makeTextButtons(const std::vector<std::wstring> &arr,
		const NONS_Color &on,
		const NONS_Color &off,
		bool shadow,
		std::wstring *entry,
		std::wstring *mouseover,
		std::wstring *click,
		NONS_Audio *audio,
		int width,
		int height);
	void addImageButton(ulong index,int posx,int posy,int width,int height,int originX,int originY);
	void addSpriteButton(ulong index,ulong sprite);
	/*
	returns:
		if >=0, the index of the button pressed
		-1 if escape was pressed and the layer was exitable (i.e. the layer was being used for the menu)
		-2 if the layer doesn't fit in the screen with the given coordinates
		-3 if escape was pressed and the layer wasn't exitable (i.e. the user tried to access the menu)
		INT_MIN if SDL_QUIT was received
	*/
	int getUserInput(int x,int y,bool override_placement=1);
	/*
	returns:
		if >=0, the index of the button pressed
		-1 if the user left-clicked, but not on a button
		<-1 for different key presses under certain circumstances
		INT_MIN if SDL_QUIT was received
	*/
	int getUserInput(ulong expiration=0);
private:
	bool react_to_movement(map_t::iterator &mouseOver,SDL_Event *event,const NONS_ConstSurface &screenCopy);
	void react_to_updown(map_t::iterator &mouseOver,SDLKey key,const NONS_ConstSurface &screenCopy);
	bool react_to_click(map_t::iterator &mouseOver,const NONS_ConstSurface &screenCopy);
};

struct NONS_Menu{
	std::vector<std::wstring> strings;
	std::vector<std::wstring> commands;
	NONS_Color off,
		on,
		nofile;
	bool shadow;
	NONS_ButtonLayer *buttons;
	ushort slots;
	NONS_ScriptInterpreter *interpreter;
	int x,y;
	NONS_Layer *shade;
	NONS_FontCache *font_cache,
		*default_font_cache;
	long fontsize,spacing,lineskip;
	NONS_Color shadeColor;
	std::wstring stringSave;
	std::wstring stringLoad;
	std::wstring stringSlot;
	std::wstring voiceEntry;
	std::wstring voiceCancel;
	std::wstring voiceMO;
	std::wstring voiceClick;
	std::wstring voiceYes;
	std::wstring voiceNo;
	NONS_Audio *audio;
	uchar rightClickMode;

	NONS_Menu(NONS_ScriptInterpreter *interpreter);
	NONS_Menu(std::vector<std::wstring> *options,NONS_ScriptInterpreter *interpreter);
	~NONS_Menu();
	int callMenu();
	void reset();
	void resetStrings(std::vector<std::wstring> *options);
	int save();
	int load();
	//0 if the user chose to quit, INT_MIN if SDL_QUIT was received
	int windowerase();
	int skip();
	int call(const std::wstring &string);
	NONS_FontCache &get_font_cache(){ return *(this->font_cache?this->font_cache:this->default_font_cache); }
private:
	int write(const std::wstring &txt,int y);
};

struct NONS_Lookback{
	NONS_Color foreground;
	NONS_StandardOutput *output;
	NONS_Button *up,
		*down;
	//surfaces<2: up button
	//surfaces>=2: down button
	//surfaces%2!=0: on state
	//surfaces%2==0: off state
	NONS_Surface surfaces[4];
	//bool use_sprites;
	//ulong up,down;
	NONS_Lookback(NONS_StandardOutput *output,const NONS_Color &color);
	~NONS_Lookback();
	bool setUpButtons(const std::wstring &upon,const std::wstring &upoff,const std::wstring &downon,const std::wstring &downoff);
	bool setUpButtons(ulong up,ulong down,NONS_ScreenSpace *screen);
	int display(NONS_VirtualScreen *dst);
	void reset(NONS_StandardOutput *output);
private:
	bool changePage(
		int dir,
		long &currentPage,
		const NONS_ConstSurface &copyDst,
		NONS_VirtualScreen *dst,
		NONS_Surface &preBlit,
		uchar &visibility,
		int &mouseOver
	);
	void resetButtons();
	void setUpButtons();
};

struct NONS_Cursor{
	NONS_Layer *data;
	long xpos,ypos;
	bool absolute;
	NONS_ScreenSpace *screen;
	NONS_Cursor(NONS_ScreenSpace *screen);
	NONS_Cursor(const std::wstring &string,long x,long y,long absolute,NONS_ScreenSpace *screen);
	NONS_Cursor(TiXmlElement *,NONS_ScreenSpace *screen,const char *name=0);
	~NONS_Cursor();
	int animate(NONS_Menu *menu,ulong expiration);
	TiXmlElement *save(const char *override_name=0);
private:
	//0 if the caller should return, 1 if it should continue
	bool callMenu(NONS_Menu *menu,NONS_EventQueue *queue);
	//0 if the caller should return, 1 if it should continue
	bool callLookback(NONS_EventQueue *queue);
};

struct NONS_CursorPair{
	NONS_Cursor *on;
	NONS_Cursor *off;
};

class NONS_GeneralArchive;

#define CONLOCATE(x,y) ((x)+(y)*this->screenW)

class NONS_DebuggingConsole{
	NONS_Font *font;
	NONS_FontCache *cache;
	ulong characterWidth,
		characterHeight,
		screenW,
		screenH,
		cursorX,
		cursorY;
	std::vector<wchar_t> screen;
	std::vector<std::wstring> pastInputs;
	std::wstring partial;
	bool print_prompt;
	void redraw(NONS_ScreenSpace *dst,long startFromLine,ulong cursor,const std::wstring &line);
	//Note: It does NOT lock the screen.
	void redraw(NONS_ScreenSpace *dst,long startFromLine,ulong lineHeight);
	std::vector<std::wstring> autocompleteVector;
	wchar_t &locate(size_t x,size_t y){
		return this->screen[CONLOCATE(x,y)];
	}
	bool input(std::wstring &input,NONS_ScreenSpace *dst);
	void autocomplete(std::vector<std::wstring> &dst,const std::wstring &line,std::wstring &suggestion,ulong cursor,ulong &space);
	void outputMatches(const std::vector<std::wstring> &matches,NONS_ScreenSpace *dst/*,long startFromLine,ulong cursor,const std::wstring &line*/);
public:
	NONS_DebuggingConsole();
	~NONS_DebuggingConsole();
	void init();
	void enter(NONS_ScreenSpace *dst);
	void output(const std::wstring &str,NONS_ScreenSpace *dst);
};

extern NONS_DebuggingConsole *console;
#endif
