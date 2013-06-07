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

#ifndef NONS_THREAD_H
#define NONS_THREAD_H

#include "Common.h"
#include "Binder.h"
#include <vector>
#if NONS_SYS_UNIX
#include <pthread.h>
#include <semaphore.h>
#elif NONS_SYS_PSP
#include <SDL.h>
#include <SDL_thread.h>
#endif

typedef void (*NONS_ThreadedFunctionPointer)(void *);

#if NONS_SYS_WINDOWS
typedef HANDLE NONS_Thread_internal;
#define NONS_Thread_DECLARE_THREAD_FUNCTION(name) unsigned long __stdcall name(void *p)
#elif NONS_SYS_UNIX
typedef pthread_t NONS_Thread_internal;
#define NONS_Thread_DECLARE_THREAD_FUNCTION(name) void *name(void *p)
#else
typedef SDL_Thread *NONS_Thread_internal;
#define NONS_Thread_DECLARE_THREAD_FUNCTION(name) int name(void *p)
#endif

class NONS_Thread{
	struct threadStruct{ NONS_ThreadedFunctionPointer f; void *d; };
	NONS_Thread_internal thread;
	static NONS_Thread_DECLARE_THREAD_FUNCTION(runningThread);
	bool called,
		join_at_destruct;
	void free();
public:
	NONS_Thread():called(0),join_at_destruct(1){}
	NONS_Thread(NONS_ThreadedFunctionPointer function,bool give_highest_priority=0):called(0),join_at_destruct(1){
		this->call(function,0,give_highest_priority);
	}
	template <typename T>
	NONS_Thread(T *o,bool give_highest_priority=0):called(0),join_at_destruct(1){
		this->call(o,give_highest_priority);
	}
	~NONS_Thread();
	void call(NONS_ThreadedFunctionPointer function,void *data,bool give_highest_priority=0);
	template <typename T>
	void call(T *o,bool give_highest_priority=0){
		this->call(&call_bound<T>,o,give_highest_priority);
	}
	void join();
	void unbind();
};

class NONS_Event{
	bool initialized;
#if NONS_SYS_WINDOWS
	HANDLE event;
#elif NONS_SYS_UNIX
	sem_t sem;
#elif NONS_SYS_PSP
	SDL_sem *sem;
#endif
public:
	NONS_Event():initialized(0){}
	void init();
	~NONS_Event();
	void set();
	void reset();
	void wait();
};

class NONS_DECLSPEC NONS_Mutex{
#if NONS_SYS_WINDOWS
	//pointer to CRITICAL_SECTION
	void *mutex;
#elif NONS_SYS_UNIX
	pthread_mutex_t mutex;
#elif NONS_SYS_PSP
	SDL_mutex *mutex;
#endif
public:
	NONS_Mutex();
	~NONS_Mutex();
	void lock();
	void unlock();
};

class NONS_MutexLocker{
	NONS_Mutex &mutex;
	NONS_MutexLocker(const NONS_MutexLocker &m):mutex(m.mutex){}
	void operator=(const NONS_MutexLocker &){}
public:
	NONS_MutexLocker(NONS_Mutex &m):mutex(m){
		this->mutex.lock();
	}
	~NONS_MutexLocker(){
		this->mutex.unlock();
	}
};
#endif
