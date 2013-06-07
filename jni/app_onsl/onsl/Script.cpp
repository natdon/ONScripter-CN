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

#include "Script.h"
#include "IOFunctions.h"
#include "sha1.h"
#include "ScriptInterpreter.h"
#include "ExpressionParser.tab.hpp"
#include <iostream>

//Returns then position of the ending quote, of npos if couldn't be found before
//the end of the string or if there isn't a string beginning at 'start'.
size_t findEndOfString(const std::wstring &str,size_t offset){
	if (offset+1>=str.size())
		return str.npos;
	if (str[offset]=='\"' || str[offset]=='`'){
		wchar_t terminator=str[offset++];
		return str.find(terminator,offset);
	}else if (offset+1<str.size() && NONS_tolower(str[offset])=='e' && str[offset+1]=='\"')
		offset+=2;
	else
		return str.npos;
	ulong size=str.size();
	while (offset<size && str[offset]!='\"'){
		if (str[offset]=='\\'){
			offset++;
			if (offset>=size)
				return str.npos;
			if (str[offset]=='x'){
				offset++;
				for (ulong a=0;offset<size && str[offset]!='\"' && a<4;a++,offset++);
			}else
				offset++;
		}else
			offset++;
	}
	return (offset<size)?offset:str.npos;
}

std::wstring readIdentifierAndAdvance(const std::wstring &str,ulong offset){
	if (str[offset]!='_' && !NONS_isalpha(str[offset]))
		return L"";
	ulong start=offset,
		end=start+1;
	for (;end<str.size() && (str[end]=='_' || NONS_isalnum(str[end]));end++);
	return std::wstring(str,start,end-start);
}

NONS_Statement::NONS_Statement(const std::wstring &string,NONS_ScriptLine *line,ulong number,ulong offset,bool terminal){
	this->stmt=string;
	this->lineOfOrigin=line;
	this->fileOffset=offset;
	this->statementNo=number;
	this->parsed=0;
	this->type=StatementType::EMPTY;
	this->error=NONS_NO_ERROR;
	if (string.size()){
		if (multicomparison(string[0],";*~`\\@!#%$?[") || string[0]>0x7F){
			switch (string[0]){
				case ';':
					this->type=StatementType::COMMENT;
					return;
				case '*':
					this->type=StatementType::BLOCK;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
					this->commandName=string.substr(1);
#else
					this->commandName=this->stmt.substr(1);
#endif
					trim_string(this->commandName);
					break;
				case '~':
					this->type=StatementType::JUMP;
					return;
				default:
					this->type=StatementType::PRINTER;
			}
		}else
			this->type=StatementType::COMMAND;
	}
}

NONS_Statement::NONS_Statement(const NONS_Statement &copy,ulong No,NONS_ScriptLine *newLOO){
	*this=copy;
	this->lineOfOrigin=newLOO;
	this->statementNo=No;
}

NONS_Statement &NONS_Statement::operator=(const NONS_Statement &copy){
	this->stmt=copy.stmt;
	this->commandName=copy.commandName;
	this->stringParameters=copy.stringParameters;
	this->parameters=copy.parameters;
	this->type=copy.type;
	this->error=copy.error;
	this->lineOfOrigin=0;
	this->statementNo=0;
	this->parsed=copy.parsed;
	return *this;
}

void NONS_Statement::parse(NONS_Script *script){
	if (!this->parsed && this->type==StatementType::COMMAND){
		this->parsed=1;
		std::wstring &string=this->stmt;
		ulong size=string.size(),
			space=0;
		for (;space<size && !iswhitespace(string[space]);space++);
		this->commandName=string.substr(0,space);
		if (!isValidIdentifier(this->commandName)){
			this->commandName.clear();
			this->error=NONS_INVALID_COMMAND_NAME;
			return;
		}
		for (;space<size && iswhitespace(string[space]);space++);
		this->stringParameters=string.substr(space);
		if (!stdStrCmpCI(this->commandName,L"if") || !stdStrCmpCI(this->commandName,L"notif")){
			this->preparseIf(script);
		}else if (!stdStrCmpCI(this->commandName,L"for")){
			this->preparseFor(script);
		}else if (!stdStrCmpCI(this->commandName,L"case")){
			this->preparseCase(script);
		}else if (!stdStrCmpCI(this->commandName,L"default")){
			this->preparseDefault(script);
		}else{
			ulong start=space,
				end;
			while (start<size){
				end=start;
				while (end<size && string[end]!=','){
					if (string[end]=='\"' || string[end]=='`' || end+1<size && NONS_tolower(string[end])=='e' && string[end+1]=='\"'){
						end=findEndOfString(string,end);
						if (end==string.npos){
							this->error=NONS_UNMATCHED_QUOTES;
							return;
						}
					}
					for (end++;end<size && iswhitespace(string[end]);end++);
				}
				if (end>size)
					end=size;
				ulong start0=end;
				for (end--;end>start && iswhitespace(string[end]);end--);
				end++;
				this->parameters.push_back(std::wstring(string,start,end-start));
				start=start0;
				if (start<size)
					start++;
				for (;start<size && iswhitespace(string[start]);start++);
			}
		}
	}
}

