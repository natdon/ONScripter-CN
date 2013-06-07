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

#ifndef NONS_THREADMANAGER_H
#define NONS_THREADMANAGER_H

#include "Thread.h"

#define USE_THREAD_MANAGER

class NONS_ManagedThread{
	bool initialized;
	NONS_Thread thread;
	static void runningThread(void *);
	ulong index;
	volatile bool destroy;
	void *parameter;
public:
	NONS_Event startCallEvent,
		callEndedEvent;
	volatile NONS_ThreadedFunctionPointer function;
	NONS_ManagedThread():initialized(0){}
	~NONS_ManagedThread();
	void init(ulong index);
	void call(NONS_ThreadedFunctionPointer f,void *p);
	void wait();
};

class NONS_ThreadManager{
	std::vector<NONS_ManagedThread> threads;
public:
	NONS_ThreadManager(){}
	NONS_ThreadManager(ulong CPUs);
	void init(ulong CPUs);
	template <typename T>
	ulong call(ulong onThread,T *o){
		if (onThread>=this->threads.size())
			return -1;
		this->threads[onThread].call(&call_bound<T>,o);
		return onThread;
	}
	NONS_DECLSPEC ulong call(ulong onThread,NONS_ThreadedFunctionPointer f,void *p);
	NONS_DECLSPEC void wait(ulong index);
	NONS_DECLSPEC void waitAll();
	static void setCPUcount();
};

extern NONS_DECLSPEC NONS_ThreadManager threadManager;
#endif
