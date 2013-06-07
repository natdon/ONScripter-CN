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

#ifndef NONS_SCRIPT_H
#define NONS_SCRIPT_H

#include "Common.h"
#include "ErrorCodes.h"
#include "Functions.h"
#include "enums.h"
#include "Archive.h"
#include <vector>
#include <map>

struct NONS_ScriptLine;
struct NONS_Script;

DECLARE_ENUM(StatementType)
	EMPTY,
	COMMENT,
	BLOCK,
	JUMP,
	PRINTER,
	COMMAND,
	INVALID
DECLARE_ENUM_CLOSE;

struct NONS_Statement{
	std::wstring stmt,
		commandName,
		stringParameters;
	std::vector<std::wstring> parameters;
	StatementType::StatementType type;
	ErrorCode error;
	NONS_ScriptLine *lineOfOrigin;
	ulong fileOffset,
		statementNo;
	NONS_Statement(const std::wstring &string,NONS_ScriptLine *line,ulong number,ulong offset,bool terminal=0);
	NONS_Statement(const NONS_Statement &copy,ulong No=0,NONS_ScriptLine *newLOO=0);
	NONS_Statement &operator=(const NONS_Statement &copy);
	void parse(NONS_Script *script);
	void preparseCase(NONS_Script *script);
	void preparseDefault(NONS_Script *script);
private:
	bool parsed;
	void preparseIf(NONS_Script *script);
	void preparseFor(NONS_Script *script);
};

struct NONS_ScriptLine{
	ulong lineNumber,
		linesSpanned;
	std::vector<NONS_Statement *> statements;
	NONS_ScriptLine():linesSpanned(1){}
	NONS_ScriptLine(ulong line,const std::wstring &string,ulong off,bool ignoreEmptyStatements);
	NONS_ScriptLine(const NONS_ScriptLine &copy,ulong startAt=0);
	NONS_ScriptLine &operator=(const NONS_ScriptLine &copy);
	~NONS_ScriptLine();
	std::wstring toString();
};

struct NONS_ScriptBlock{
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
	std::wstring name,
		data;
#else
	NONS_Script *script;
	std::wstring name;
#endif
	ulong first_offset,
		last_offset,
		first_line,
		last_line;
	bool used;
#ifndef NONS_LOW_MEMORY_ENVIRONMENT
	NONS_ScriptBlock(const std::wstring &name,const wchar_t *buffer,ulong start,ulong end,ulong line_start,ulong line_end);
#else
	NONS_ScriptBlock(const std::wstring &name,NONS_Script *script,ulong start,ulong end,ulong line_start,ulong line_end);
	std::string get_data() const;
#endif
};

inline bool sortBlocksByName(const NONS_ScriptBlock *a,const NONS_ScriptBlock *b){
	return stdStrCmpCI(a->name,b->name)<0;
}

inline int findBlocksByName(const std::wstring &a,NONS_ScriptBlock * const &b){
	return stdStrCmpCI(a,b->name);
}

inline int findBlocksByOffset(const ulong &a,NONS_ScriptBlock * const &b){
	ulong start=b->first_offset,
		end=b->last_offset;
	if (a>=start && a<=end)
		return 0;
	if (a<start)
		return -1;
	return 1;
}

inline int findBlocksByLine(const ulong &a,NONS_ScriptBlock * const &b){
	ulong start=b->first_line,
		end=b->last_line;
	if (a>=start && a<=end)
		return 0;
	if (a<start)
		return -1;
	return 1;
}

#define NONS_FIRST_BLOCK L"0BOF0"
#ifdef NONS_LOW_MEMORY_ENVIRONMENT
#define CACHE_FILENAME L"script.cache"
#endif
struct NONS_Script{
#ifdef NONS_LOW_MEMORY_ENVIRONMENT
	std::wstring cache_filename;
#endif
	size_t scriptSize;
	std::vector<NONS_ScriptBlock *> blocksByLine,
		blocksByName;
	//First: line No. of the jump. Second: file offset of the jump.
	std::vector<std::pair<ulong,ulong> > jumps;
	std::vector<ulong> lineOffsets;
	unsigned hash[5];
	NONS_Script();
	~NONS_Script();
	ErrorCode init(const std::wstring &scriptname,ENCODING::ENCODING encoding,ENCRYPTION::ENCRYPTION encryption);
	NONS_ScriptBlock *blockFromLabel(std::wstring name);
	NONS_ScriptBlock *blockFromOffset(ulong offset);
	NONS_ScriptBlock *blockFromLine(ulong line);
	size_t blockIndexFromLine(ulong line);
	std::wstring labelFromOffset(ulong offset);
	size_t jumpIndexForBackwards(ulong offset);
	size_t jumpIndexForForward(ulong offset);
	ulong offsetFromLine(ulong line);
#ifdef NONS_LOW_MEMORY_ENVIRONMENT
	std::string get_string(ulong offset,ulong size);
#endif
};

struct NONS_ScriptThread{
	NONS_Script *script;
	const NONS_ScriptBlock *currentBlock;
	ulong currentLine,
		currentStatement,
		nextLine,
		nextStatement,
		first_offset,
		last_offset,
		first_line,
		last_line;
	std::vector<NONS_ScriptLine *> lines;
	bool valid;
	NONS_ScriptThread(NONS_Script *script);
	~NONS_ScriptThread();
	//0 if can't advance, 1 otherwise.
	bool advanceToNextStatement();
	bool gotoLabel(const std::wstring &label);
	bool skip(long offset);
	NONS_Statement *getCurrentStatement(){
		if (this->currentLine>=this->lines.size())
			return 0;
		NONS_ScriptLine *line=this->lines[this->currentLine];
		if (this->currentStatement>=line->statements.size())
			return 0;
		return line->statements[this->currentStatement];
	}
	bool gotoPair(const std::pair<ulong,ulong> &to);
	bool gotoJumpBackwards(ulong offset);
	bool gotoJumpForward(ulong offset);
	std::pair<ulong,ulong> getNextStatementPair();
private:
	//start_at_offset: offset from the start of block->data.
	//block->first_offset+start_at_offset is the real offset from BOF.
	//The same applies for start_at_line.
	bool readBlock(const NONS_ScriptBlock &block,ulong start_at_offset=0,ulong start_at_line=0);
};
#endif
