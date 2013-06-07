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

#include <fstream>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include "Common.h"
#include "ErrorCodes.h"
#include "ScriptInterpreter.h"
#include "IOFunctions.h"
#include "ThreadManager.h"
#include "Options.h"
#include "version.h"
#if NONS_SYS_WINDOWS
#include <windows.h>
#elif NONS_SYS_PSP
#include <pspkernel.h>
#endif

SDL_Surface* filp_screen;

int my_x;
int my_y;
int my_w;
int my_h;

SDL_mutex *ONSLlock;
int refreshLock=0;
unsigned int MainThreadID;

void useArgumentsFile(const wchar_t *filename,std::vector<std::wstring> &argv){
	NONS_DataStream *stream=general_archive.open(filename,FILESYSTEM_FIRST);
	if (!stream)
		return;
	std::string str;
	stream->read_all(str);
	general_archive.close(stream);
	append(argv,getParameterList(UniFromUTF8(str),0));
}

void enditall(bool stop_thread){
	if (stop_thread)
		gScriptInterpreter->stop();
}

void handle_SIGTERM(int){
	enditall(1);
}

void handle_SIGINT(int){
	STD_COUT <<"Forcefully terminating.\n";
}

volatile bool stopEventHandling=0;

int lastClickX=0;
int lastClickY=0;
bool useDebugMode=0,
	video_playback=0;

bool decode_joystick_event(SDL_Event &event){
	if (event.type==SDL_JOYAXISMOTION){
		int value=event.jaxis.value;
		if (ABS(value)<(1<<14))
			return 0;
		int axis=0;
		if (SDL_JoystickNumAxes(InputObserver.joysticks[event.jaxis.which])>=2)
			axis=1;
		if (event.jaxis.axis!=axis)
			return 0;
		SDLKey key=(value<0)?SDLK_UP:SDLK_DOWN;
		event.type=SDL_KEYDOWN;
		event.key.keysym.sym=key;
	}else if (event.type==SDL_JOYBUTTONDOWN){
		if (!event.jbutton.state || event.jbutton.button>=5)
			return 0;
		static SDLKey buttons[]={
			SDLK_RETURN,
			SDLK_ESCAPE,
			SDLK_PERIOD,
			SDLK_f,
			SDLK_F12
		};
		SDLKey key=buttons[event.jbutton.button];
		event.type=SDL_KEYDOWN;
		event.key.keysym.sym=key;
	}
	return 1;
}

