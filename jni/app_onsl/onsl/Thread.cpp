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

#include "Thread.h"
#include <ctime>
#include <cstdlib>

#if NONS_SYS_WINDOWS
#include <windows.h>
#include <Aclapi.h>
#elif NONS_SYS_UNIX
#include <unistd.h>
#include <cerrno>
#elif NONS_SYS_PSP
#undef NONS_PARALLELIZE
#endif

void NONS_Event::init(){
#if NONS_SYS_WINDOWS
	this->event=CreateEvent(0,0,0,0);
#elif NONS_SYS_UNIX
	sem_init(&this->sem,0,0);
#elif NONS_SYS_PSP
	this->sem=SDL_CreateSemaphore(0);
#endif
	this->initialized=1;
}

NONS_Event::~NONS_Event(){
	if (!this->initialized)
		return;
#if NONS_SYS_WINDOWS
	CloseHandle(this->event);
#elif NONS_SYS_UNIX
	sem_destroy(&this->sem);
#elif NONS_SYS_PSP
	SDL_DestroySemaphore(this->sem);
#endif
}

void NONS_Event::set(){
#if NONS_SYS_WINDOWS
	SetEvent(this->event);
#elif NONS_SYS_UNIX
	sem_post(&this->sem);
#elif NONS_SYS_PSP
	SDL_SemPost(this->sem);
#endif
}

void NONS_Event::reset(){
#if NONS_SYS_WINDOWS
	ResetEvent(this->event);
#endif
}

void NONS_Event::wait(){
#if NONS_SYS_WINDOWS
	WaitForSingleObject(this->event,INFINITE);
#elif NONS_SYS_UNIX
	while (sem_wait(&this->sem)<0 && errno==EINTR);
#elif NONS_SYS_PSP
	SDL_SemWait(this->sem);
#endif
}

NONS_Thread::~NONS_Thread(){
	if (!this->called)
		return;
	if (this->join_at_destruct){
		this->join();
		this->free();
	}
}

void NONS_Thread::free(){
#if NONS_SYS_WINDOWS
	CloseHandle(this->thread);
#endif
}

void NONS_Thread::call(NONS_ThreadedFunctionPointer function,void *data,bool give_highest_priority){
	if (this->called)
		return;
	threadStruct *ts=new threadStruct;
	ts->f=function;
	ts->d=data;
#if NONS_SYS_WINDOWS
	if (!give_highest_priority)
		this->thread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)runningThread,ts,0,0);
	else{
		this->thread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)runningThread,ts,REALTIME_PRIORITY_CLASS,0);
		SetThreadPriority(this->thread,THREAD_PRIORITY_TIME_CRITICAL);
	}
#elif NONS_SYS_UNIX
	pthread_attr_t attr,
		*pattr=0;
	if (give_highest_priority){
		pattr=&attr;
		pthread_attr_init(pattr);
		sched_param params;
		pthread_attr_getschedparam(pattr,&params);
		int policy;
		pthread_attr_getschedpolicy(pattr,&policy);
		params.sched_priority=sched_get_priority_max(policy);
		pthread_attr_setschedparam(pattr,&params);
	}
	pthread_create(&this->thread,pattr,runningThread,ts);
	if (give_highest_priority)
		pthread_attr_destroy(pattr);
#elif NONS_SYS_PSP
	this->thread=SDL_CreateThread(runningThread,ts);
#endif
	this->called=1;
}

void NONS_Thread::join(){
	if (!this->called)
		return;
#if NONS_SYS_WINDOWS
	WaitForSingleObject(this->thread,INFINITE);
#elif NONS_SYS_UNIX
	pthread_join(this->thread,0);
#elif NONS_SYS_PSP
	SDL_WaitThread(this->thread,0);
#endif
	this->called=0;
}

void NONS_Thread::unbind(){
	if (!this->join_at_destruct)
		return;
	this->join_at_destruct=0;
	this->free();
}

NONS_Thread_DECLARE_THREAD_FUNCTION(NONS_Thread::runningThread){
	srand((unsigned int)time(0));
	NONS_ThreadedFunctionPointer f=((threadStruct *)p)->f;
	void *d=((threadStruct *)p)->d;
	delete (threadStruct *)p;
	f(d);
	return 0;
}

NONS_Mutex::NONS_Mutex(){
#if NONS_SYS_WINDOWS
	this->mutex=new CRITICAL_SECTION;
	InitializeCriticalSection((CRITICAL_SECTION *)this->mutex);
#elif NONS_SYS_UNIX
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&this->mutex,&attr);
	pthread_mutexattr_destroy(&attr);
#elif NONS_SYS_PSP
	this->mutex=SDL_CreateMutex();
#endif
}

NONS_Mutex::~NONS_Mutex(){
#if NONS_SYS_WINDOWS
	DeleteCriticalSection((CRITICAL_SECTION *)this->mutex);
	delete (CRITICAL_SECTION *)this->mutex;
#elif NONS_SYS_UNIX
	pthread_mutex_destroy(&this->mutex);
#elif NONS_SYS_PSP
	SDL_DestroyMutex(this->mutex);
#endif
}

void NONS_Mutex::lock(){
#if NONS_SYS_WINDOWS
	EnterCriticalSection((CRITICAL_SECTION *)this->mutex);
#elif NONS_SYS_UNIX
	pthread_mutex_lock(&this->mutex);
#elif NONS_SYS_PSP
	SDL_LockMutex(this->mutex);
#endif
}

void NONS_Mutex::unlock(){
#if NONS_SYS_WINDOWS
	LeaveCriticalSection((CRITICAL_SECTION *)this->mutex);
#elif NONS_SYS_UNIX
	pthread_mutex_unlock(&this->mutex);
#elif NONS_SYS_PSP
	SDL_UnlockMutex(this->mutex);
#endif
}
