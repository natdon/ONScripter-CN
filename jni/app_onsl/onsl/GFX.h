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

#ifndef NONS_GFX_H
#define NONS_GFX_H

#include "Common.h"
#include "ErrorCodes.h"
#include "VirtualScreen.h"
#include "tinyxml/tinyxml.h"
#include "SDLhead.h"
#include <set>
#include <map>

enum{
	TRANSITION=0,
	POSTPROCESSING=1
};

#define TRANSIC_EFFECT_F(name)         \
	void name(                         \
		ulong effectNo,                \
		ulong duration,                \
		const NONS_ConstSurface &src,  \
		const NONS_ConstSurface &rule, \
		NONS_VirtualScreen &dst        \
	)
typedef TRANSIC_EFFECT_F((*transitionFX_f));

struct NONS_GFX{
	ulong effect;
	ulong duration;
	std::wstring rule;
	long type;
	bool stored;
	NONS_Color color;
	static ulong effectblank;
	static bool listsInitialized;
	static std::vector<filterFX_f> filters;
	static std::vector<transitionFX_f> transitions;

	NONS_GFX(ulong effect=0,ulong duration=0,const std::wstring *rule=0);
	NONS_GFX(const NONS_GFX &b);
	NONS_GFX(TiXmlElement *,const char *name=0);
	NONS_GFX &operator=(const NONS_GFX &b);
	static ErrorCode callEffect(
		ulong number,
		long duration,
		const std::wstring *rule,
		const NONS_ConstSurface &src,
		const NONS_Surface &dst0,
		NONS_VirtualScreen &dst
	);
	static ErrorCode callFilter(
		ulong number,
		const NONS_Color &color,
		const std::wstring &rule,
		const NONS_ConstSurface &src,
		const NONS_Surface &dst
	);
	ErrorCode call(const NONS_ConstSurface &src,const NONS_Surface &dst0,NONS_VirtualScreen *dst);
	void effectNothing(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectOnlyUpdate(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectRshutter(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectLshutter(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectDshutter(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectUshutter(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectRcurtain(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectLcurtain(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectDcurtain(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectUcurtain(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectCrossfade(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectRscroll(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectLscroll(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectDscroll(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectUscroll(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectHardMask(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectMosaic(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst,long start,long end,long step);
	void effectMosaicIn(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectMosaicOut(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);
	void effectSoftMask(const NONS_ConstSurface &src0,const NONS_ConstSurface &src1,NONS_VirtualScreen &dst);

	TiXmlElement *save(const char *override_name=0);

	static void initializeLists();
};

struct NONS_GFXstore{
	std::map<ulong,NONS_GFX *> effects;
	NONS_GFX *add(ulong code,ulong effect,ulong duration,const std::wstring *rule=0);
	NONS_GFX *retrieve(ulong code);
	bool remove(ulong code);
	NONS_GFXstore();
	~NONS_GFXstore();
};

NONS_DECLSPEC bool effect_standard_check(NONS_LongRect &dst,const NONS_ConstSurface &s,NONS_VirtualScreen &d);
NONS_DECLSPEC void effect_epilogue();
#define EFFECT_INITIALIZE_DELAYS(base)                                      \
	NONS_Clock clock;                                                       \
	NONS_Clock::t delay=NONS_Clock::t(double(this->duration)/double(base)), \
		idealtimepos=0,                                                     \
		lastT=NONS_Clock::MAX,                                              \
		start=clock.get()
#define EFFECT_ITERATION_PROLOGUE(condition,action)                         \
	idealtimepos+=delay;                                                    \
	NONS_Clock::t t0=clock.get();                                           \
	if ((t0-start-idealtimepos>lastT || CURRENTLYSKIPPING) && (condition)){ \
		{action}                                                            \
		continue;                                                           \
	}
#define EFFECT_ITERATION_EPILOGUE      \
	NONS_Clock::t t1=clock.get();      \
	lastT=t1-t0;                       \
	if (lastT<delay)                   \
		SDL_Delay(Uint32(delay-lastT))
#endif