void NONS_Statement::preparseIf(NONS_Script *script){
	std::wstringstream stream(this->stringParameters);
	PreParserData ppd;
	ppd.mode=1;
	ppd.trigger=1;
	NONS_Expression::Expression *p;
	if (!expressionParser_yyparse(&stream,gScriptInterpreter->store,p,&ppd)){
		NONS_Expression::ExpressionCompiler c(ppd.res[0]);
		this->parameters.push_back(c.unparse());
		this->parameters.push_back(this->stringParameters.substr(ppd.then_position));
	}else
		this->error=NONS_UNDEFINED_SYNTAX_ERROR;
}

void NONS_Statement::preparseFor(NONS_Script *script){
	std::wstringstream stream(this->stringParameters);
	PreParserData ppd;
	ppd.mode=2;
	ppd.trigger=1;
	NONS_Expression::Expression *p;
	if (!expressionParser_yyparse(&stream,gScriptInterpreter->store,p,&ppd)){
		for (size_t a=0;a<ppd.res.size();a++){
			NONS_Expression::ExpressionCompiler c(ppd.res[a]);
			this->parameters.push_back(c.unparse());
		}
	}else
		this->error=NONS_UNDEFINED_SYNTAX_ERROR;
}

NONS_ScriptLine::NONS_ScriptLine(ulong line,const std::wstring &string,ulong off,bool ignoreEmptyStatements){
	this->lineNumber=line;
	this->linesSpanned=1+std::count(string.begin(),string.end(),10);
	ulong start=string.find_first_not_of(L"\x09\x20");
	if (start==string.npos)
		start=0;
	off+=start;
	std::wstring temp=string.substr(start);
	wchar_t *C_temp=&temp[0];
	std::wstring temp2;
	for (ulong a=0,size=string.size();a<size;){
		bool terminal=0;
		ulong original_a=a;
		if (multicomparison(temp[a],";~`?%$!\\@#[") || temp[a]>0x7F || firstcharsCI(temp,a,L"if") || firstcharsCI(temp,a,L"notif") || firstcharsCI(temp,a,L"default") || firstcharsCI(temp,a,L"case") ){
			temp2=std::wstring(temp,a);
			terminal=1;
			a=size;
		}else{
			ulong b=a;
			while (b<size){
				if (C_temp[b]=='\"' || C_temp[b]=='`' || b+1<size && NONS_tolower(C_temp[b])=='e' && C_temp[b+1]=='\"')
					b=findEndOfString(temp,b);
				else if (C_temp[b]==':' || C_temp[b]==';')
					break;
				if (b!=temp.npos)
					b++;
			}
			if (b>=size)
				b=size;
			ulong c=b-1;
			for (;C_temp[c]<128 && iswhitespace((char)C_temp[c]);c--);
			c++;
			temp2=std::wstring(temp,a,c-a);
			a=b;
			if (C_temp[a]==':')
				for (a++;a<size && C_temp[c]<128 && iswhitespace((char)C_temp[c]);a--);
		}
		NONS_Statement *stmt=new NONS_Statement(temp2,this,this->statements.size(),off+original_a,terminal);
		if (ignoreEmptyStatements && (
				stmt->type==StatementType::EMPTY ||
				stmt->type==StatementType::COMMENT ||
				stmt->type==StatementType::JUMP))
			delete stmt;
		else
			this->statements.push_back(stmt);
	}
}

NONS_ScriptLine::NONS_ScriptLine(const NONS_ScriptLine &copy,ulong startAt){
	this->linesSpanned=copy.linesSpanned;
	if (!startAt)
		*this=copy;
	else{
		this->lineNumber=copy.lineNumber;
		for (ulong a=0;a<this->statements.size();a++)
			delete this->statements[a];
		this->statements.resize(copy.statements.size()-startAt);
		for (ulong a=startAt;a<copy.statements.size();a++)
			this->statements[a-startAt]=new NONS_Statement(*copy.statements[a],a-startAt,this);
	}
}

