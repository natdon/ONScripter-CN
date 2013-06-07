#ifndef __SDLHEAD_H__
#define __SDLHEAD_H__

#if !defined(ANDROID)
#include <SDL/SDL_image.h>
#include <SDL/SDL.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_stdinc.h>
#elif defined(ANDROID)
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_stdinc.h>
#include <jni.h>
#include <android/log.h>
#endif
#endif
