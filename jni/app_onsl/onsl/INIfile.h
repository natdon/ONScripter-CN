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

#ifndef NONS_INIsection_H
#define NONS_INIsection_H

#include "Common.h"
#include "Functions.h"
#include "ErrorCodes.h"
#include "enums.h"
#include <map>

class INIvalue{
	std::wstring value;
public:
	INIvalue(){}
	INIvalue(long a){
		this->setIntValue(a);
	}
	INIvalue(const std::wstring &a){
		this->setStrValue(a);
	}
	void setIntValue(long a){
		this->value=itoaw(a);
	}
	void setStrValue(const std::wstring &a){
		this->value=a;
	}
	long getIntValue(){
		return atol(this->value);
	}
	const std::wstring &getStrValue(){
		return this->value;
	}
};

class INIsection{
	std::map<std::wstring,INIvalue> variables;
public:
	INIsection(){}
	INIsection(const std::map<std::wstring,std::wstring> &vars);
	void setIntValue(const std::wstring &index,long a);
	void setStrValue(const std::wstring &index,const std::wstring &a);
	long getIntValue(const std::wstring &index);
	const std::wstring &getStrValue(const std::wstring &index);
	INIvalue *getValue(const std::wstring &index);
};

class INIfile{
	std::map<std::wstring,INIsection> sections;
	void readFile(std::vector<uchar> &buffer,ENCODING::ENCODING encoding=ENCODING::ISO_8859_1);
public:
	INIfile();
	INIfile(const std::wstring &filename,ENCODING::ENCODING encoding=ENCODING::ISO_8859_1);
	INIfile(std::vector<uchar> &buffer,ENCODING::ENCODING encoding=ENCODING::ISO_8859_1);
	INIsection *getSection(const std::wstring &index);
};
#endif
