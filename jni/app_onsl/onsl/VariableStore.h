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

#ifndef NONS_VARIABLESTORE_H
#define NONS_VARIABLESTORE_H

#include "Common.h"
#include "ErrorCodes.h"
#include "FileLog.h"
#include "ExpressionParser.tab.hpp"
#include "tinyxml/tinyxml.h"
#include <vector>
#include <map>
#include <set>

struct PreParserData{
	ulong mode;
	bool trigger;
	std::vector<NONS_Expression::Expression *> res;
	ulong then_position;
};

struct NONS_VariableStore;
class NONS_VariableMember;

namespace NONS_Expression{
	struct Value{
		enum ValueType{
			INVALID,
			INTEGER,
			STRING
		} type;
		long integer;
		std::wstring string;
		bool negated;
		ErrorCode error;

		Value();
		Value(long i);
		Value(const std::wstring &s);
		Value(NONS_VariableMember *member);
		void negate(bool invert_terms){
			if (this->type!=INTEGER || this->negated || !invert_terms)
				return;
			this->integer=!this->integer;
			this->negated=1;
		}
		bool is_err(){ return this->type==INVALID; }
		bool is_int(){ return this->type==INTEGER; }
		bool is_str(){ return this->type==STRING; }
	};

#define NONS_Expression_operator_params std::vector<Value *> &operands,NONS_VariableStore *store,bool invert_terms
	typedef Value *(*operator_f)(NONS_Expression_operator_params);

#define NONS_Expression_DECLARE_OPERATOR(id) Value *id(NONS_Expression_operator_params)
	NONS_Expression_DECLARE_OPERATOR(eval);
	NONS_Expression_DECLARE_OPERATOR(atoi);
	NONS_Expression_DECLARE_OPERATOR(itoa);
	NONS_Expression_DECLARE_OPERATOR(concat);
	NONS_Expression_DECLARE_OPERATOR(array_indexing);
	NONS_Expression_DECLARE_OPERATOR(add);
	NONS_Expression_DECLARE_OPERATOR(sub);
	NONS_Expression_DECLARE_OPERATOR(mul);
	NONS_Expression_DECLARE_OPERATOR(div);
	NONS_Expression_DECLARE_OPERATOR(and_operator);
	NONS_Expression_DECLARE_OPERATOR(or_operator);
	NONS_Expression_DECLARE_OPERATOR(neg);
	NONS_Expression_DECLARE_OPERATOR(integer_dereference);
	NONS_Expression_DECLARE_OPERATOR(string_dereference);
	NONS_Expression_DECLARE_OPERATOR(equals);
	NONS_Expression_DECLARE_OPERATOR(nequals);
	NONS_Expression_DECLARE_OPERATOR(lower);
	NONS_Expression_DECLARE_OPERATOR(greatereq);
	NONS_Expression_DECLARE_OPERATOR(greater);
	NONS_Expression_DECLARE_OPERATOR(lowereq);
	NONS_Expression_DECLARE_OPERATOR(fchk);
	NONS_Expression_DECLARE_OPERATOR(lchk);

	struct Expression{
		std::vector<Expression *> operands;
		operator_f op;
		Value *val;
		Expression(operator_f op,...);
		Expression(Value *val);
		~Expression();
		void vectorize(std::vector<Expression *> &dst);
	};

	class ExpressionCompiler{
		ErrorCode error;
		Expression *expr;
		ErrorCode run(NONS_VariableStore *,bool,const std::vector<Expression *> &,std::vector<Value *> &,size_t);
	public:
		ExpressionCompiler(const std::wstring &exp,NONS_VariableStore *store);
		ExpressionCompiler(Expression *expr);
		~ExpressionCompiler();
		Value *evaluate(NONS_VariableStore *store,bool invert_terms);
		NONS_VariableMember *retrieve(NONS_VariableStore *store);
		ErrorCode getArrayDeclaration(std::vector<long> &dst,NONS_VariableStore *store);
		ErrorCode getError(){ return this->error; }
		std::wstring unparse();
	};
};

