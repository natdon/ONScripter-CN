/*
* Copyright (c) 2010, Helios (helios.vmg@gmail.com)
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

/* C-friendly header follows. */

#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H
#ifndef __PLUGIN__
#include <SDL.h>

#ifndef NONS_SYS_WINDOWS
#define NONS_SYS_WINDOWS (defined _WIN32 || defined _WIN64)
#endif
#ifndef NONS_SYS_LINUX
#define NONS_SYS_LINUX (defined linux || defined __linux)
#endif
#ifndef NONS_SYS_UNIX
#define NONS_SYS_UNIX (defined __unix__ || defined __unix || (__APPLE__ && __MACH__))
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#if NONS_SYS_WINDOWS
#ifdef BUILD_VIDEO_PLAYER
#define VIDEO_DECLSPEC __declspec(dllexport)
#else
#define VIDEO_DECLSPEC __declspec(dllimport)
#endif
#define VIDEO_DLLexport __declspec(dllexport)
#define VIDEO_DLLimport __declspec(dllimport)
#else
#define VIDEO_DECLSPEC
#define VIDEO_DLLexport
#define VIDEO_DLLimport
#endif

typedef unsigned long ulong;
typedef SDL_Surface *(*playback_cb)(volatile SDL_Surface *,void *);
typedef struct{
	bool (*write)(const void *src,ulong length,ulong channels,ulong frequency,void *);
	double (*get_time_offset)(void *);
	void (*wait)(void *,bool);
} audio_f;
#endif

typedef struct{
	void *data;
	int (*open)(void *,const char *);
	int (*close)(void *);
	int (*read)(void *,uint8_t *,int);
	int64_t (*seek)(void *,int64_t,int);
} file_protocol;

#ifndef __PLUGIN__
typedef struct{
/*versionless section*/
	ulong version;
/*version 1:*/
	SDL_Surface *screen;
	const char *input;
	void *user_data;
	audio_f audio_output;
	ulong trigger_callback_pairs_n;
	typedef struct{
		volatile int *trigger;
		playback_cb callback;
	} trigger_callback_pair;
	trigger_callback_pair *pairs;
	volatile int *stop;
	int print_debug;
	char *exception_string;
	size_t exception_string_size;
	file_protocol protocol;
/*version 2:*/
} C_play_video_params;
#define C_PLAY_VIDEO_PARAMS_VERSION 1

#define VIDEO_CONSTRUCTOR_PARAMETERS void
#define VIDEO_CONSTRUCTOR_NAME new_player
#define VIDEO_CONSTRUCTOR_NAME_STRING "new_player"
#define VIDEO_CONSTRUCTOR_SIGNATURE \
	EXTERN_C VIDEO_DECLSPEC void *VIDEO_CONSTRUCTOR_NAME(VIDEO_CONSTRUCTOR_PARAMETERS)

/*
 * video_constructor_fp:
 *
 * Description (new_player()):
 *     Constructs a new video player object that will hold all internal state
 *     and returns a pointer to it. Free this object by calling
 *     delete_player().
 *
 * Parameters:
 *     None.
 * Returns:
 *     A generic pointer to the new object. 
 *
 */
typedef void *(*video_constructor_fp)(VIDEO_CONSTRUCTOR_PARAMETERS);
VIDEO_CONSTRUCTOR_SIGNATURE;

#define VIDEO_DESTRUCTOR_PARAMETERS void *player
#define VIDEO_DESTRUCTOR_NAME delete_player
#define VIDEO_DESTRUCTOR_NAME_STRING "delete_player"
#define VIDEO_DESTRUCTOR_SIGNATURE \
	EXTERN_C VIDEO_DECLSPEC void VIDEO_DESTRUCTOR_NAME(VIDEO_DESTRUCTOR_PARAMETERS)

/*
 * video_destructor_fp:
 *
 * Description (delete_player()):
 *     Destructs a video player object constructed by new_player().
 *
 * Parameters:
 *     A generic pointer to the object that will be freed.
 *
 */
typedef void (*video_destructor_fp)(VIDEO_DESTRUCTOR_PARAMETERS);
VIDEO_DESTRUCTOR_SIGNATURE;

#define PLAYBACK_FUNCTION_PARAMETERS void *player,void *parameters
#define PLAYBACK_FUNCTION_NAME C_play_video
#define PLAYBACK_FUNCTION_NAME_STRING "C_play_video"
#define PLAYBACK_FUNCTION_SIGNATURE \
	EXTERN_C VIDEO_DECLSPEC int PLAYBACK_FUNCTION_NAME(PLAYBACK_FUNCTION_PARAMETERS)

