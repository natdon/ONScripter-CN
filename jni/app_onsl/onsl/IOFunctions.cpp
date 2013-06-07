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

#include "IOFunctions.h"
#include "Options.h"
#include "SDLhead.h"
#include <iostream>
#include <ctime>
#include <cassert>
#include <cfloat>
#if NONS_SYS_WINDOWS
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#elif NONS_SYS_UNIX
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#endif

#define STDOUT_FILENAME "stdout.txt"
#define STDERR_FILENAME "stderr.txt"
#define STDLOG_FILENAME "stdlog.txt"

NONS_InputObserver InputObserver;
#ifndef NONS_NO_STDOUT
NONS_RedirectedOutput o_stdout(std::cout);
NONS_RedirectedOutput o_stderr(std::cerr);
#else
NONS_RedirectedOutput o_stdout(new std::ofstream(STDOUT_FILENAME));
NONS_RedirectedOutput o_stderr(new std::ofstream(STDERR_FILENAME));
#endif
NONS_FileSystem filesystem;
NONS_MemoryFS memoryFS;

NONS_File::NONS_File(const std::wstring &path,bool read){
	this->is_open=0;
#if NONS_SYS_WINDOWS
	this->file=INVALID_HANDLE_VALUE;
#elif NONS_SYS_UNIX
	this->file=0;
#endif
	this->open(path,read);
}