void handleInputEvent(SDL_Event event){
	long x,y;
	switch(event.type){
		case SDL_USEREVENT:
		
			refreshLock=0;
		break;
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
			if (!decode_joystick_event(event))
				return;
			handleInputEvent(event);
			break;
		case SDL_QUIT:
			enditall(1);
			InputObserver.notify(&event);
			break;
		case SDL_KEYDOWN:
			if (useDebugMode){
				bool notify=0,
					full=0;
				switch (event.key.keysym.sym){
					case SDLK_RETURN:
						if (!!(event.key.keysym.mod&KMOD_ALT))
							full=1;
						else
							notify=1;
						break;
					case SDLK_F12:
						o_stdout <<"Screenshot saved to \""<<gScriptInterpreter->screen->screen->takeScreenshot()<<"\".\n";
						break;
					case SDLK_LCTRL:
					case SDLK_RCTRL:
					case SDLK_NUMLOCK:
					case SDLK_CAPSLOCK:
					case SDLK_SCROLLOCK:
					case SDLK_RSHIFT:
					case SDLK_LSHIFT:
					case SDLK_RALT:
					case SDLK_LALT:
					case SDLK_RMETA:
					case SDLK_LMETA:
					case SDLK_LSUPER:
					case SDLK_RSUPER:
					case SDLK_MODE:
					case SDLK_COMPOSE:
						break;
					default:
						notify=1;
				}
				if (full && gScriptInterpreter->screen)
					gScriptInterpreter->screen->screen->toggleFullscreen();
				if (notify)
					InputObserver.notify(&event);
			}else{
				bool notify=0,
					full=0;
				switch (event.key.keysym.sym){
					case SDLK_LCTRL:
					case SDLK_RCTRL:
						ctrlIsPressed=1;
						break;
					case SDLK_PERIOD:
						ctrlIsPressed=!ctrlIsPressed;
						break;
					case SDLK_f:
						if (!video_playback)
							full=1;
						else
							notify=1;
						break;
					case SDLK_s:
						if (gScriptInterpreter->audio)
							gScriptInterpreter->audio->toggle_mute();
						break;
					case SDLK_RETURN:
						if (CHECK_FLAG(event.key.keysym.mod,KMOD_ALT) && !video_playback)
							full=1;
						else
							notify=1;
						break;
					case SDLK_F12:
						if (!video_playback)
							o_stdout <<"Screenshot saved to \""<<gScriptInterpreter->screen->screen->takeScreenshot()<<"\".\n";
						else
							notify=1;
						break;
					case SDLK_NUMLOCK:
					case SDLK_CAPSLOCK:
					case SDLK_SCROLLOCK:
					case SDLK_RSHIFT:
					case SDLK_LSHIFT:
					case SDLK_RALT:
					case SDLK_LALT:
					case SDLK_RMETA:
					case SDLK_LMETA:
					case SDLK_LSUPER:
					case SDLK_RSUPER:
					case SDLK_MODE:
					case SDLK_COMPOSE:
						break;
					default:
						notify=1;
				}
				if (full && gScriptInterpreter->screen)
					gScriptInterpreter->screen->screen->toggleFullscreen();
				if (notify)
					InputObserver.notify(&event);
			}
			break;
		case SDL_KEYUP:
			InputObserver.notify(&event);
			if (event.key.keysym.sym==SDLK_LCTRL || event.key.keysym.sym==SDLK_RCTRL)
				ctrlIsPressed=0;
			break;
		case SDL_MOUSEMOTION:
			x=event.motion.x;
			y=event.motion.y;
			if (gScriptInterpreter->screen){
				x=(long)gScriptInterpreter->screen->screen->unconvertX(x);
				y=(long)gScriptInterpreter->screen->screen->unconvertY(y);
				event.motion.x=(Uint16)x;
				event.motion.y=(Uint16)y;
			}
			if (x>0 && y>0)
				InputObserver.notify(&event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			x=event.button.x;
			y=event.button.y;
			if (gScriptInterpreter->screen){
				x=(long)gScriptInterpreter->screen->screen->unconvertX(x);
				y=(long)gScriptInterpreter->screen->screen->unconvertY(y);
				event.button.x=(Uint16)x;
				event.button.y=(Uint16)y;
			}
			if (x>0 && y>0)
				InputObserver.notify(&event);
			lastClickX=event.button.x;
			lastClickY=event.button.y;
			break;
		default:
			InputObserver.notify(&event);
	}
}

std::vector<std::wstring> getArgumentsVector(char **argv){
	std::vector<std::wstring> ret;
	for (argv++;*argv;argv++)
		ret.push_back(UniFromUTF8(std::string(*argv)));
	return ret;
}

std::vector<std::wstring> getArgumentsVector(wchar_t **argv){
	std::vector<std::wstring> ret;
	for (argv++;*argv;argv++)
		ret.push_back(std::wstring(*argv));
	return ret;
}

extern wchar_t SJIS2Unicode[0x10000],
	Unicode2SJIS[0x10000],
	SJIS2Unicode_compact[];
void initialize_conversion_tables();
extern uchar integer_division_lookup[0x10000];

#if NONS_SYS_PSP
PSP_MODULE_INFO("ONSlaught", 0, 1, 1);
#endif

#if NONS_SYS_WINDOWS && defined _CONSOLE && defined main
#undef main
#endif

std::string get_version_string(){
	std::stringstream stream;
	stream <<"ONSlaught: An ONScripter clone with Unicode support.\n";
#if ONSLAUGHT_BUILD_VERSION<99999999


	stream <<"Build "<<ONSLAUGHT_BUILD_VERSION<<", ";
#endif
	stream <<ONSLAUGHT_BUILD_VERSION_STR"\n"
#ifdef NONS_LOW_MEMORY_ENVIRONMENT
		"Low memory usage build.\n"
#endif
		"\n"
		"Copyright (c) "ONSLAUGHT_COPYRIGHT_YEAR_STR", Helios (helios.vmg@gmail.com)\n"
		"All rights reserved.\n\n\n";
	return stream.str();
}

void initialize(int argc,char **argv){

	srand((unsigned int)time(0));
#if !defined NONS_SVN && !defined _DEBUG
	signal(SIGTERM,handle_SIGTERM);
	signal(SIGINT,handle_SIGINT);
	signal(SIGABRT,handle_SIGINT);
#endif
	initialize_conversion_tables();
	//initialize lookup table/s
	memset(integer_division_lookup,0,256);
	for (ulong y=1;y<256;y++)
		for (ulong x=0;x<256;x++)
			integer_division_lookup[x+y*256]=uchar(x*255/y);

	config_directory=getConfigLocation();

	general_archive.init();

	/*std::vector<std::wstring> cmdl_arg=getArgumentsVector(argv);
	useArgumentsFile(L"arguments.txt",cmdl_arg);
	CLOptions.parse(cmdl_arg);*/

	SDL_Init(SDL_INIT_EVERYTHING&~SDL_INIT_AUDIO);
	atexit(SDL_Quit);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(250,20);

	InputObserver.setup_joysticks();

#ifndef NONS_NO_STDOUT
	if (CLOptions.override_stdout){
		o_stdout.redirect();
		o_stderr.redirect();
		o_stdout <<get_version_string();
		STD_COUT <<"Redirecting.\n";
	}
#endif

	threadManager.setCPUcount();
#ifdef USE_THREAD_MANAGER
	threadManager.init(cpu_count);
#endif

	settings.init(config_directory+settings_filename);
	

	SDL_WM_SetCaption("ONSlaught ("ONSLAUGHT_BUILD_VERSION_STR")",0);
#if NONS_SYS_WINDOWS
	findMainWindow(L"ONSlaught ("ONSLAUGHT_BUILD_VERSION_WSTR L")");
#endif
}

void print_version_string(){
	STD_COUT <<get_version_string();
}

void uninitialize(){
	InputObserver.free_joysticks();
}

void mainThread(void *);

#ifdef BENCHMARK_ALPHA_BLENDING
extern Uint64 over_pixel_count;
extern double over_time_sum;
#endif

#ifdef defined(ANDROID)
int SDL_main(int argc,char **argv){
__android_log_print(ANDROID_LOG_ERROR, "test", "in main");
#else
int main(int argc,char **argv){
#endif

//MainThreadID=SDL_ThreadID();
//ONSLlock = SDL_CreateMutex();
#ifdef NONS_NO_STDOUT
	//Have at least one reference to std::cout, or some functions are liable to
	//crash after main() has returned.
	std::ostream &cout=std::cout;
#endif
	print_version_string();
	initialize(argc,argv);
	{
		NONS_ScriptInterpreter interpreter;
		gScriptInterpreter=&interpreter;
		/*NONS_DebuggingConsole debug_console;
		if (CLOptions.debugMode){
			debug_console.init();
			console=&debug_console;
		}*/

		NONS_Thread thread(mainThread);
		if (CLOptions.play.size())
			gScriptInterpreter->generic_play(CLOptions.play);
		else
			while (gScriptInterpreter->interpretNextLine());
			stopEventHandling=1;

	}
#ifdef BENCHMARK_ALPHA_BLENDING
	if (over_time_sum!=0)
		STD_COUT <<"Alpha blending speed: "<<double(over_pixel_count)/over_time_sum/1000.0<<" kpx/ms.\n";
#endif
	uninitialize();
	return 0;
}

void mainThread(void *){
	SDL_Event event;
		while (!stopEventHandling){
			while (!stopEventHandling){
				{
					
#if NONS_SYS_UNIX
					NONS_MutexLocker ml(caption_mutex);
#endif
					if (!SDL_PollEvent(&event))
						break;
				}
				handleInputEvent(event);
			}
			SDL_Delay(10);
		}
	
}
