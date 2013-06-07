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

#ifndef NONS_IOFUNCTIONS_H
#define NONS_IOFUNCTIONS_H

#include "Common.h"
#include "Functions.h"
#include "ErrorCodes.h"
#include "VirtualScreen.h"
#include "ThreadManager.h"
#include "SDLhead.h"
#include <string>
#include <fstream>
#include <queue>
#include <list>

class NONS_File{
#if NONS_SYS_WINDOWS
	void *
#elif NONS_SYS_UNIX
	int
#else
	std::fstream
#endif
		file;
	bool opened_for_read;
	bool is_open;
	Uint64 _filesize;
	NONS_File(const NONS_File &){}
	const NONS_File &operator=(const NONS_File &){ return *this; }
	Uint64 reload_filesize();
public:
	typedef unsigned char type;
	NONS_File():is_open(0){}
	NONS_File(const std::wstring &path,bool open_for_read);
	~NONS_File(){ this->close(); }
	void open(const std::wstring &path,bool open_for_read);
	void close();
	bool operator!() const;
	bool read(void *dst,size_t read_bytes,size_t &bytes_read,Uint64 offset) const;
	bool read(void *dst,size_t &bytes_read){ return this->read(dst,(size_t)this->_filesize,bytes_read,0); }
	type *read(size_t read_bytes,size_t &bytes_read,Uint64 offset) const;
	type *read(size_t &bytes_read){ return this->read((size_t)this->_filesize,bytes_read,0); }
	bool write(void *buffer,size_t size,bool write_at_end=1);
	Uint64 filesize(){ return (this->opened_for_read)?this->_filesize:this->reload_filesize(); }
	static type *read(const std::wstring &path,size_t read_bytes,size_t &bytes_read,Uint64 offset);
	static type *read(const std::wstring &path,size_t &bytes_read);
	static bool write(const std::wstring &path,void *buffer,size_t size);
	static bool delete_file(const std::wstring &path);
	static bool file_exists(const std::wstring &name);
	static bool get_file_size(Uint64 &size,const std::wstring &name);
};

class NONS_EventQueue{
	std::queue<SDL_Event> data;
	NONS_Mutex mutex;
public:
	NONS_EventQueue();
	~NONS_EventQueue();
	void push(SDL_Event a);
	SDL_Event pop();
	//returns true if SDL_QUIT was found in the queue
	bool emptify();
	bool empty();
	void WaitForEvent(int delay=100);
};

struct NONS_InputObserver{
	std::vector<SDL_Joystick *> joysticks;
	std::vector<NONS_EventQueue *> data;
	NONS_Mutex mutex;
	NONS_InputObserver();
	void attach(NONS_EventQueue *what);
	void detach(NONS_EventQueue *what);
	void notify(SDL_Event *event);
	void setup_joysticks();
	void free_joysticks();
};

ErrorCode handleErrors(ErrorCode error,ulong original_line,const char *caller,bool queue,std::wstring extraInfo=L"");
void waitUntilClick(NONS_EventQueue *queue=0);
void waitCancellable(long delay,NONS_EventQueue *queue=0);
void waitNonCancellable(long delay);
Uint8 getCorrectedMousePosition(NONS_VirtualScreen *screen,int *x,int *y);
inline std::ostream &operator<<(std::ostream &stream,const std::wstring &str){
	return stream<<UniToUTF8(str);
}

struct NONS_CommandLineOptions;
extern NONS_CommandLineOptions CLOptions;

#if NONS_SYS_WINDOWS
class VirtualConsole{
	HANDLE near_end,
		far_end,
		process;
public:
	bool good;
	VirtualConsole(const std::string &name,ulong color);
	~VirtualConsole();
	void put(const char *str,size_t size=0);
	void put(const std::string &str){
		this->put(str.c_str(),str.size());
	}
};
#endif

#define INDENTATION_STRING "    "

struct NONS_RedirectedOutput{
	std::ofstream *file;
#ifndef NONS_NO_STDOUT
	std::ostream &cout;
#endif
#if NONS_SYS_WINDOWS
	VirtualConsole *vc;
#endif
	ulong indentation;
	bool addIndentationNext;
#ifndef NONS_NO_STDOUT
	NONS_RedirectedOutput(std::ostream &a);
#else
	NONS_RedirectedOutput(std::ofstream *a);
#endif
	~NONS_RedirectedOutput();
	NONS_RedirectedOutput &operator<<(ulong);
	NONS_RedirectedOutput &operator<<(long);
	NONS_RedirectedOutput &operator<<(wchar_t);
	NONS_RedirectedOutput &operator<<(double);
	NONS_RedirectedOutput &operator<<(const char *);
	NONS_RedirectedOutput &operator<<(const std::string &);
	NONS_RedirectedOutput &operator<<(const std::wstring &);
#ifndef NONS_NO_STDOUT
	void redirect();
#endif
	void indent(long);
private:
	void write_to_stream(const std::stringstream &str);
};

extern NONS_InputObserver InputObserver;
extern NONS_RedirectedOutput o_stdout;
extern NONS_RedirectedOutput o_stderr;
//extern NONS_RedirectedOutput o_stdlog;

class NONS_DataStream;

