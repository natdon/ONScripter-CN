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

#include "Plugin.h"
#include "../Functions.h"
#include "../IOFunctions.h"
#include <iostream>
#include <vector>



#ifdef EFFECT_INITIALIZE_DELAYS
#undef EFFECT_INITIALIZE_DELAYS
#endif
#define EFFECT_INITIALIZE_DELAYS(base)                                \
	NONS_Clock clock;                                                 \
	NONS_Clock::t delay=NONS_Clock::t(double(duration)/double(base)), \
		idealtimepos=0,                                               \
		lastT=NONS_Clock::MAX,                                        \
		start=clock.get()

/*
Available data:
ulong cpu_count;
	The number of physical cores in the system. When threading is disabled, this is always 1.
bool ctrlIsPressed;
	Whether the user is holding down CTRL. Also becomes true if the user pressed period or
	selected a menu option to skip.
bool forceSkip;
	Whether a function has asynchronously requested termination.
NONS_ThreadManager threadManager;
	See below.
NONS_Mutex screenMutex;
	Used to lock accesses to the screen, to prevent race conditions. See transitionExample()
	for an example on how to use it.

Available functions:
bool effect_standard_check(NONS_LongRect &dst,const NONS_ConstSurface &s,NONS_VirtualScreen &d);
	Performs some checks common to all transition effects.
	dst receives the area of d that will be affected.
	s is the source surface.
	d is the destination screen.
void effect_epilogue();
	Performs cleanup operations common to all transition effects.

Available classes:
class NONS_ConstSurface;
	See src/Image.h.
class NONS_Surface:NONS_ConstSurface;
	See src/Image.h.
class NONS_Clock;
	Declared in src/IOFunctions.h
	Wrapper for high-precision clock. Under Windows, the precision matches the CPU's frequency. The
	precision is of 0.001 s or worse for other systems.
class NONS_Mutex;
	Declared in src/IOFunctions.h
	Wrapper for recursive mutexes.

class NONS_ThreadManager{
	Declared in src/ThreadManager.h
	Manages a custom-implemented thread pool. It contains n-1 threads, where n is the number of
	available CPU cores (the last instance is executed on the main thread). When a thread isn't
	being used, it consumes no CPU time.
public:
	template <typename T>
	ulong call(ulong onThread,T *o);
	ulong call(ulong onThread,NONS_ThreadedFunctionPointer f,void *p);
		Call a function on thread #onThread. Non-blocking.
		onThread should be >=0 and <cpu_count-1.
		The first version is designed to take either a binder * or a member_binder * as its first
		parameter. It can be used to call both global and member functions. There are many examples
		on how to use member and member_binder and other facilities defined in src/Binder.h all
		around the engine.
		The second version is a more typical setup. The second parameter must be a pointer to a
		global function that returns nothing and takes a void * as its only parameter, and the
		third parameter a possibly null pointer to user data.
		NONS_ThreadedFunctionPointer is defined as:
			typedef void (*NONS_ThreadedFunctionPointer)(void *);
		On most cases, the first version is easier to use.
	void wait(ulong index);
		Lock the caller thread until the managed thread #index returns from its function. If the
		thread isn't in a function already, the call does nothing.
	void waitAll();
		Calls wait() for every thread. You'll most commonly use this, rather than wait().
};
	
Utility class that guarantees that each mutex lock is paired with an unlock. It's heavily
recommended to use this class, rather than making the un/locks yourself.
class NONS_MutexLocker{
	NONS_Mutex &mutex;
public:
	NONS_MutexLocker(NONS_Mutex &m):mutex(m){
		this->mutex.lock();
	}
	~NONS_MutexLocker(){
		this->mutex.unlock();
	}
};

class NONS_VirtualScreen;
	Declared in src/VirtualScreen.h
	Unless told to by an example, you shouldn't use any of the functions declared by this class
	directly.
*/

//Declare new functions here.
FILTER_EFFECT_F(filterExample);
FILTER_EFFECT_F(filterExample2);
TRANSIC_EFFECT_F(transitionExample);
ASYNC_EFFECT_F(asyncExample);
ASYNC_EFFECT_INIT_F(asyncExample_init);
ASYNC_EFFECT_UNINIT_F(asyncExample_uninit);

void *getTransitionEffectFunctions(){
	static std::vector<transitionFX_f> ret;
	if (!ret.size()){
		//Add new functions here.
		ret.push_back(transitionExample);
	}
	return &ret;
}

void *getFilterEffectFunctions(){
	static std::vector<filterFX_f> ret;
	if (!ret.size()){
		//Add new functions here.
		ret.push_back(filterExample);
		ret.push_back(filterExample2);
	}
	return &ret;
}

void *getAsyncEffectFunctions(){
	static std::vector<asyncFXfunctionSet> ret;
	if (!ret.size()){
		//Add new functions here.
		ret.push_back(asyncFXfunctionSet(asyncExample_init,asyncExample,asyncExample_uninit));
	}
	return &ret;
}

extern "C" PLUGIN_DLLexport void *getFunctionPointers(void *param){
	switch ((ulong)param){
		case 0:
			return getTransitionEffectFunctions();
		case 1:
			return getFilterEffectFunctions();
		case 2:
			return getAsyncEffectFunctions();
	}
	return 0;
}