void NONS_File::open(const std::wstring &path,bool open_for_read){
	this->close();
	this->opened_for_read=open_for_read;
#if NONS_SYS_WINDOWS
	this->file=CreateFile(
		&path[0],
		(open_for_read?GENERIC_READ:GENERIC_WRITE),
		(open_for_read?FILE_SHARE_READ:0),
		0,
		(open_for_read?OPEN_EXISTING:TRUNCATE_EXISTING),
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	if (this->file==INVALID_HANDLE_VALUE && GetLastError()==ERROR_FILE_NOT_FOUND){
		this->file=CreateFile(
			&path[0],
			(open_for_read?GENERIC_READ:GENERIC_WRITE),
			(open_for_read?FILE_SHARE_READ:0),
			0,
			(open_for_read?OPEN_EXISTING:CREATE_NEW),
			FILE_ATTRIBUTE_NORMAL,
			0
		);
	}
	this->is_open=this->file!=INVALID_HANDLE_VALUE;
#elif NONS_SYS_UNIX
	this->file=::open(
		UniToUTF8(path).c_str(),
		(open_for_read?O_RDONLY:O_WRONLY|O_TRUNC)|O_LARGEFILE
	);
	if (this->file<0){
		this->file=::open(
			UniToUTF8(path).c_str(),
			(open_for_read?O_RDONLY:O_WRONLY|O_CREAT)|O_LARGEFILE,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
		);
	}
	this->is_open=this->file>=0;
#else
	this->file.open(UniToUTF8(path).c_str(),std::ios::binary|(open_for_read?std::ios::in:std::ios::out));
	this->is_open=this->file.is_open();
#endif
	this->_filesize=this->reload_filesize();
}

Uint64 NONS_File::reload_filesize(){
	if (this->is_open){
#if NONS_SYS_WINDOWS
		LARGE_INTEGER li;
		return (GetFileSizeEx(this->file,&li))?li.QuadPart:0;
#elif NONS_SYS_UNIX
		assert(this->file>=0);
		return (Uint64)lseek64(this->file,0,SEEK_END);
#else
		if (this->opened_for_read){
			this->file.seekg(0,std::ios::end);
			return this->file.tellg();
		}else{
			this->file.seekp(0,std::ios::end);
			return this->file.tellp();
		}
#endif
	}
	return 0;
}

void NONS_File::close(){
	if (!this->is_open)
		return;
#if NONS_SYS_WINDOWS
	if (!!*this)
		CloseHandle(this->file);
#elif NONS_SYS_UNIX
	if (!!*this)
		::close(this->file);
#else
	this->file.close();
#endif
	this->is_open=0;
}
	
bool NONS_File::operator!() const{
	return
#if NONS_SYS_WINDOWS
		this->file==INVALID_HANDLE_VALUE;
#elif NONS_SYS_UNIX
		this->file<0;
#else
		!this->file;
#endif
}

bool NONS_File::read(void *dst,size_t read_bytes,size_t &bytes_read,Uint64 offset) const{
	if (!this->is_open || !this->opened_for_read){
		bytes_read=0;
		return 0;
	}
	if (offset>=this->_filesize){
		bytes_read=0;
		return 0;
	}
#if NONS_SYS_WINDOWS
	{
		LARGE_INTEGER temp;
		temp.QuadPart=offset;
		SetFilePointerEx(this->file,temp,0,FILE_BEGIN);
	}
#elif NONS_SYS_UNIX
	lseek64(this->file,offset,SEEK_SET);
#else
	this->file.seekg((size_t)offset);
#endif
	if (this->_filesize-offset<read_bytes)
		read_bytes=size_t(this->_filesize-offset);
	if (dst){
#if NONS_SYS_WINDOWS
		DWORD rb=read_bytes,br=0;
		ReadFile(this->file,dst,rb,&br,0);
#elif NONS_SYS_UNIX
		::read(this->file,dst,read_bytes);
#else
		this->file.read((char *)dst,read_bytes);
#endif
	}
	bytes_read=read_bytes;
	return 1;
}

NONS_File::type *NONS_File::read(size_t read_bytes,size_t &bytes_read,Uint64 offset) const{
	bool a;
	if (!(a=this->read(0,read_bytes,bytes_read,offset)) || !bytes_read){
		bytes_read=0;
		return 0;
	}
	assert(bytes_read>0);
	NONS_File::type *buffer=new NONS_File::type[bytes_read];
	this->read(buffer,read_bytes,bytes_read,offset);
	return buffer;
}

bool NONS_File::write(void *buffer,size_t size,bool write_at_end){
	if (!this->is_open || this->opened_for_read)
		return 0;
#if NONS_SYS_WINDOWS
	if (!write_at_end)
		SetFilePointer(this->file,0,0,FILE_BEGIN);
	else
		SetFilePointer(this->file,0,0,FILE_END);
	DWORD a=size;
	WriteFile(this->file,buffer,a,&a,0);
#elif NONS_SYS_UNIX
	if (!write_at_end)
		lseek64(this->file,0,SEEK_SET);
	else
		lseek64(this->file,0,SEEK_END);
	::write(this->file,buffer,size);
#else
	if (!write_at_end)
		this->file.seekp(0);
	else
		this->file.seekp(0,std::ios::end);
	this->file.write((char *)buffer,size);
#endif
	return 1;
}

NONS_File::type *NONS_File::read(const std::wstring &path,size_t read_bytes,size_t &bytes_read,Uint64 offset){
	NONS_File file(path,1);
	return file.read(read_bytes,bytes_read,offset);
}

NONS_File::type *NONS_File::read(const std::wstring &path,size_t &bytes_read){
	NONS_File file(path,1);
	return file.read(bytes_read);
}

bool NONS_File::write(const std::wstring &path,void *buffer,size_t size){
	NONS_File file(path,0);
	return file.write(buffer,size);
}

bool NONS_File::delete_file(const std::wstring &path){
#if NONS_SYS_WINDOWS
	return !!DeleteFile(&path[0]);
#else
	std::string s=UniToUTF8(path);
	return remove(&s[0])==0;
#endif
}

bool NONS_File::file_exists(const std::wstring &name){
	bool ret;
#if NONS_SYS_WINDOWS
	HANDLE file=CreateFile(&name[0],0,0,0,OPEN_EXISTING,0,0);
	ret=(file!=INVALID_HANDLE_VALUE);
	CloseHandle(file);
#else
	std::ifstream file(UniToUTF8(name).c_str());
	ret=!!file;
	file.close();
#endif
	return ret;
}

bool NONS_File::get_file_size(Uint64 &size,const std::wstring &name){
	NONS_File file(name,1);
	if (!file)
		return 0;
	size=file.filesize();
	return 1;
}

NONS_EventQueue::NONS_EventQueue(){
	InputObserver.attach(this);
}

NONS_EventQueue::~NONS_EventQueue(){
	InputObserver.detach(this);
}

void NONS_EventQueue::push(SDL_Event a){
	NONS_MutexLocker ml(this->mutex);
	this->data.push(a);
}

SDL_Event NONS_EventQueue::pop(){
	NONS_MutexLocker ml(this->mutex);
	SDL_Event ret=this->data.front();
	this->data.pop();
	return ret;
}

bool NONS_EventQueue::emptify(){
	NONS_MutexLocker ml(this->mutex);
	bool ret=0;
	while (!this->data.empty()){
		if (this->data.front().type==SDL_QUIT)
			ret=1;
		this->data.pop();
	}
	return ret;
}

bool NONS_EventQueue::empty(){
	NONS_MutexLocker ml(this->mutex);
	bool ret=this->data.empty();
	return ret;
}

void NONS_EventQueue::WaitForEvent(int delay){
	while (this->data.empty())
		SDL_Delay(delay);
}

NONS_InputObserver::NONS_InputObserver(){
	this->data.reserve(50);
}

void NONS_InputObserver::attach(NONS_EventQueue *what){
	NONS_MutexLocker ml(this->mutex);
	ulong pos=this->data.size();
	for (ulong a=0;a<pos;a++)
		if (!this->data[a])
			pos=a;
	if (pos==this->data.size())
		this->data.push_back(what);
	else
		this->data[pos]=what;
}

void NONS_InputObserver::detach(NONS_EventQueue *what){
	NONS_MutexLocker ml(this->mutex);
	for (ulong a=0;a<this->data.size();a++){
		if (this->data[a]==what){
			this->data[a]=0;
			break;
		}
	}
}

void NONS_InputObserver::notify(SDL_Event *event){
	NONS_MutexLocker ml(this->mutex);
	for (ulong a=0;a<this->data.size();a++)
		if (!!this->data[a])
			this->data[a]->push(*event);
}

void NONS_InputObserver::setup_joysticks(){
	this->free_joysticks();
	int n=SDL_NumJoysticks();
	for (int a=0;a<n;a++){
		SDL_Joystick *j=SDL_JoystickOpen(a);
		if (SDL_JoystickOpened(a) && (SDL_JoystickNumAxes(j)<1 || SDL_JoystickNumButtons(j)<2)){
			SDL_JoystickClose(j);
			j=0;
		}
		this->joysticks.push_back(j);
	}
	SDL_JoystickEventState(SDL_ENABLE);
}

void NONS_InputObserver::free_joysticks(){
	for (size_t a=0;a<this->joysticks.size();a++)
		SDL_JoystickClose(this->joysticks[a]);
	this->joysticks.clear();
}

struct reportedError{
	ErrorCode error;
	long original_line;
	std::string caller;
	std::wstring extraInfo;
	reportedError(ErrorCode error,long original_line,const char *caller,std::wstring &extra){
		this->error=error;
		this->original_line=original_line;
		this->caller=caller;
		this->extraInfo=extra;
	}
	reportedError(const reportedError &b){
		this->error=b.error;
		this->original_line=b.original_line;
		this->caller=b.caller;
		this->extraInfo=b.extraInfo;
	}
};

typedef std::map<Uint32,std::queue<reportedError> > errorManager;

void printError(NONS_RedirectedOutput &stream,ErrorCode error,ulong original_line,const std::string &caller,const std::wstring &extraInfo){
	if (!CHECK_FLAG(error,NONS_NO_ERROR_FLAG)){
		if (caller.size()>0)
			stream <<caller<<"(): ";
		if (CHECK_FLAG(error,NONS_INTERNAL_ERROR))
			stream <<"Internal error. ";
		else{
			if (CHECK_FLAG(error,NONS_FATAL_ERROR))
				stream <<"Fatal error";
			else if (CHECK_FLAG(error,NONS_WARNING))
				stream <<"Warning";
			else
				stream <<"Error";
			if (original_line!=ULONG_MAX)
				stream <<" near line "<<original_line<<". ";
			else
				stream <<". ";
		}
		if (CHECK_FLAG(error,NONS_UNDEFINED_ERROR))
			stream <<"Unspecified error.\n";
		else
			stream <<"("<<ulong(error&0xFFFF)<<") "<<errorMessages[error&0xFFFF]<<"\n";
		if (extraInfo.size()){
			stream.indent(1);
			stream <<"Extra information: "<<extraInfo<<"\n";
			stream.indent(-1);
		}
	}
}

ErrorCode handleErrors(ErrorCode error,ulong original_line,const char *caller,bool queue,std::wstring extraInfo){
	static errorManager manager;
	Uint32 currentThread=SDL_ThreadID();
	errorManager::iterator currentQueue=manager.find(currentThread);
	/*if (error==NONS_END){
		if (currentQueue!=manager.end())
			while (!currentQueue->second.empty())
				currentQueue->second.pop();
		return error;
	}*/
	if (queue){
		(currentQueue!=manager.end()?currentQueue->second:manager[currentThread]).push(reportedError(error,original_line,caller,extraInfo));
		return error;
	}else if (CHECK_FLAG(error,NONS_NO_ERROR_FLAG) && currentQueue!=manager.end()){
		while (!currentQueue->second.empty())
			currentQueue->second.pop();
	}
	if (currentQueue!=manager.end()){
		while (!currentQueue->second.empty() && CHECK_FLAG(currentQueue->second.front().error,NONS_NO_ERROR_FLAG))
			currentQueue->second.pop();
		while (!currentQueue->second.empty()){
			reportedError &topError=currentQueue->second.front();
			printError(o_stderr,topError.error,topError.original_line,topError.caller,topError.extraInfo);
			currentQueue->second.pop();
		}
	}
	printError(o_stderr,error,original_line,caller,extraInfo);
	if (CHECK_FLAG(error,NONS_FATAL_ERROR)){
		o_stderr <<"I'll just go ahead and kill myself.\n";
		exit(error);
	}
	return error;
}

void waitUntilClick(NONS_EventQueue *queue){
	bool detach=!queue;
	if (detach)
		queue=new NONS_EventQueue;
	while (!CURRENTLYSKIPPING){
		SDL_Delay(25);
		while (!queue->empty()){
			SDL_Event event=queue->pop();
			if (event.type==SDL_MOUSEBUTTONDOWN || event.type==SDL_KEYDOWN){
				if (detach)
					delete queue;
				return;
			}
		}
	}
}

void waitCancellable(long delay,NONS_EventQueue *queue){
	bool detach=!queue;
	if (detach)
		queue=new NONS_EventQueue;
	while (delay>0 && !CURRENTLYSKIPPING){
		SDL_Delay(25);
		delay-=25;
		while (!queue->empty()){
			SDL_Event event=queue->pop();
			if (event.type==SDL_MOUSEBUTTONDOWN || event.type==SDL_KEYDOWN /*&& (event.key.keysym.sym==SDLK_LCTRL || event.key.keysym.sym==SDLK_RCTRL)*/){
				delay=0;
				break;
			}
		}
	}
	if (detach)
		delete queue;
}

void waitNonCancellable(long delay){
	while (delay>0 && !CURRENTLYSKIPPING){
		SDL_Delay(10);
		delay-=10;
	}
}

Uint8 getCorrectedMousePosition(NONS_VirtualScreen *screen,int *x,int *y){
	int x0,y0;
	Uint8 r=SDL_GetMouseState(&x0,&y0);
	x0=(int)screen->unconvertX(x0);
	y0=(int)screen->unconvertY(y0);
	*x=x0;
	*y=y0;
	return r;
}

#if NONS_SYS_WINDOWS
VirtualConsole::VirtualConsole(const std::string &name,ulong color){
	this->good=0;
	SECURITY_ATTRIBUTES sa;
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle=1;
	sa.lpSecurityDescriptor=0;
	this->near_end=0;
	this->far_end=0;
	this->process=0;
	if (!CreatePipe(&this->far_end,&this->near_end,&sa,0)){
		assert(this->near_end==INVALID_HANDLE_VALUE);
		return;
	}
	SetHandleInformation(this->near_end,HANDLE_FLAG_INHERIT,0);
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&pi,sizeof(pi));
	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(STARTUPINFO);
	si.hStdInput=this->far_end;
	si.dwFlags|=STARTF_USESTDHANDLES;
	TCHAR program[]=TEXT("console.exe");
	TCHAR arguments[100];
#ifndef UNICODE
	sprintf(arguments,"%d",color);
#else
	swprintf(arguments,L"0 %d",color);
#endif
	if (!CreateProcess(program,arguments,0,0,1,CREATE_NEW_CONSOLE|CREATE_UNICODE_ENVIRONMENT,0,0,&si,&pi))
		return;
	this->process=pi.hProcess;
	CloseHandle(pi.hThread);
	this->good=1;

	this->put(name);
	this->put("\n",1);
}