NONS_ScriptLine &NONS_ScriptLine::operator=(const NONS_ScriptLine &copy){
	this->lineNumber=copy.lineNumber;
	for (ulong a=0;a<this->statements.size();a++)
		delete this->statements[a];
	this->statements.resize(copy.statements.size());
	for (ulong a=0;a<this->statements.size();a++)
		this->statements[a]=new NONS_Statement(*copy.statements[a],a,this);
	this->linesSpanned=copy.linesSpanned;
	return *this;
}

NONS_ScriptLine::~NONS_ScriptLine(){
	for (ulong a=0;a<this->statements.size();a++)
		delete this->statements[a];
}

std::wstring NONS_ScriptLine::toString(){
	if (!this->statements.size())
		return L"";
	std::wstring res=this->statements[0]->stmt;
	for (ulong a=1;a<this->statements.size();a++){
		res.push_back(':');
		res.append(this->statements[a]->stmt);
	}
	return res;
}

#ifndef NONS_LOW_MEMORY_ENVIRONMENT
NONS_ScriptBlock::NONS_ScriptBlock(const std::wstring &name,const wchar_t *buffer,ulong start,ulong end,ulong line_start,ulong line_end){
	this->data=std::wstring(buffer+start,end-start+1);
#else
NONS_ScriptBlock::NONS_ScriptBlock(const std::wstring &name,NONS_Script *script,ulong start,ulong end,ulong line_start,ulong line_end){
	this->script=script;
#endif
	this->name=name;
	this->first_offset=start;
	this->last_offset=end;
	this->first_line=line_start;
	this->last_line=line_end;
}

#ifdef NONS_LOW_MEMORY_ENVIRONMENT
std::string NONS_ScriptBlock::get_data() const{
	return this->script->get_string(this->first_offset,this->last_offset-this->first_offset+1);
}
#endif

NONS_Script::NONS_Script(){
	memset(this->hash,0,sizeof(unsigned)*5);
}

extern std::wstring save_directory;
long start,end2;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
ErrorCode NONS_Script::init(const std::wstring &scriptname,ENCODING::ENCODING encoding,ENCRYPTION::ENCRYPTION encryption){
	std::wstring wtemp;start=SDL_GetTicks();
	{
		std::vector<uchar> temp;
		{
			NONS_DataStream *stream=general_archive.open(scriptname);
			if (!stream)
				return NONS_FILE_NOT_FOUND;
			stream->read_all(temp);
			general_archive.close(stream);
		}
		{
			if (encryption == ENCRYPTION::NT3_KEY) {
				temp.erase(temp.begin(),temp.begin()+2332);
				ErrorCode error_code=NT3Decryption(&temp[0],temp.size());
				temp.erase(temp.begin(),temp.begin()+4);
			}else{
				ErrorCode error_code=inPlaceDecryption(&temp[0],temp.size(),encryption);
				if (error_code!=NONS_NO_ERROR)
				return error_code;
			} 

		}
		switch (encoding){
			case ENCODING::AUTO:
				if (isValidUTF8(&temp[0],temp.size())){
					o_stderr <<"The script seems to be a valid UTF-8 stream. Using it as such.\n";
					wtemp=UniFromUTF8(temp);
				}else if (isValidSJIS(&temp[0],temp.size())){
					o_stderr <<"The script seems to be a valid Shift JIS stream. Using it as such.\n";
					wtemp=UniFromSJIS(temp);
				}else{
					o_stderr <<"The script seems to be a valid ISO-8859-1 stream. Using it as such.\n";
					wtemp=UniFromISO88591(temp);
				}
				break;
			case ENCODING::ISO_8859_1:
				wtemp=UniFromISO88591(temp);
				break;
			case ENCODING::UTF8:
				wtemp=UniFromUTF8(temp);
				break;
			case ENCODING::SJIS:
				wtemp=UniFromSJIS(temp);
				break;
			default:
				break;
		}
	}
	this->scriptSize=wtemp.size();
	wchar_t *buffer=&wtemp[0];
	ulong currentLine=1,
		start_of_block_offset=0,
		start_of_block_line=currentLine;
	std::wstring block_name=NONS_FIRST_BLOCK;
	std::set<std::wstring,stdStringCmpCI<wchar_t> > *checkDuplicates=new std::set<std::wstring,stdStringCmpCI<wchar_t> >;
	ErrorCode error=NONS_NO_ERROR;
	for (ulong a=0,size=wtemp.size();a<size;){
		this->lineOffsets.push_back(a);
		ulong start_of_line=wtemp.find_first_not_of(L"\x09\x20",a),
			end_of_line=wtemp.find_first_of(L"\x0A\x0D",a),
			currentLineCopy=currentLine;
		if (end_of_line==wtemp.npos)
			end_of_line=wtemp.size();
		if (start_of_line!=end_of_line){
			while (buffer[end_of_line-1]=='/' && end_of_line<size-1){
				a=end_of_line;
				if (buffer[a]==10)
					a++;
				else if (a+1<wtemp.size() && buffer[a+1]==10)
					a+=2;
				else
					a++;
				this->lineOffsets.push_back(a);
				currentLine++;
				end_of_line=wtemp.find_first_of(L"\x0A\x0D",a);
				if (end_of_line==wtemp.npos)
					end_of_line=size-1;
			}
			if (buffer[start_of_line]=='*'){
				ulong beg=wtemp.find_first_not_of(WCS_WHITESPACE,start_of_line+1);
				ulong len=wtemp.find_first_of(WCS_WHITESPACE,beg);
				if (len!=wtemp.npos)
					len-=beg;
				std::wstring id(wtemp,beg,len);
				id=string_replace<wchar_t>(id,L"/\x0D\x0A",0);
				id=string_replace<wchar_t>(id,L"/\x0D",0);
				id=string_replace<wchar_t>(id,L"/\x0A",0);
				if (checkDuplicates->find(id)!=checkDuplicates->end()){
					handleErrors(NONS_DUPLICATE_LABEL,currentLine,"NONS_Script::init",0);
					error=NONS_FATAL_ERROR;
				}
				if (isValidLabel(id)){
					if (start_of_line)
						this->blocksByLine.push_back(new NONS_ScriptBlock(
							block_name,
							buffer,
							start_of_block_offset,
							start_of_line-1,
							start_of_block_line,
							currentLine-1));
					start_of_block_offset=start_of_line;
					start_of_block_line=currentLine;
					block_name=id;
					checkDuplicates->insert(id);
				}else
					handleErrors(NONS_INVALID_ID_NAME,currentLineCopy,"NONS_Script::init",0,L"The label will be ignored");
			}else if (buffer[start_of_line]=='~')
				this->jumps.push_back(std::pair<ulong,ulong>(currentLineCopy,start_of_line));
		}
		a=end_of_line;
		if (buffer[a]==10)
			a++;
		else if (a+1<wtemp.size() && buffer[a+1]==10)
			a+=2;
		else
			a++;
		currentLine++;
	}
	delete checkDuplicates;
	if (!CHECK_FLAG(error,NONS_NO_ERROR_FLAG))
		return error;
	this->blocksByLine.push_back(new NONS_ScriptBlock(
		block_name,
		buffer,
		start_of_block_offset,
		wtemp.size()-1,
		start_of_block_line,
		currentLine));
	wtemp.clear();
	this->blocksByName.assign(this->blocksByLine.begin(),this->blocksByLine.end());
	std::sort(this->blocksByName.begin(),this->blocksByName.end(),sortBlocksByName);
	if (!this->blockFromLabel(L"define"))
		return NONS_NO_DEFINE_LABEL;
	SHA1 hash;
	for (ulong a=0;a<this->blocksByLine.size();a++){
		std::wstring &b=this->blocksByLine[a]->name;
		std::vector<char> temp2;
		temp2.resize(b.size());
		for (size_t c=0;c<b.size();c++)
			temp2[c]=(char)b[c];
		hash.Input(&temp2[0],temp2.size());
	}
	hash.Result(this->hash);
	save_directory=getSaveLocation(this->hash);
	end2=SDL_GetTicks()-start;
	o_stderr<<"load script:"<<end2<<"ms\n";
	return NONS_NO_ERROR;
}
#else
ErrorCode NONS_Script::init(const std::wstring &scriptname,ENCODING::ENCODING encoding,ENCRYPTION::ENCRYPTION encryption){
	NONS_Clock clock;
	if (encoding!=ENCODING::UTF8 || encryption!=ENCRYPTION::NONE){
		this->cache_filename=CACHE_FILENAME;
		std::vector<uchar> buffer;
		{
			NONS_DataStream *stream=general_archive.open(scriptname);
			if (!stream)
				return NONS_FILE_NOT_FOUND;
			stream->read_all(buffer);
			general_archive.close(stream);
		}
		{
			ErrorCode error_code=inPlaceDecryption(&buffer[0],buffer.size(),encryption);
			if (error_code!=NONS_NO_ERROR)
				return error_code;
		}
		if (encoding==ENCODING::AUTO){
			if (isValidUTF8(&buffer[0],buffer.size())){
				o_stderr <<"The script seems to be a valid UTF-8 stream. Using it as such.\n";
				encoding=ENCODING::UTF8;
			}else if (isValidSJIS(&buffer[0],buffer.size())){
				o_stderr <<"The script seems to be a valid Shift JIS stream. Using it as such.\n";
				encoding=ENCODING::SJIS;
			}else{
				o_stderr <<"The script seems to be a valid ISO-8859-1 stream. Using it as such.\n";
				encoding=ENCODING::ISO_8859_1;
			}
		}

		{
			NONS_Clock::t t0,t1;
			t0=clock.get();
			NONS_File cache(this->cache_filename,0);
			ulong advance;
			std::string obuffer;
			obuffer.reserve(4099);
			for (ulong a=0;a<buffer.size();a+=advance){
				wchar_t c;
				if (!ConvertSingleCharacter(c,&buffer[a],buffer.size()-a,advance,encoding))
					continue;
				ulong bytes;
				char utf8[4];
				ConvertSingleCharacterToUTF8(utf8,c,bytes);
				obuffer.append(utf8,bytes);
				if (obuffer.size()>=4096){
					cache.write(&obuffer[0],obuffer.size());
					obuffer.clear();
				}
			}
			if (obuffer.size())
				cache.write(&obuffer[0],obuffer.size());
			t1=clock.get();
			STD_COUT <<"Script converted in "<<t1-t0<<" ms.\n";
		}
	}else
		this->cache_filename=scriptname;
	NONS_Clock::t t0,t1;
	t0=clock.get();
	{
		std::string buffer;
		{
			NONS_File file(this->cache_filename,1);
			this->scriptSize=(size_t)file.filesize();
			buffer.resize(this->scriptSize);
			file.read(&buffer[0],this->scriptSize,this->scriptSize,0);
			buffer.resize(this->scriptSize);
		}

		ulong currentLine=1,
			start_of_block_offset=0,
			start_of_block_line=currentLine;
		std::wstring block_name=NONS_FIRST_BLOCK;
		std::set<std::wstring,stdStringCmpCI<wchar_t> > *checkDuplicates=new std::set<std::wstring,stdStringCmpCI<wchar_t> >;
		ErrorCode error=NONS_NO_ERROR;
		char *char_buffer=(char *)&buffer[0];
		for (ulong a=0,size=buffer.size();a<size;){
			this->lineOffsets.push_back(a);
			ulong start_of_line=buffer.find_first_not_of("\x09\x20",a),
				end_of_line=buffer.find_first_of("\x0A\x0D",a),
				currentLineCopy=currentLine;
			if (end_of_line==buffer.npos)
				end_of_line=buffer.size();
			if (start_of_line!=end_of_line){
				while (char_buffer[end_of_line-1]=='/' && end_of_line<size-1){
					a=end_of_line;
					if (char_buffer[a]==10)
						a++;
					else if (a+1<buffer.size() && char_buffer[a+1]==10)
						a+=2;
					else
						a++;
					this->lineOffsets.push_back(a);
					currentLine++;
					end_of_line=buffer.find_first_of("\x0A\x0D",a);
					if (end_of_line==buffer.npos)
						end_of_line=size-1;
				}
				if (char_buffer[start_of_line]=='*'){
					ulong beg=buffer.find_first_not_of(STR_WHITESPACE,start_of_line+1);
					ulong len=buffer.find_first_of(STR_WHITESPACE,beg);
					if (len!=buffer.npos)
						len-=beg;
					std::wstring id=UniFromUTF8(buffer.substr(beg,len));
					id=string_replace<wchar_t>(id,L"/\x0D\x0A",0);
					id=string_replace<wchar_t>(id,L"/\x0D",0);
					id=string_replace<wchar_t>(id,L"/\x0A",0);
					if (checkDuplicates->find(id)!=checkDuplicates->end()){
						handleErrors(NONS_DUPLICATE_LABEL,currentLine,"NONS_Script::init",0);
						error=NONS_FATAL_ERROR;
					}
					if (isValidLabel(id)){
						if (start_of_line)
							this->blocksByLine.push_back(new NONS_ScriptBlock(
								block_name,
								this,
								start_of_block_offset,
								start_of_line-1,
								start_of_block_line,
								currentLine-1));
						start_of_block_offset=start_of_line;
						start_of_block_line=currentLine;
						block_name=id;
						checkDuplicates->insert(id);
					}else
						handleErrors(NONS_INVALID_ID_NAME,currentLineCopy,"NONS_Script::init",0,L"The label will be ignored");
				}else if (char_buffer[start_of_line]=='~')
					this->jumps.push_back(std::pair<ulong,ulong>(currentLineCopy,start_of_line));
			}
			a=end_of_line;
			if (char_buffer[a]==10)
				a++;
			else if (a+1<buffer.size() && char_buffer[a+1]==10)
				a+=2;
			else
				a++;
			currentLine++;
		}
		delete checkDuplicates;
		if (!CHECK_FLAG(error,NONS_NO_ERROR_FLAG))
			return error;
		this->blocksByLine.push_back(new NONS_ScriptBlock(
			block_name,
			this,
			start_of_block_offset,
			this->scriptSize-1,
			start_of_block_line,
			currentLine));
	}
	this->blocksByName.assign(this->blocksByLine.begin(),this->blocksByLine.end());
	std::sort(this->blocksByName.begin(),this->blocksByName.end(),sortBlocksByName);
	if (!this->blockFromLabel(L"define"))
		return NONS_NO_DEFINE_LABEL;
	SHA1 hash;
	for (ulong a=0;a<this->blocksByLine.size();a++){
		std::wstring &b=this->blocksByLine[a]->name;
		std::vector<char> temp2;
		temp2.resize(b.size());
		for (size_t c=0;c<b.size();c++)
			temp2[c]=(char)b[c];
		hash.Input(&temp2[0],temp2.size());
	}
	hash.Result(this->hash);
	save_directory=getSaveLocation(this->hash);
	t1=clock.get();
	STD_COUT <<"Script data loaded in "<<t1-t0<<" ms.\n";
	return NONS_NO_ERROR;
}
#endif

NONS_Script::~NONS_Script(){
	for (ulong a=0;a<this->blocksByLine.size();a++)
		delete this->blocksByLine[a];
#ifdef NONS_LOW_MEMORY_ENVIRONMENT
	if (this->cache_filename==CACHE_FILENAME)
		NONS_File::delete_file(this->cache_filename);
#endif
}

NONS_ScriptBlock *NONS_Script::blockFromLabel(std::wstring name){
	if (name[0]=='*')
		name=name.substr(1);
	trim_string(name);
	size_t off;
	if (!this->blocksByName.size() || !binary_search<NONS_ScriptBlock *,std::wstring>(
			&this->blocksByName[0],
			0,
			this->blocksByName.size()-1,
			name,
			off,
			&findBlocksByName))
		return 0;
	return this->blocksByName[off];
}

NONS_ScriptBlock *NONS_Script::blockFromOffset(ulong offset){
	size_t off;
	if (!this->blocksByName.size() || !binary_search<NONS_ScriptBlock *,ulong>(
			&this->blocksByLine[0],
			0,
			this->blocksByLine.size()-1,
			offset,
			off,
			&findBlocksByOffset))
		return 0;
	return this->blocksByLine[off];
}


std::wstring NONS_Script::labelFromOffset(ulong offset){
	size_t off;
	if (!this->blocksByLine.size() || !binary_search<NONS_ScriptBlock *,ulong>(
			&this->blocksByLine[0],
			0,
			this->blocksByLine.size()-1,
			offset,
			off,
			&findBlocksByOffset))
		return L"";
	return this->blocksByName[off]->name;
}

NONS_ScriptBlock *NONS_Script::blockFromLine(ulong line){
	size_t off;
	if (!this->blocksByLine.size() || !binary_search<NONS_ScriptBlock *,ulong>(
			&this->blocksByLine[0],
			0,
			this->blocksByLine.size()-1,
			line,
			off,
			&findBlocksByLine))
		return 0;
	return this->blocksByLine[off];
}

size_t NONS_Script::blockIndexFromLine(ulong line){
	size_t off;
	if (!this->blocksByLine.size() || !binary_search<NONS_ScriptBlock *,ulong>(
			&this->blocksByLine[0],
			0,
			this->blocksByLine.size()-1,
			line,
			off,
			&findBlocksByLine))
		off=this->blocksByLine.size();
	return off;
}

size_t NONS_Script::jumpIndexForBackwards(ulong offset){
	ulong size=this->jumps.size();
	for (long a=size-1;a>=0;a--)
		if (this->jumps[a].second<=offset)
			return a;
	return size;
}

size_t NONS_Script::jumpIndexForForward(ulong offset){
	ulong size=this->jumps.size();
	for (ulong a=0;a<size;a++)
		if (this->jumps[a].second>=offset)
			return a;
	return size;
}

ulong NONS_Script::offsetFromLine(ulong line){
	if (line>this->lineOffsets.size())
		return this->scriptSize;
	return this->lineOffsets[line-1];
}

#ifdef NONS_LOW_MEMORY_ENVIRONMENT
std::string NONS_Script::get_string(ulong offset,ulong size){
 	if (offset+size>this->scriptSize)
 		size=this->scriptSize-offset;
	std::string res(size,0);
 	std::ifstream file(CACHE_FILENAME,std::ios::binary);
	file.seekg(offset);
	file.read(&res[0],size);
 	return res;
}
#endif

NONS_ScriptThread::NONS_ScriptThread(NONS_Script *script){
	this->script=script;
	this->valid=0;
	if (!this->script->blocksByLine.size())
		return;
	for (ulong a=0,size=this->script->blocksByLine.size();a<size && !this->readBlock(*this->script->blocksByLine[a]);a++);
	if (!this->lines.size())
		return;
	this->currentLine=0;
	this->currentStatement=0;
	std::pair<ulong,ulong> pair=this->getNextStatementPair();
	this->nextLine=pair.first;
	this->nextStatement=pair.second;
}

NONS_ScriptThread::~NONS_ScriptThread(){
	for (ulong a=0;a<this->lines.size();a++)
		delete this->lines[a];
}

bool NONS_ScriptThread::advanceToNextStatement(){
	if (this->nextLine==this->lines[this->currentLine]->lineNumber)
		this->currentStatement=this->nextStatement;
	else if (this->nextLine>=this->first_line && this->nextLine<=this->last_line){
		long moveTo=-1;
		if (this->lines[this->currentLine]->lineNumber<=this->nextLine){
			for (ulong a=this->currentLine;a<this->lines.size() && moveTo<0;a++)
				if (this->nextLine<=this->lines[a]->lineNumber)
					moveTo=a;
		}else{
			for (ulong a=0;a<this->lines.size() && moveTo<0;a++)
				if (this->nextLine<=this->lines[a]->lineNumber)
					moveTo=a;
		}
		if (moveTo<0){
			this->nextLine=this->last_line+1;
			return this->advanceToNextStatement();
		}
		this->currentLine=moveTo;
		this->currentStatement=0;
	}else{
		NONS_ScriptBlock *block=this->script->blockFromLine(this->nextLine);
		if (!block)
			return 0;
		ulong offset=this->script->offsetFromLine(this->nextLine),
			line=this->nextLine-block->first_line;
		if (offset==this->script->scriptSize)
			return 0;
		if (block->first_offset>offset)
			offset=0;
		else
			offset-=block->first_offset;
		if (!this->readBlock(*block,offset,line)){
			size_t index=this->script->blockIndexFromLine(this->nextLine);
			offset=0;
			line=0;
			for (ulong a=index,size=this->script->blocksByLine.size();a<size && !this->readBlock(*block,offset,line);a++);
			if (!this->lines.size())
				return 0;
		}
		this->currentLine=0;
		this->currentStatement=0;
	}
	std::pair<ulong,ulong> pair=this->getNextStatementPair();
	this->nextLine=pair.first;
	this->nextStatement=pair.second;
	return 1;
}

bool NONS_ScriptThread::gotoLabel(const std::wstring &label){
	NONS_ScriptBlock *block=this->script->blockFromLabel(label);
	if (!block)
		return 0;
	this->nextLine=block->first_line;
	this->nextStatement=0;
	return 1;
}

bool NONS_ScriptThread::skip(long offset){
	ulong line=this->lines[this->currentLine]->lineNumber;
	if (!offset){
		this->nextLine=line;
		this->nextStatement=0;
		return 1;
	}
	line+=offset;
	if (this->script->blocksByLine.back()->last_line<line)
		return 0;
	this->nextLine=line;
	this->nextStatement=0;
	return 1;
}

std::pair<ulong,ulong> NONS_ScriptThread::getNextStatementPair(){
	std::pair<ulong,ulong> pair;
	if (this->lines[this->currentLine]->statements.size()<=this->currentStatement+1){
		pair.first=this->lines[this->currentLine]->lineNumber+this->lines[this->currentLine]->linesSpanned;
		pair.second=0;
	}else{
		pair.first=this->lines[this->currentLine]->lineNumber;
		pair.second=this->currentStatement+1;
	}
	return pair;
}

bool NONS_ScriptThread::gotoPair(const std::pair<ulong,ulong> &to){
	this->nextLine=to.first;
	this->nextStatement=to.second;
	return 1;
}

bool NONS_ScriptThread::gotoJumpBackwards(ulong offset){
	size_t index=this->script->jumpIndexForBackwards(offset);
	if (index==this->script->jumps.size())
		return 0;
	this->nextLine=this->script->jumps[index].first;
	this->nextStatement=0;
	return 1;
}

bool NONS_ScriptThread::gotoJumpForward(ulong offset){
	size_t index=this->script->jumpIndexForForward(offset);
	if (index==this->script->jumps.size())
		return 0;
	this->nextLine=this->script->jumps[index].first;
	this->nextStatement=0;
	return 1;
}

bool NONS_ScriptThread::readBlock(const NONS_ScriptBlock &block,ulong start_at_offset,ulong start_at_line){
	const ulong lines_limit=100;

	for (ulong a=0;a<this->lines.size();a++)
		delete this->lines[a];
	this->lines.clear();
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
	if (start_at_offset>=block.data.size())
#else
	if (start_at_offset>=block.last_offset-block.first_offset+1)
#endif
		return 0;
	this->currentBlock=&block;

	this->first_line=block.first_line+start_at_line;
	this->first_offset=block.first_offset+start_at_offset;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
	const std::wstring &txt=block.data;
#else
	std::string txt=block.get_data();
#endif
	ulong lineNo=block.first_line+start_at_line;
	ulong a=start_at_offset;
	for (ulong size=txt.size();a<size && this->lines.size()<lines_limit;){
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
		ulong start_of_line=txt.find_first_not_of(L"\x09\x20",a),
			end_of_line=txt.find_first_of(L"\x0A\x0D",a),
#else
		ulong start_of_line=txt.find_first_not_of("\x09\x20",a),
			end_of_line=txt.find_first_of("\x0A\x0D",a),
#endif
			lineNo0=lineNo;
		if (start_of_line==txt.npos)
			break;
		if (end_of_line==txt.npos)
			end_of_line=txt.size();
		if (start_of_line!=end_of_line){
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
			std::wstring lineCopy=txt.substr(start_of_line,end_of_line-start_of_line);
#else
			std::string lineCopy=txt.substr(start_of_line,end_of_line-start_of_line);
#endif
readBlock_000:
			while (txt[end_of_line-1]=='/' && end_of_line<size-1){
				lineCopy.resize(lineCopy.size()-1);
				a=end_of_line;
				if (txt[a]==10)
					a++;
				else if (a+1<size && txt[a+1]==10)
					a+=2;
				else
					a++;
				lineNo++;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
				end_of_line=txt.find_first_of(L"\x0A\x0D",a);
#else
				end_of_line=txt.find_first_of("\x0A\x0D",a);
#endif
				if (end_of_line==txt.npos)
					end_of_line=size-1;
				lineCopy.append(txt,a,end_of_line-a);
			}
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
			NONS_ScriptLine *line=new NONS_ScriptLine(lineNo0,lineCopy,block.first_offset+a,1);
#else
			NONS_ScriptLine *line=new NONS_ScriptLine(lineNo0,UniFromUTF8(lineCopy),block.first_offset+a,1);
#endif
			if (line->statements.size()){
				if (line->statements.back()->type==StatementType::COMMAND &&
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
						lineCopy[lineCopy.find_last_not_of(WCS_WHITESPACE)]==','){
#else
						lineCopy[find_last_not_of_in_utf8(lineCopy,WCS_NON_NEWLINE_WHITESPACE)]==','){
#endif
					delete line;

					while (1){
						lineCopy.push_back(10);
						a=end_of_line;
						if (txt[a]==10)
							a++;
						else if (a+1<size && txt[a+1]==10)
							a+=2;
						else
							a++;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
						a=txt.find_first_not_of(WCS_NON_NEWLINE_WHITESPACE,a);
#else
						a=find_first_not_of_in_utf8(txt,WCS_NON_NEWLINE_WHITESPACE,a);
#endif
						lineNo++;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
						end_of_line=txt.find_first_of(L"\x0A\x0D",a);
#else
						end_of_line=txt.find_first_of("\x0A\x0D",a);
#endif
						if (end_of_line!=a)
							break;
					}
					if (end_of_line==txt.npos)
						end_of_line=size-1;
					lineCopy.append(txt,a,end_of_line-a);


					goto readBlock_000;
				}
				this->lines.push_back(line);
			}else
				delete line;
		}
		a=end_of_line;
		if (txt[a]==10)
			a++;
		else if (a+1<size && txt[a+1]==10)
			a+=2;
		else
			a++;
		lineNo++;
	}
	this->last_line=lineNo-1;
	this->last_offset=a-1;
	return !!this->lines.size();
}

void NONS_Statement::preparseCase(NONS_Script *script){
	 	std::wstring &string=this->stringParameters;
	 	ulong size=string.size(),start=0,end=0;
	 	for(;end<size && !((string[end])=='(');end++);
	 	start=++end;
	 	ulong count=1;
	 	for(;end<size;end++){
			if (string[end]=='(') count++;
			if (string[end]==')') count--;
			if (!count) break;
	 	}
	 	if (count) { //it is an error
			this->parameters.push_back(L"NONS_UNDEFINED_SYNTAX_ERROR");
		} else {
			this->parameters.push_back(std::wstring(string,start,end-start));
			end++;
		 	for(;end<size && iswhitespace(string[end]);end++);
		 	this->parameters.push_back(std::wstring(string,end,size-end));
	 	}
}
	 	
	 	
void NONS_Statement::preparseDefault(NONS_Script *script){
	 	std::wstring &string=this->stringParameters;
	 	ulong size=string.size();
		if (size>0)
		 	this->parameters.push_back(std::wstring(string,0,size));
}