//Define new functions here.

/*
Asynchronous effect initialization function (#01):

Signature:
	void *asyncExample_init(ulong effectNo)
Parameters:
	effectNo: The effect number.
Returns:
	A generic pointer that can be used by the matching effect function and will be freed by the
	matching uninitializer function.
Description: 
	Can be used to initialize state used by #03. If it's not necessary, it can be left undefined
	and pass a zero to asyncFXfunctionSet::asyncFXfunctionSet()'s first parameter. The engine
	will not call it in that case.
*/
ASYNC_EFFECT_INIT_F(asyncExample_init){
	return new ulong(0);
}

/*
Asynchronous effect uninitialization function (#02):

Signature:
	void *asyncExample_uninit(ulong effectNo, void *userData)
Parameters:
	effectNo: The effect number.
	userData: The generic pointer that was returned by #01.
Returns:
	Nothing.
Description:
	Can be used to free state allocated by #01. If it's not necessary, it can be left undefined and
	pass a zero to asyncFXfunctionSet::asyncFXfunctionSet()'s third parameter. The engine will not
	call it in that case.
*/
ASYNC_EFFECT_UNINIT_F(asyncExample_uninit){
	delete (ulong *)userData;
}

/*
Asynchronous effect function (#03):

Signature:
	bool asyncExample(
		ulong effectNo,
		const NONS_ConstSurface &src,
		const NONS_Surface &dst,
		void *userData
	)
Parameters:
	effectNo: The effect number.
	src: The source surface.
	dst: The destination surface.
	userData: The generic pointer that was returned by #01.
Returns:
	Whether the engine should refresh the screen after the function returns.
Description:
	The function is called n times per second (n being the second parameter for async_effect in the
	script) to apply an effect. That means that, on average, the function shouldn't take longer
	than 1000/n milliseconds to finish, so performance is a priority. To give a general	idea, this
	example function takes ~11 ms on a Core 2 Duo @ 1.68 GHz.
	This function should not use threading, since it's already running on its own thread. Adding
	more threading wouldn't improve the performance, specially in single core systems. Unless the
	engine was compiled with threading disabled, NONS_Surface::over() and all of its variants use
	threads, so they shouldn't be called from here, either.
	This particular example makes the engine update the screen each time it's called, but that's not a
	requirement. Refresh the screen as little as possible.
*/
ASYNC_EFFECT_F(asyncExample){
	NONS_ConstSurfaceProperties ssp;
	NONS_SurfaceProperties dsp;
	src.get_properties(ssp);
	dst.get_properties(dsp);
	ulong *n=(ulong *)userData;
	const uchar *pos_src=ssp.pixels;
	uchar *pos_dst=dsp.pixels;
	for (ulong y=0;y<dsp.h;y++){
		const uchar *pos_src0=pos_src;
		uchar *pos_dst0=pos_dst;
		if (y!=*n){
			for (ulong x=0;x<dsp.w;x++){
				ulong v=rand()&0xFF;
				//c = c*7/8 + v*1/8 (It's an alpha blend with the noise layer at 12.5% alpha.)
				pos_dst[dsp.offsets[0]]=uchar((ulong(pos_src[ssp.offsets[0]])*7+v)>>3);
				pos_dst[dsp.offsets[1]]=uchar((ulong(pos_src[ssp.offsets[1]])*7+v)>>3);
				pos_dst[dsp.offsets[2]]=uchar((ulong(pos_src[ssp.offsets[2]])*7+v)>>3);
				//(The alpha remains the same.)
				pos_dst[dsp.offsets[3]]=pos_src[ssp.offsets[3]];
				pos_src+=4;
				pos_dst+=4;
			}
		}else
			memset(pos_dst,0xFF,dsp.pitch);
		pos_src=pos_src0+ssp.pitch;
		pos_dst=pos_dst0+dsp.pitch;
	}
	*n=(*n+2)%dsp.h;
	return 1;
}