class NONS_VariableMember{
	long intValue;
	std::wstring wcsValue;
	bool constant;
	yytokentype type;
	long _long_upper_limit;
	long _long_lower_limit;
	const static std::wstring null;
	void set_default_limits(){
		this->_long_upper_limit=LONG_MAX;
		this->_long_lower_limit=LONG_MIN;
	}
public:
	NONS_VariableMember **dimension;
	ulong dimensionSize;
	NONS_VariableMember(yytokentype type);
	NONS_VariableMember(long value);
	NONS_VariableMember(const std::wstring &a);
	//Assumes: All dimensions have a non-negative size.
	NONS_VariableMember(std::vector<long> &sizes,size_t startAt);
	NONS_VariableMember(const NONS_VariableMember &b);
	NONS_VariableMember(TiXmlElement *);
	~NONS_VariableMember();
	void makeConstant();
	bool isConstant();
	yytokentype getType();
	long getInt();
	const std::wstring &getWcs();
	NONS_VariableMember *getIndex(ulong i);
	void set(long a);
	void atoi(const std::wstring &a);
	void set(const std::wstring &a);
	void inc();
	void dec();
	void add(long a);
	void sub(long a);
	void mul(long a);
	void div(long a);
	void mod(long a);
	void setlimits(long lower,long upper);
	TiXmlElement *save(int index);
private:
	void fixint();
};

struct NONS_Variable{
	NONS_VariableMember *intValue;
	NONS_VariableMember *wcsValue;
	NONS_Variable();
	NONS_Variable(const NONS_Variable &b);
	NONS_Variable &operator=(const NONS_Variable &b);
	NONS_Variable(TiXmlElement *);
	~NONS_Variable();
	TiXmlElement *save(int index);
};

#define VARIABLE_HAS_NO_DATA(x) (!(x) || !(x)->intValue->getInt() && !(x)->wcsValue->getWcs().size())

extern NONS_LabelLog labellog;

typedef std::map<std::wstring,NONS_VariableMember *,stdStringCmpCI<wchar_t> > constants_map_T;
typedef std::map<Sint32,NONS_Variable *> variables_map_T;
typedef std::map<Sint32,NONS_VariableMember *> arrays_map_T;

struct NONS_VariableStore{
	static const Sint32 indexLowerLimit=-0x40000000,
		indexUpperLimit=0x3FFFFFFF;

	constants_map_T constants;
	variables_map_T variables;
	arrays_map_T arrays;
	bool commitGlobals;
	NONS_VariableStore();
	~NONS_VariableStore();
	//Destroys all variables and arrays. Use with care!
	void reset();
	void saveData();
	TiXmlElement *save_locals();
	void load_locals(TiXmlElement *);
	/*
	exp: (IN) The expression to be evaluated.
	invert_terms: (IN) Pass 1 if the expression comes from command_notif and
		we're using the old style evaluator. This parameter is designed to be
		used with simple expressions (e.g. * [&& * [&& * [&& ...]]], * being
		comparisons). Using it with more complex expressions will have weirder
		and weirder results as the expression gets more complex.
	Returns: A complex object that includes every possible type of the
		expression. That is, integer, string, or invalid.
	*/
	NONS_Expression::Value *evaluate(const std::wstring &exp,bool invert_terms);
	/*
	dst: (OUT) dst[0] is the "name" the new array should have, dst[ [1;n) ] are
		the sizes of each of the dimensions.
	exp: (IN) The expression to be evaluated.
	*/
	ErrorCode array_declaration(std::vector<long> &dst,const std::wstring &exp);
	void push(Sint32 pos,NONS_Variable *var);
	NONS_VariableMember *retrieve(const std::wstring &name,ErrorCode *error);
	NONS_Variable *retrieve(Sint32 position,ErrorCode *error);
	NONS_VariableMember *getConstant(const std::wstring &name);
	NONS_VariableMember *getArray(Sint32 arrayNo);
	NONS_VariableMember *retrieveFromArray(Sint32 array,const std::vector<Sint32> &v,ErrorCode *error,ulong *erroredIndex=0);
	Sint32 getVariableIndex(NONS_VariableMember *var);

	ErrorCode getWcsValue(const std::wstring &str,std::wstring &value,bool invert_terms);
	ErrorCode getWcsValue(NONS_Expression::ExpressionCompiler *ec,std::wstring &value,bool invert_terms);
	ErrorCode getIntValue(const std::wstring &str,long &value,bool invert_terms);
	ErrorCode getIntValue(NONS_Expression::ExpressionCompiler *ec,long &value,bool invert_terms);
};
#endif