VirtualConsole::~VirtualConsole(){
	if (this->near_end!=INVALID_HANDLE_VALUE){
		if (this->process!=INVALID_HANDLE_VALUE){
			TerminateProcess(this->process,0);
			CloseHandle(this->process);
		}
		CloseHandle(this->near_end);
		CloseHandle(this->far_end);
	}
}
	
void VirtualConsole::put(const char *str,size_t size){
	if (!this->good)
		return;
	if (!size)
		size=strlen(str);
	DWORD l;
	WriteFile(this->near_end,str,size,&l,0);
}
#endif

#ifndef NONS_NO_STDOUT
NONS_RedirectedOutput::NONS_RedirectedOutput(std::ostream &a)
		:cout(a),file(0){
#else
NONS_RedirectedOutput::NONS_RedirectedOutput(std::ofstream *a)
		:file(a){
#endif
	this->indentation=0;
	this->addIndentationNext=1;
#if NONS_SYS_WINDOWS
	this->vc=0;
#endif
}

NONS_RedirectedOutput::~NONS_RedirectedOutput(){
	if (this->file)
		this->file->close();
#if NONS_SYS_WINDOWS
	if (this->vc)
		delete this->vc;
#endif
}

void NONS_RedirectedOutput::write_to_stream(const std::stringstream &str){
#if NONS_SYS_WINDOWS
	if (CLOptions.verbosity>=VERBOSITY_RESERVED && this->vc)
		this->vc->put(str.str());
	else
#endif
#ifndef NONS_NO_STDOUT
		if (CLOptions.override_stdout && this->file)
#endif


			*this->file <<str.str();
#ifndef NONS_NO_STDOUT
		else
			this->cout <<str.str();
#endif
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(ulong a){
	return *this <<itoac(a);
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(long a){
	return *this <<itoac(a);
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(wchar_t a){
	std::wstring s;
	s.push_back(a);
	return *this <<UniToUTF8(s);
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(double a){
	return *this <<itoac(a);
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(const char *a){
	return *this <<std::string(a);
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(const std::string &a){
	std::stringstream stream;
	for (ulong b=0;b<a.size();b++){
		char c=a[b];
		if (this->addIndentationNext)
			for (ulong d=0;d<this->indentation;d++)
				stream <<INDENTATION_STRING;
		if (c=='\n')
			this->addIndentationNext=1;
		else
			this->addIndentationNext=0;
		stream <<c;
	}
	this->write_to_stream(stream);
	return *this;
}

NONS_RedirectedOutput &NONS_RedirectedOutput::operator<<(const std::wstring &a){
	return *this <<UniToUTF8(a);
}

#ifndef NONS_NO_STDOUT
void NONS_RedirectedOutput::redirect(){
	if (this->file)
		delete this->file;
	const char *str;
	ulong color=7;
	if (this->cout==std::cout)
		str=STDOUT_FILENAME;
	else if (this->cout==std::cerr){
		str=STDERR_FILENAME;
		color=12;
	}else
		str=STDLOG_FILENAME;
#if NONS_SYS_WINDOWS
	if (CLOptions.verbosity>=VERBOSITY_RESERVED){
		this->vc=new VirtualConsole(str,color);
		if (this->vc->good)
			return;
		delete this->vc;
		this->vc=0;
	}
#endif
	if (!CLOptions.override_stdout){
		this->file=0;
		return;
	}
	this->file=new std::ofstream(str,CLOptions.reset_redirection_files?std::ios::trunc:std::ios::app);
	if (!this->file->is_open()){
		delete this->file;
		this->file=0;
	}
	else if (!CLOptions.reset_redirection_files)
		*this->file <<"\n\n"
			"--------------------------------------------------------------------------------\n"
			"Session "<<getTimeString<char>()<<"\n"
			"--------------------------------------------------------------------------------"<<std::endl;
}
#endif

void NONS_RedirectedOutput::indent(long a){
	if (!a)
		return;
	if (a<0){
		if (ulong(-a)>this->indentation)
			this->indentation=0;
		else
			this->indentation-=-a;
	}else
		this->indentation+=a;
}

NONS_DataSource::~NONS_DataSource(){
	while (this->streams.size()){
		delete this->streams.back();
		this->streams.pop_back();
	}
}

NONS_DataStream *NONS_DataSource::open(NONS_DataStream *p,const std::wstring &path){
	p->original_path=path;
	if (CLOptions.verbosity>=VERBOSITY_LOG_OPEN_STREAMS && p->original_path.size())
		o_stderr <<"Opening stream to "<<p->original_path<<"\n";
	this->streams.push_back(p);
	return p;
}

NONS_DataStream *NONS_FileSystem::open(const std::wstring &path,bool keep_in_memory){
	std::wstring name=path;
	toforwardslash(name);
	if (!NONS_File::file_exists(name))
		return 0;
	NONS_DataStream *p;
	if (!keep_in_memory)
		p=new NONS_InputFile(*this,name);
	else{
		NONS_File file(name,1);
		p=new NONS_MemoryFile(*this,file);
	}
	return NONS_DataSource::open(p,normalize_path(p->get_name()));

}

std::wstring get_temp_path(){
	static std::wstring path;
	if (path.size())
		return path;
#if NONS_SYS_WINDOWS
	path.assign(GetTempPath(0,0),0);
	GetTempPath(path.size(),&path[0]);
	path[path.size()-1]='/';
#elif NONS_SYS_UNIX
	path=L"/tmp/";
#else
	path=L"./";
#endif
	return path;
}

std::wstring NONS_FileSystem::new_temp_name(){
	this->mutex.lock();
	ulong id=this->temp_id++;
	this->mutex.unlock();
	return get_temp_path()+L"__ONSlaught_temp_"+itoaw(id)+L".tmp";
}

bool NONS_DataSource::close(NONS_DataStream *p){
	std::list<NONS_DataStream *>::iterator i=std::find(
		this->streams.begin(),
		this->streams.end(),
		p
	);
	if (i==this->streams.end())
		return 0;
	if (CLOptions.verbosity>=VERBOSITY_LOG_OPEN_STREAMS && p->original_path.size())
		o_stderr <<"Closing stream to "<<p->original_path<<"\n";
	delete *i;
	this->streams.erase(i);
	return 1;
}

NONS_DataStream *NONS_FileSystem::new_temporary_file(const std::wstring &path){
	return new NONS_TemporaryFile(*this,path);
}

NONS_DataStream *NONS_MemoryFS::new_temporary_file(const void *src,size_t n){
	return new NONS_MemoryFile(*this,src,n);
}

Uint64 NONS_DataStream::seek(Sint64 offset,int direction){
	switch (direction){
		case -1:
			this->offset=this->size-offset;
			break;
		case 0:
			this->offset+=offset;
			break;
		case 1:
			this->offset=offset;
			break;
		default:
			return 0;
	}
	return this->offset;
}

Uint64 NONS_DataStream::stdio_seek(Sint64 offset,int direction){
	switch (direction){
		case SEEK_CUR:
			direction=0;
			break;
		case SEEK_END:
			direction=-1;
			offset=-offset;
			break;
		case SEEK_SET:
			direction=1;
	}
	return this->seek(offset,direction);
}

SDL_RWops NONS_DataStream::to_rwops(){
	SDL_RWops ops;
	memset(&ops,0,sizeof(ops));
	ops.hidden.unknown.data1=this;
	ops.seek=rw_seek;
	ops.read=rw_read;
	ops.write=rw_write;
	ops.close=rw_close;
	return ops;
}

int SDLCALL NONS_DataStream::rw_seek(SDL_RWops *ops,int offset,int whence){
	return (int)((NONS_DataStream *)ops->hidden.unknown.data1)->stdio_seek(offset,whence);
}

int SDLCALL NONS_DataStream::rw_read(SDL_RWops *ops,void *dst,int a,int b){
	NONS_DataStream *ds=(NONS_DataStream *)ops->hidden.unknown.data1;
	size_t size=a*b;
	if (size<=0)
		return -1;
	if (!ds->read(dst,size,size))
		return -1;
	return size/a;
}

int SDLCALL NONS_DataStream::rw_write(SDL_RWops *,const void *,int,int){
	return -1;
}

int SDLCALL NONS_DataStream::rw_close(SDL_RWops *ops){
	NONS_DataStream *ds=(NONS_DataStream *)ops->hidden.unknown.data1;
	NONS_DataSource *source=ds->source;
	source->close(ds);
	return 0;
}

NONS_InputFile::NONS_InputFile(NONS_DataSource &ds,const std::wstring &name):NONS_DataStream(ds,name),file(name,1){
	this->size=file.filesize();
}

bool NONS_InputFile::read(void *dst,size_t &bytes_read,size_t count){
	if (!this->file.read(dst,count,bytes_read,this->offset))
		return 0;
	this->offset+=bytes_read;
	return 1;
}

NONS_TemporaryFile::~NONS_TemporaryFile(){
	this->file.close();
	NONS_File::delete_file(this->name);
}

NONS_MemoryFile::NONS_MemoryFile(NONS_DataSource &ds,const void *src,size_t n):NONS_DataStream(ds,L""){
	this->data=(uchar *)malloc(n);
	memcpy(this->data,src,n);
	this->size=n;
}

NONS_MemoryFile::NONS_MemoryFile(NONS_DataSource &ds,NONS_File &file):NONS_DataStream(ds,L""){
	if (!file){
		this->data=0;
		this->size=0;
	}
	this->size=(size_t)file.filesize();
	this->data=(uchar *)malloc((size_t)this->size);
	size_t temp=(size_t)this->size;
	file.read(this->data,temp,temp,0);
	this->size=temp;
}

NONS_MemoryFile::~NONS_MemoryFile(){
	free(this->data);
}

bool NONS_MemoryFile::read(void *dst,size_t &bytes_read,size_t count){
	if (!this->data){
		bytes_read=0;
		return 0;
	}
	bytes_read=(count>this->size-this->offset)?(size_t)(this->size-this->offset):count;
	memcpy(dst,this->data+this->offset,bytes_read);
	this->offset+=bytes_read;
	return 1;
}

NONS_Clock::t NONS_Clock::MAX=DBL_MAX;

#if NONS_SYS_WINDOWS
NONS_Clock::NONS_Clock(){
	Uint64 *p=new Uint64;
	LARGE_INTEGER li;
	*p=(!QueryPerformanceFrequency(&li))?0:li.QuadPart/1000;
	this->data=p;
}

NONS_Clock::~NONS_Clock(){
	delete (Uint64 *)this->data;
}
#endif

NONS_Clock::t NONS_Clock::get() const{
#if NONS_SYS_WINDOWS
	const Uint64 *p=(const Uint64 *)this->data;
	if (!*p)
		return SDL_GetTicks();
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return NONS_Clock::t(li.QuadPart)/NONS_Clock::t(*p);
#elif NONS_SYS_UNIX
	timespec ts;
	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&ts)<0)
		return SDL_GetTicks();
	return NONS_Clock::t(ts.tv_sec)*1000.0+NONS_Clock::t(ts.tv_nsec)/1000000.0;
#else
	return SDL_GetTicks();
#endif
}

#if NONS_SYS_WINDOWS
DECLARE_ENUM(WINDOWS_VERSION)
	ERR=0,


	//9x kernel
	V95=1,
	V98=2,
	VME=3,
	//NT kernel
	V2K=4,
	VXP=5,
	VVI=6,
	VW7=7
DECLARE_ENUM_CLOSE;

WINDOWS_VERSION::WINDOWS_VERSION getWindowsVersion(){
	//First try with the 9x kernel
	HKEY k;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\0"),0,KEY_READ,&k)!=ERROR_SUCCESS)
		return WINDOWS_VERSION::ERR;
	DWORD type,size;
	WINDOWS_VERSION::WINDOWS_VERSION ret;
	if (RegQueryValueEx(k,TEXT("Version"),0,&type,0,&size)!=ERROR_SUCCESS || type!=REG_SZ){
		//Not the 9x kernel
		RegCloseKey(k);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),0,KEY_READ,&k)!=ERROR_SUCCESS)
			return WINDOWS_VERSION::ERR;
		if (RegQueryValueEx(k,TEXT("CurrentVersion"),0,&type,0,&size)!=ERROR_SUCCESS || type!=REG_SZ)
			return WINDOWS_VERSION::ERR;
		std::wstring str(size/sizeof(wchar_t),0);
		RegQueryValueEx(k,TEXT("CurrentVersion"),0,&type,(LPBYTE)&str[0],&size);
		RegCloseKey(k);
		str.resize(size/sizeof(wchar_t)-1);
		if (str[0]=='5')
			ret=WINDOWS_VERSION::VXP;
		else if (str==L"6.0")
			ret=WINDOWS_VERSION::VVI;
		else if (str==L"6.1")
			ret=WINDOWS_VERSION::VW7;
		else
			ret=WINDOWS_VERSION::ERR;
	}else{
		std::string str(size,0);
		RegQueryValueEx(k,TEXT("VersionNumber"),0,&type,(LPBYTE)&str[0],&size);
		RegCloseKey(k);
		switch (str[2]){
			case '0':
				ret=WINDOWS_VERSION::V95;
				break;
			case '1':
				ret=WINDOWS_VERSION::V98;
				break;
			case '9':
				ret=WINDOWS_VERSION::VME;
				break;
			default:
				ret=WINDOWS_VERSION::ERR;
		}
	}
	return ret;
}
#endif

std::wstring save_directory;
std::wstring config_directory;
const wchar_t *settings_filename=L"settings.cfg";

tm *getDate(const std::wstring &filename){
	tm *res=new tm();
#if NONS_SYS_WINDOWS
	FILETIME time;
	SYSTEMTIME time2;
#ifdef UNICODE
	HANDLE h=CreateFile(filename.c_str(),FILE_READ_DATA,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
#else
	HANDLE h=CreateFile(UniToISO88591(filename).c_str(),FILE_READ_DATA,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
#endif
	GetFileTime(h,0,0,&time);
	CloseHandle(h);
	FileTimeToSystemTime((const FILETIME *)&time,&time2);
	SystemTimeToTzSpecificLocalTime(0,&time2,&time2);
	res->tm_year=time2.wYear-1900;
	res->tm_mon=time2.wMonth-1;
	res->tm_mday=time2.wDay;
	res->tm_hour=time2.wHour;
	res->tm_min=time2.wMinute;
	res->tm_sec=time2.wSecond;

	res->tm_year=2000;
	res->tm_mon=0;
	res->tm_mday=1;
	res->tm_hour=0;
	res->tm_min=0;
	res->tm_sec=0;
#endif
	return res;
}

std::vector<tm *> existing_files(const std::wstring &location){
	std::vector<tm *> res;
	res.reserve(20);
	std::wstring path=location;
	toforwardslash(path);
	if (path[path.size()-1]!='/')
		path.push_back('/');
	for (short a=1;a<21;a++){
		std::wstring filename=path+L"save"+itoaw(a)+L".dat";
		if (!NONS_File::file_exists(filename))
			res.push_back(0);
		else
			res.push_back(getDate(filename));
	}
	return res;
}

std::wstring getConfigLocation(){
/*#if NONS_SYS_WINDOWS
	if (getWindowsVersion()<WINDOWS_VERSION::V2K)
		return L"./";
	HKEY k;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0,KEY_READ,&k)!=ERROR_SUCCESS)
		return L"./";
	DWORD type,size;
	if (RegQueryValueEx(k,L"Personal",0,&type,0,&size)!=ERROR_SUCCESS || type!=REG_SZ){
		RegCloseKey(k);
		return L"./";
	}
	std::wstring path(size/sizeof(TCHAR),0);
	RegQueryValueEx(k,L"Personal",0,&type,(LPBYTE)&path[0],&size);
	RegCloseKey(k);
	path.resize(wcslen(path.c_str()));
	toforwardslash(path);
	if (path[path.size()-1]!='/')
		path.append(L"/.ONSlaught");
	else
		path.append(L".ONSlaught");
	if (!CreateDirectory((LPCTSTR)path.c_str(),0) && GetLastError()!=ERROR_ALREADY_EXISTS){
		return L"./";
	}
	path.push_back('/');
	return path;
#elif NONS_SYS_UNIX
	passwd* pwd=getpwuid(getuid());
	if (!pwd)
		return L"./";
	std::string res=pwd->pw_dir;
	if (res[res.size()-1]!='/')
		res.append("/.ONSlaught");
	else
		res.append(".ONSlaught");
	if (mkdir(res.c_str(),~0) && errno!=EEXIST){
		return L"./";
	}
	res.push_back('/');
	return UniFromUTF8(res);
#else*/
	return L"./";
//#endif
}

std::wstring getSaveLocation(unsigned hash[5]){
#if NONS_SYS_WINDOWS
	if (getWindowsVersion()<WINDOWS_VERSION::V2K)
		return L"./";
#endif
	std::wstring root=config_directory;
#if NONS_SYS_WINDOWS
#ifdef UNICODE
	std::wstring path=root;
#else
	std::string path=UniToISO88591(root);
#endif
#elif NONS_SYS_UNIX
	std::wstring path=root;
#else
	return root;
#endif
#if NONS_SYS_WINDOWS || NONS_SYS_UNIX
	if (!CLOptions.savedir.size()){
		path.append(L"savedata");
	}else
		path.append(CLOptions.savedir);
#endif
#if NONS_SYS_WINDOWS
	if (!CreateDirectory((LPCTSTR)path.c_str(),0) && GetLastError()!=ERROR_ALREADY_EXISTS)
		return root;
	path.push_back('/');
	return path;
#elif NONS_SYS_UNIX
	if (mkdir(UniToUTF8(path).c_str(),~0) && errno!=EEXIST)
		return root;
	path.push_back('/');
	return path;
#endif
}