class NONS_DataSource{
	std::list<NONS_DataStream *> streams;
public:
	virtual ~NONS_DataSource();
	virtual NONS_DataStream *open(const std::wstring &name,bool keep_in_memory)=0;
	NONS_DataStream *open(NONS_DataStream *p,const std::wstring &path);
	virtual bool close(NONS_DataStream *stream);
	virtual bool get_size(Uint64 &size,const std::wstring &name)=0;
	virtual bool read(void *dst,size_t &bytes_read,NONS_DataStream &stream,size_t count)=0;
	virtual uchar *read_all(const std::wstring &name,size_t &bytes_read)=0;
	virtual bool exists(const std::wstring &name)=0;
};

std::wstring get_temp_path();

class NONS_FileSystem:public NONS_DataSource{
	NONS_Mutex mutex;
	ulong temp_id;
public:
	NONS_FileSystem():temp_id(0){}
	NONS_DataStream *open(const std::wstring &name,bool keep_in_memory);
	NONS_DataStream *new_temporary_file(const std::wstring &path);
	bool get_size(Uint64 &size,const std::wstring &name){
		return NONS_File::get_file_size(size,name);
	}
	bool read(void *dst,size_t &bytes_read,NONS_DataStream &stream,size_t count){
		return 1;
	}
	uchar *read_all(const std::wstring &name,size_t &bytes_read){
		return (uchar *)NONS_File::read(name,bytes_read);
	}
	bool exists(const std::wstring &path){
		std::wstring name=path;
		toforwardslash(name);
		return NONS_File::file_exists(name);
	}
	std::wstring new_temp_name();
};

class NONS_MemoryFS:public NONS_DataSource{
	NONS_Mutex mutex;
public:
	NONS_DataStream *open(const std::wstring &name,bool){
		return 0;
	}
	NONS_DataStream *new_temporary_file(const void *src,size_t n);
	bool get_size(Uint64 &size,const std::wstring &name){
		return 0;
	}
	bool read(void *dst,size_t &bytes_read,NONS_DataStream &stream,size_t count){
		return 0;
	}
	uchar *read_all(const std::wstring &name,size_t &bytes_read){
		return 0;
	}
	bool exists(const std::wstring &name){
		return 0;
	}
};

extern NONS_FileSystem filesystem;
extern NONS_MemoryFS memoryFS;

class NONS_DataStream{
protected:
	NONS_DataSource *source;
	std::wstring name;
	Uint64 offset,
		size;
public:
	std::wstring original_path;
	NONS_DataStream(NONS_DataSource &ds,const std::wstring &name):source(&ds),name(name),offset(0){}
	virtual ~NONS_DataStream(){}
	virtual bool read(void *dst,size_t &bytes_read,size_t count)=0;
	virtual Uint64 seek(Sint64 offset,int direction);
	Uint64 stdio_seek(Sint64 offset,int direction);
	void reset(){ this->seek(0,1); }
	template <typename T>
	void read_all(T &dst){
		this->reset();
		size_t size=(size_t)this->size;
		dst.resize(size);
		this->read(&dst[0],size,size);
		dst.resize(size);
	}
	Uint64 get_size() const{ return this->size; }
	Uint64 get_offset() const{ return this->offset; }
	const std::wstring &get_name() const{ return this->name; }

	SDL_RWops to_rwops();
	static int SDLCALL rw_seek(SDL_RWops *,int,int);
	static int SDLCALL rw_read(SDL_RWops *,void *,int,int);
	static int SDLCALL rw_write(SDL_RWops *,const void *,int,int);
	static int SDLCALL rw_close(SDL_RWops *);
};

class NONS_InputFile:public NONS_DataStream{
protected:
	NONS_File file;
public:
	NONS_InputFile(NONS_DataSource &ds,const std::wstring &name);
	virtual ~NONS_InputFile(){}
	bool read(void *dst,size_t &bytes_read,size_t count);
};

class NONS_TemporaryFile:public NONS_InputFile{
public:
	NONS_TemporaryFile(NONS_DataSource &ds,const std::wstring &name):NONS_InputFile(ds,name){}
	~NONS_TemporaryFile();
};

class NONS_MemoryFile:public NONS_DataStream{
protected:
	uchar *data;
public:
	NONS_MemoryFile(NONS_DataSource &ds,const void *src,size_t n);
	NONS_MemoryFile(NONS_DataSource &ds,NONS_File &file);
	~NONS_MemoryFile();
	bool read(void *dst,size_t &bytes_read,size_t count);
};

class NONS_DECLSPEC NONS_Clock{
#if NONS_SYS_WINDOWS
	void *data;
#endif
public:
	typedef double t;
	static t MAX;
#if NONS_SYS_WINDOWS
	NONS_Clock();
	~NONS_Clock();
#endif
	t get() const;
	t get(double m) const{ return this->get()*m; }
};

std::vector<tm *> existing_files(const std::wstring &location=L"./");
std::wstring getConfigLocation();
std::wstring getSaveLocation(unsigned hash[5]);
tm *getDate(const std::wstring &filename);

extern std::wstring save_directory;
extern std::wstring config_directory;
extern const wchar_t *settings_filename;
#endif
