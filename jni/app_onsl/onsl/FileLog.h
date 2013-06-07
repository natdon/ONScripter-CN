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

#ifndef NONS_FILELOG_H
#define NONS_FILELOG_H

#include "Common.h"
#include "Functions.h"
#include <set>

typedef std::set<std::wstring,stdStringCmpCI<wchar_t> > logSet_t;

struct NONS_LogStrings{
	std::wstring saveAs;
	logSet_t log;
	bool commit;
	NONS_LogStrings():commit(0){}
	NONS_LogStrings(const std::wstring &oldName,const std::wstring &newName);
	~NONS_LogStrings();
	void init(const std::wstring &oldName,const std::wstring &newName);
	void writeOut();
	virtual bool addString(const std::wstring &string);
	virtual bool check(const std::wstring &string);
};

struct NONS_FileLog:NONS_LogStrings{
	NONS_FileLog(const std::wstring &oldName,const std::wstring &newName)
		:NONS_LogStrings(oldName,newName){}
	bool addString(const std::wstring &string);
	bool check(const std::wstring &string);
};

struct NONS_LabelLog:NONS_LogStrings{
	NONS_LabelLog()
		:NONS_LogStrings(){}
	NONS_LabelLog(const std::wstring &oldName,const std::wstring &newName)
		:NONS_LogStrings(oldName,newName){}
	bool addString(const std::wstring &string);
	bool check(const std::wstring &string);
};
#endif
