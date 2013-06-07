/*
* Copyright (c) 2009-2010, Helios (helios.vmg@gmail.com)
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

#ifndef SVG_LOADER_H
#define SVG_LOADER_H
#include <SDL.h>

#ifndef NONS_SYS_WINDOWS
#define NONS_SYS_WINDOWS (defined _WIN32 || defined _WIN64)
#endif
#ifndef NONS_SYS_LINUX
#define NONS_SYS_LINUX (defined linux || defined __linux)
#endif
#ifndef NONS_SYS_BSD
#define NONS_SYS_BSD (defined __bsdi__)
#endif
#ifndef NONS_SYS_UNIX
#define NONS_SYS_UNIX (defined __unix__ || defined __unix)
#endif
#ifndef NONS_SYS_PSP
#define NONS_SYS_PSP (defined PSP)
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_BLOCK_O extern "C"{
#define EXTERN_C_BLOCK_C }
#else
#define EXTERN_C
#define EXTERN_C_BLOCK_O
#define EXTERN_C_BLOCK_C
#endif

typedef unsigned long ulong;
typedef unsigned char uchar;

EXTERN_C_BLOCK_O
#ifndef BUILD_SVG
#define SVG_DECLARE_FUNCTION(return,name,parameters) typedef return (*name##_f)parameters;\
SVG_DECLSPEC return name parameters
#else
#define SVG_DECLARE_FUNCTION(return,name,parameters) typedef return(*name##_f)parameters;
#endif

#if NONS_SYS_WINDOWS
#ifdef BUILD_SVG
#define SVG_DECLSPEC __declspec(dllexport)
#else
#define SVG_DECLSPEC __declspec(dllimport)
#endif
#define SVG_DLLexport __declspec(dllexport)
#define SVG_DLLimport __declspec(dllimport)
#else
#define SVG_DECLSPEC
#define SVG_DLLexport
#define SVG_DLLimport
#endif

SVG_DECLARE_FUNCTION(ulong,SVG_load,(void *buffer,size_t size));
SVG_DECLARE_FUNCTION(int,SVG_unload,(ulong index));
SVG_DECLARE_FUNCTION(int,SVG_get_dimensions,(ulong index,double *w,double *h));
SVG_DECLARE_FUNCTION(int,SVG_set_scale,(ulong index,double scale_x,double scale_y));
SVG_DECLARE_FUNCTION(int,SVG_best_fit,(ulong index,ulong max_x,ulong max_y));
SVG_DECLARE_FUNCTION(int,SVG_set_rotation,(ulong index,double angle));
SVG_DECLARE_FUNCTION(int,SVG_set_matrix,(ulong index,double matrix[4]));
SVG_DECLARE_FUNCTION(int,SVG_transform_coordinates,(ulong index,double x,double y,double *dst_x,double *dst_y));
SVG_DECLARE_FUNCTION(int,SVG_add_scale,(ulong index,double scale_x,double scale_y));
SVG_DECLARE_FUNCTION(SDL_Surface *,SVG_render,(ulong index));
SVG_DECLARE_FUNCTION(int,SVG_render2,(ulong index,SDL_Surface *dst,double offset_x,double offset_y,uchar alpha));
SVG_DECLARE_FUNCTION(int,SVG_have_linear_transformations,());
EXTERN_C_BLOCK_C
#endif
