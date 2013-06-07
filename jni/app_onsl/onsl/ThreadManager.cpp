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

#include "ThreadManager.h"
#include "IOFunctions.h"
#include "Options.h"
#include <iostream>
#include <cassert>
#include <cerrno>

#define NONS_PARALLELIZE

#if NONS_SYS_WINDOWS
#include <windows.h>
#elif NONS_SYS_UNIX
#include <unistd.h>
#elif NONS_SYS_PSP
#undef NONS_PARALLELIZE
#endif

NONS_DLLexport ulong cpu_count=1;
NONS_DLLexport NONS_ThreadManager threadManager;

void NONS_ManagedThread::init(ulong index){
	this->initialized=1;
	this->index=index;
	this->startCallEvent.init();
	this->callEndedEvent.init();
	this->thread.call(runningThread,this);
	this->function=0;
	this->parameter=0;
	this->destroy=0;
}

NONS_ManagedThread::~NONS_ManagedThread(){
	if (!this->initialized)
		return;
	this->destroy=1;
	this->startCallEvent.set();
	this->thread.join();
}

void NONS_ManagedThread::call(NONS_ThreadedFunctionPointer f,void *p){
	this->function=f;
	this->parameter=p;
	this->startCallEvent.set();
}

void NONS_ManagedThread::wait(){
	this->callEndedEvent.wait();
}

NONS_ThreadManager::NONS_ThreadManager(ulong CPUs){
	this->init(CPUs);
}

void NONS_ThreadManager::init(ulong CPUs){
	this->threads.resize(CPUs-1);
	for (ulong a=0;a<this->threads.size();a++)
		this->threads[a].init(a);
}

ulong NONS_ThreadManager::call(ulong onThread,NONS_ThreadedFunctionPointer f,void *p){
	if (onThread>=this->threads.size())
		return -1;
	this->threads[onThread].call(f,p);
	return onThread;
}

void NONS_ThreadManager::wait(ulong index){
	this->threads[index].wait();
}

void NONS_ThreadManager::waitAll(){
	for (ulong a=0;a<this->threads.size();a++)
		this->wait(a);
}

void NONS_ManagedThread::runningThread(void *p){
	NONS_ManagedThread *t=(NONS_ManagedThread *)p;
	while (1){
		t->startCallEvent.wait();
		if (t->destroy)
			break;
		assert(t->function!=0);
		t->function(t->parameter);
		t->parameter=0;
		t->function=0;
		t->callEndedEvent.set();
	}
}

void NONS_ThreadManager::setCPUcount(){
	if (CLOptions.noThreads)
		cpu_count=1;
	else{
#ifdef NONS_PARALLELIZE
		//get CPU count
#if NONS_SYS_WINDOWS
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		cpu_count=si.dwNumberOfProcessors;
#elif NONS_SYS_UNIX
		cpu_count=sysconf(_SC_NPROCESSORS_ONLN);
		if (cpu_count<1)
			cpu_count=1;
#else
		cpu_count=1;
#endif
#else
		cpu_count=1;
#endif
	}
	if (cpu_count==1)
		o_stdout <<"Parallelization disabled.\n";
	else
		o_stdout <<"Using "<<cpu_count<<" CPUs.\n";
}
