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

#include "INIfile.h"
#include "INIParser.tab.hpp"
#include "Archive.h"

/*void INIvalue::setIntValue(long a){
	this->value=itoaw(a);
}*/

INIsection::INIsection(const std::map<std::wstring,std::wstring> &vars){
	for (std::map<std::wstring,std::wstring>::const_iterator i=vars.begin(),end=vars.end();i!=end;i++)
		this->variables[i->first]=INIvalue(i->second);
}

void INIsection::setIntValue(const std::wstring &index,long a){
	INIvalue *v=this->getValue(index);
	if (!v)
		this->variables[index]=INIvalue(a);
	else
		v->setIntValue(a);
}

void INIsection::setStrValue(const std::wstring &index,const std::wstring &a){
	INIvalue *v=this->getValue(index);
	if (!v)
		this->variables[index]=INIvalue(a);
	else
		v->setStrValue(a);
}

long INIsection::getIntValue(const std::wstring &index){
	INIvalue *v=this->getValue(index);
	if (!v)
		return 0;
	return v->getIntValue();
}

const std::wstring &INIsection::getStrValue(const std::wstring &index){
	INIvalue *v=this->getValue(index);
	if (!v){
		this->setStrValue(index,L"");
		return this->getValue(index)->getStrValue();
	}
	return v->getStrValue();
}

INIvalue *INIsection::getValue(const std::wstring &a){
	std::map<std::wstring,INIvalue>::iterator i=this->variables.find(a);
	if (i==this->variables.end())
		return 0;
	return &i->second;
}

INIfile::INIfile(){}

INIfile::INIfile(const std::wstring &filename,ENCODING::ENCODING encoding){
	NONS_DataStream *stream=general_archive.open(filename);
	if (!stream)
		return;
	std::vector<uchar> buffer;
	stream->read_all(buffer);
	general_archive.close(stream);
	this->readFile(buffer,encoding);
}

INIfile::INIfile(std::vector<uchar> &buffer,ENCODING::ENCODING encoding){
	this->readFile(buffer,encoding);
}

void INIfile::readFile(std::vector<uchar> &buffer,ENCODING::ENCODING encoding){
	std::wstring buffer2;
	switch (encoding){
		case ENCODING::ISO_8859_1:
			buffer2=UniFromISO88591(buffer);
			break;
		case ENCODING::SJIS:
			buffer2=UniFromSJIS(buffer);
			break;
		case ENCODING::UTF8:
			buffer2=UniFromUTF8(buffer);
		//Shut GCC up.
		default:;
	}
	std::wstringstream stream;
	stream <<'\0'<<buffer2;
	std::map<std::wstring,std::map<std::wstring,std::wstring> > *map;
	if (!INIParser_yyparse(stream,map)){
		for (std::map<std::wstring,std::map<std::wstring,std::wstring> >::iterator i=map->begin(),end=map->end();i!=end;i++)
			this->sections[i->first]=INIsection(i->second);
		delete map;
	}
}

INIsection *INIfile::getSection(const std::wstring &index){
	std::map<std::wstring,INIsection>::iterator i=this->sections.find(index);
	if (i==this->sections.end())
		return 0;
	return &i->second;
}