/*
Filter effect function (#04):

Signature:
	void filterExample(
		ulong effectNo,
		NONS_Color color,
		const NONS_ConstSurface &src,
		const NONS_ConstSurface &rule,
		const NONS_Surface &dst,
		NONS_LongRect area
	)
Parameters:
	effectNo: The effect number.
	color: A color value. If this isn't used, it can be ignored. The color is obtained from the
	       second parameter provided for effects (e.g. the fourth parameter for the command
	       effect).
	src: The source surface.
	rule: A "rule" surface that can be used for some effect. E.g. the rule could be an image that
	      will be alpha-blended into the image.
	      The rule is obtained from the third parameter provided for effects (e.g. the fourth
	      parameter for the command effect). If the rule cannot be found as a file, a null surface
	      will be passed (if (!rule) src is null). So if this parameter is needed, that should be
	      checked.
	dst: The destination surface.
	area: The area on dst than needs to be filtered.
Returns:
	Nothing.
Description:
	The function is used to apply a filter to a surface. For example, the engine internally uses
	this very interface to implement the function behind the command nega. In fact, this example
	function performs the exact same effect.
	Depending on what commands were used in the script, src and dst may actually always point to
	the same surface. However, this shouldn't be relied on.
	The function is always called from the main thread, so it's allowed to use threading functions.
	such as NONS_Image::over(), to speed up its execution. However, the function is never called
	from performance-critical sections.
*/
FILTER_EFFECT_F(filterExample){
	//Obtain information about the surfaces.
	NONS_ConstSurfaceProperties ssp;
	NONS_SurfaceProperties dsp;
	src.get_properties(ssp);
	dst.get_properties(dsp);
	//Advance pointers to the top-left corner of the rectangle being filtered.
	ssp.pixels+=area.x*4+area.y*ssp.pitch;
	dsp.pixels+=area.x*4+area.y*dsp.pitch;
	for (long y=0;y<area.h;y++){
		const uchar *pos00=ssp.pixels;
		uchar *pos10=dsp.pixels;
		for (long x=0;x<area.w;x++){
			//This is a faster version of c=255-c.
			dsp.pixels[dsp.offsets[0]]=~ssp.pixels[ssp.offsets[0]];
			dsp.pixels[dsp.offsets[1]]=~ssp.pixels[ssp.offsets[1]];
			dsp.pixels[dsp.offsets[2]]=~ssp.pixels[ssp.offsets[2]];
			//(The alpha remains the same.)
			dsp.pixels[dsp.offsets[3]]=ssp.pixels[ssp.offsets[3]];
			//Move pointers to the next pixel.
			ssp.pixels+=4;
			dsp.pixels+=4;
		}
		//Move pointers to the next row, relative from the start of the current row.
		ssp.pixels=pos00+ssp.pitch;
		dsp.pixels=pos10+dsp.pitch;
	}
}

inline uchar posterize(ulong c,ulong levels){
	ulong divide=(256/levels);
	ulong v=c/divide*divide*255/(0xFF^divide);
	saturate_value<ulong>(v,0,255);
	return (uchar)v;
}

FILTER_EFFECT_F(filterExample2){
	NONS_ConstSurfaceProperties ssp;
	NONS_SurfaceProperties dsp;
	src.get_properties(ssp);
	dst.get_properties(dsp);
	ssp.pixels+=area.x*4+area.y*ssp.pitch;
	dsp.pixels+=area.x*4+area.y*dsp.pitch;
	for (long y=0;y<area.h;y++){
		const uchar *pos00=ssp.pixels;
		uchar *pos10=dsp.pixels;
		for (long x=0;x<area.w;x++){
			const ulong levels=8;
			dsp.pixels[dsp.offsets[0]]=posterize(ssp.pixels[ssp.offsets[0]],levels);
			dsp.pixels[dsp.offsets[1]]=posterize(ssp.pixels[ssp.offsets[1]],levels);
			dsp.pixels[dsp.offsets[2]]=posterize(ssp.pixels[ssp.offsets[2]],levels);
			dsp.pixels[dsp.offsets[3]]=ssp.pixels[ssp.offsets[3]];
			ssp.pixels+=4;
			dsp.pixels+=4;
		}
		ssp.pixels=pos00+ssp.pitch;
		dsp.pixels=pos10+dsp.pitch;
	}
}

/*
Transition effect function (#05):

Signature:
	void transitionExample(
		ulong effectNo,
		ulong duration,
		const NONS_ConstSurface &src,
		const NONS_ConstSurface &rule,
		NONS_VirtualScreen &dst
	)
Parameters:
	effectNo: The effect number.
	duration: The function shouldn't take longer to complete than this many milliseconds.
	src: The source surface.
	rule: The "rule" surface. See #04 for details.
	dst: The destination virtual screen.
Returns:
	Nothing.
Description:
	This is the only effect that doesn't write directly to an SDL_Surface. This is because
	a transition effect is expected to update the screen several times before completing.
	Transition effects are also allowed to use threading functions, which may be important
	for more complex and expensive effects.
*/
TRANSIC_EFFECT_F(transitionExample){
	NONS_LongRect src_rect;
	//Perform common checks.
	if (!effect_standard_check(src_rect,src,dst))
		return;
	//We'll only need to update a small rectangle each iteration.
	NONS_LongRect rect(0,0,1,src_rect.h);
	long w=src_rect.w;
	//Initialize objects used for timing iteration delays.
	EFFECT_INITIALIZE_DELAYS(w);
	for (long a=0;a<w;a++){
		//Advance counters and perform checks to skip iterations.
		EFFECT_ITERATION_PROLOGUE(a<w-1,rect.w++;);
		{
			//Get screen object. If necessary, this locks mutexes to prevent race
			//conditions.
			NONS_Surface screen=dst.get_screen();
			screen.copy_pixels(src,&rect,&rect);
		}
		//We could also call dst.updateWholeScreen(), but the would update unnecessary pixels.
		dst.updateScreen(rect.x,rect.y,rect.w,rect.h);
		rect.x+=rect.w;
		rect.w=1;
		//Conditionally delay execution.
		EFFECT_ITERATION_EPILOGUE;
	}
	//Perform cleanup.
	effect_epilogue();
}