/*
 * video_playback_fp:
 *
 * Description (C_play_video()):
 *     Plays a video using libVLC for parsing, decoding, and audio output, and
 *     SDL for video output.
 *     The video, audio, and subtitle streams to use will be the default ones,
 *     if they exist.
 *     Subtitle redering depends on available plugins.
 *     The function requires exclusive access to the destination surface, so if
 *     there are other threads running in the caller program that could read or
 *     write to the surface, either they should be stopped, or accesses to the
 *     surface should be locked with a mutex.
 *     If the video and the destination surface are of different dimensions, an
 *     interpolation function will be applied to each frame.
 *     If the video and the destination surface are of different aspect ratios,
 *     the video is sized and positioned so that it fits completely inside the
 *     surface. For example, playing a 1920x1080 (16:9) video on a
 *     1024x768 (4:3) surface would produce a letterboxed picture, while doing
 *     the opposite would produce a pillarboxed picture.
 *     The function clears (i.e. fill with black) the destination before, but
 *     not after being finishing. That job is left to the caller.
 *     Like any video-related code, the function is rather resource-consuming.
 *     The computational requirements depend on the kind of codec used by the
 *     video. The exact memory requirements depend on: the size of the screen,
 *     the size of the video, and the kind of codec and container used.
 *
 * Parameters:
 *     void *player
 *         The pointer returned by new_player.
 *     void *parameters
 *         It should be a pointer to a C_play_video_params structure. See below
 *         for details.
 * Returns:
 *     0 if the function failed, anything else otherwise.
 *
 * C_play_video_params:
 *     This structure is designed to allow ABI compatibility between different
 *     dynamic library versions.
 *     Its members are:
 *     ulong version
 *         The structure version that the client application is using. Set this
 *         to C_PLAY_VIDEO_PARAMS_VERSION. If this version is higher than the
 *         one the library was built for, the funtion will fail.
 *     SDL_Surface *screen
 *         A pointer to the SDL_Surface that the video will be rendered to. The
 *         surface *must* be the real screen (i.e. the pointer returned by
 *         SDL_SetVideoMode()).
 *     const char *input
 *         The path to the video file to be played.
 *     void *user_data
 *         This pointer will be passed to the callback functions if they are
 *         called.
 *     audio_output_cb audio_output
 *         Pointer to a function that handles writing to an audio device.
 *     ulong trigger_callback_pairs_n
 *         The size of the array pointed to by pairs.
 *     trigger_callback_pair *pairs
 *         A pointer to an array of pairs of int pointers and function
 *         pointers. The function continually checks each of the ints pointed
 *         to and, when it finds one that's !0, calls the associated function.
 *         All pointers must be valid. The function doesn't check them.
 *         The callbacks may not be ran from the same thread as the one that
 *         called C_play_video.
 *         All callbacks must return updated pointers to the screen. If a
 *         callback doesn't relocate the screen (e.g. by not calling
 *         SDL_SetVideoMode()), it should return the same value it was passed.
 *         Using callbacks may place the caller program under the GPL.
 *     int *stop
 *         The function continually checks the int pointed to by stop and
 *         returns as soon as possible when its value becomes !0. The pointer
 *         is not checked for validity.
 *     int print_debug
 *         The function writes debug messages to stdout if this is !0.
 *     char *exception_string
 *         A pointer to a char array that will be used to write extra error
 *         information in case the function fails.
 *     size_t exception_string_size
 *         The size of the buffer pointed to by exception_string. The function
 *         Will write at most this many characters to it.
 *
 */
typedef int (*video_playback_fp)(PLAYBACK_FUNCTION_PARAMETERS);
PLAYBACK_FUNCTION_SIGNATURE;

#define PLAYER_TYPE_FUNCTION_PARAMETERS void
#define PLAYER_TYPE_FUNCTION_NAME get_player_type
#define PLAYER_TYPE_FUNCTION_NAME_STRING "get_player_type"
#define PLAYER_TYPE_FUNCTION_SIGNATURE \
	EXTERN_C VIDEO_DECLSPEC const char *PLAYER_TYPE_FUNCTION_NAME \
	(PLAYER_TYPE_FUNCTION_PARAMETERS)

/*
 * player_type_fp:
 *
 * Description (get_player_type()):
 *     Returns the library that the player uses as a string. E.g.: "FFmpeg",
 *     "libVLC".
 *
 */
typedef const char *(*player_type_fp)(PLAYER_TYPE_FUNCTION_PARAMETERS);
PLAYER_TYPE_FUNCTION_SIGNATURE;

#define PLAYER_VERSION_FUNCTION_PARAMETERS void
#define PLAYER_VERSION_FUNCTION_NAME get_player_version
#define PLAYER_VERSION_FUNCTION_NAME_STRING "get_player_version"
#define PLAYER_VERSION_FUNCTION_SIGNATURE \
	EXTERN_C VIDEO_DECLSPEC ulong PLAYER_VERSION_FUNCTION_NAME \
	(PLAYER_VERSION_FUNCTION_PARAMETERS)

/*
 * player_type_fp:
 *
 * Description (get_player_type()):
 *     Returns the value of C_PLAY_VIDEO_PARAMS_VERSION that the player was
 *     built with.
 *
 */
typedef ulong (*player_version_fp)(PLAYER_VERSION_FUNCTION_PARAMETERS);
PLAYER_VERSION_FUNCTION_SIGNATURE;
#endif
#endif
