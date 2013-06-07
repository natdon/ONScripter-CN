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

#include "VariableStore.h"
#include "IOFunctions.h"
#include "Options.h"
#include <cstdarg>
#include <iostream>

namespace NONS_Expression{
	inline Value *makeError(ErrorCode error){
		Value *ret=new Value;
		ret->error=error;
		return ret;
	}

	Value::Value(){
		this->type=INVALID;
		this->error=NONS_UNDEFINED_ERROR;
	}

	Value::Value(long i){
		this->type=INTEGER;
		this->integer=i;
		this->negated=0;
	}

	Value::Value(const std::wstring &s){
		this->type=STRING;
		this->string=s;
	}

	Value::Value(NONS_VariableMember *member){
		if (member->getType()==::INTEGER){
			this->type=INTEGER;
			this->integer=member->getInt();
		}else{
			this->type=STRING;
			this->string=member->getWcs();
		}
	}

#define CHECK_OPERANDS(min) \
	ulong base;\
	{\
		ulong minimum=(min);\
		base=operands.size()-minimum;\
		if (operands.size()<minimum)\
			return new Value;\
		for (size_t a=base;a<operands.size();a++)\
			if (!operands[a])\
				return new Value;\
	}
#define OPERAND(i) operands[base+i]
#define CHECK_TYPE(op,of_type,error) \
	if (OPERAND(op)->type!=of_type)\
		return makeError(error)
#define EXPECT_INTEGER(op) CHECK_TYPE(op,Value::INTEGER,NONS_EXPECTED_INTEGRAL_VALUE)
#define EXPECT_STRING(op) CHECK_TYPE(op,Value::STRING,NONS_EXPECTED_STRING_VALUE)

	static inline void deleteTop(std::vector<Value *> &v,ulong n=1){
		for (;n;n--){
			delete v.back();
			v.pop_back();
		}
	}

	NONS_Expression_DECLARE_OPERATOR(eval){
		CHECK_OPERANDS(1);
		EXPECT_STRING(0);
		std::wstring expr=OPERAND(0)->string;
		deleteTop(operands);
		return store->evaluate(expr,0);
	}

	NONS_Expression_DECLARE_OPERATOR(atoi){
		CHECK_OPERANDS(1);
		EXPECT_STRING(0);
		Value *ret=new Value(::atol(OPERAND(0)->string));
		deleteTop(operands);
		return ret;
	}
	NONS_Expression_DECLARE_OPERATOR(itoa){
		CHECK_OPERANDS(1);
		EXPECT_INTEGER(0);
		Value *ret=new Value(::itoaw(OPERAND(0)->integer));
		deleteTop(operands);
		return ret;
	}
	NONS_Expression_DECLARE_OPERATOR(concat){
		CHECK_OPERANDS(2);
		EXPECT_STRING(0);
		EXPECT_STRING(1);
		OPERAND(0)->string.append(OPERAND(1)->string);
		deleteTop(operands);
		return 0;
	}
	NONS_Expression_DECLARE_OPERATOR(array_indexing){
		ulong n=operands.back()->integer+1;
		CHECK_OPERANDS(n);
		deleteTop(operands);
		n--;
		std::vector<Sint32> v(n-1);
		EXPECT_INTEGER(0);
		Sint32 array=OPERAND(0)->integer;
		for (ulong a=1;a<n;a++){
			EXPECT_INTEGER(a);
			v[a-1]=OPERAND(a)->integer;
		}
		deleteTop(operands,n-1);
		ErrorCode e;
		NONS_VariableMember *member=store->retrieveFromArray(array,v,&e);
		if (!member)
			return makeError(e);
		if (member->getType()==::INTEGER_ARRAY)
			return makeError(NONS_INSUFFICIENT_DIMENSIONS);
		OPERAND(0)->integer=member->getInt();
		return 0;
	}
#define BINARY_OPERATOR(op) {\
		CHECK_OPERANDS(2);\
		EXPECT_INTEGER(0);\
		EXPECT_INTEGER(1);\
		OPERAND(0)->integer=(OPERAND(0)->integer op OPERAND(1)->integer);\
		OPERAND(0)->negated=OPERAND(0)->negated && OPERAND(1)->negated;\
		deleteTop(operands);\
		return 0;\
	}

	NONS_Expression_DECLARE_OPERATOR(add)
		BINARY_OPERATOR(+)
	NONS_Expression_DECLARE_OPERATOR(sub)
		BINARY_OPERATOR(-)
	NONS_Expression_DECLARE_OPERATOR(mul)
		BINARY_OPERATOR(*)
	NONS_Expression_DECLARE_OPERATOR(div){
		CHECK_OPERANDS(2);
		EXPECT_INTEGER(0);
		EXPECT_INTEGER(1);
		if (!OPERAND(1)->integer)
			return new Value;
		OPERAND(0)->integer=OPERAND(0)->integer/OPERAND(1)->integer;
		deleteTop(operands);
		return 0;
	}
	NONS_Expression_DECLARE_OPERATOR(and_operator){
		CHECK_OPERANDS(2);
		EXPECT_INTEGER(0);
		EXPECT_INTEGER(1);
		OPERAND(0)->negate(invert_terms);
		OPERAND(1)->negate(invert_terms);
		OPERAND(0)->integer=OPERAND(0)->integer && OPERAND(1)->integer;
		deleteTop(operands);
		return 0;
	}
	NONS_Expression_DECLARE_OPERATOR(or_operator)
		BINARY_OPERATOR(||)
	NONS_Expression_DECLARE_OPERATOR(neg){
		CHECK_OPERANDS(1);
		EXPECT_INTEGER(0);
		OPERAND(0)->integer=-OPERAND(0)->integer;
		return 0;
	}
	NONS_Expression_DECLARE_OPERATOR(integer_dereference){
		CHECK_OPERANDS(1);
		EXPECT_INTEGER(0);
		ErrorCode error;
		NONS_Variable *var=store->retrieve(OPERAND(0)->integer,&error);
		if (!var)
			return makeError(error);
		OPERAND(0)->integer=var->intValue->getInt();
		return 0;
	}
	NONS_Expression_DECLARE_OPERATOR(string_dereference){
		CHECK_OPERANDS(1);
		EXPECT_INTEGER(0);
		ErrorCode error;
		NONS_Variable *var=store->retrieve(OPERAND(0)->integer,&error);
		deleteTop(operands);
		if (!var)
			return makeError(error);
		return new Value(var->wcsValue->getWcs());
	}
	NONS_Expression_DECLARE_OPERATOR(equals)
		BINARY_OPERATOR(==)
	NONS_Expression_DECLARE_OPERATOR(nequals)
		BINARY_OPERATOR(!=)
	NONS_Expression_DECLARE_OPERATOR(lower)
		BINARY_OPERATOR(<)
	NONS_Expression_DECLARE_OPERATOR(greatereq)
		BINARY_OPERATOR(>=)
	NONS_Expression_DECLARE_OPERATOR(greater)
		BINARY_OPERATOR(>)
	NONS_Expression_DECLARE_OPERATOR(lowereq)
		BINARY_OPERATOR(<=)
	NONS_Expression_DECLARE_OPERATOR(fchk){
		CHECK_OPERANDS(1);
		EXPECT_STRING(0);
		long a=NONS_Surface::filelog_check(OPERAND(0)->string);
		deleteTop(operands);
		return new Value(a);
	}
	NONS_Expression_DECLARE_OPERATOR(lchk){
		CHECK_OPERANDS(1);
		EXPECT_STRING(0);
		long a=labellog.check(OPERAND(0)->string);
		deleteTop(operands);
		return new Value(a);
	}

	Expression::Expression(operator_f op,...){
		this->op=op;
		va_list list;
		va_start(list,op);
		while (1){
			Expression *expr=va_arg(list,Expression *);
			if (!expr)
				break;
			this->operands.push_back(expr);
		}
		va_end(list);
		this->val=0;
	}

	Expression::Expression(Value *val){
		this->op=0;
		this->val=val;
	}

	Expression::~Expression(){
		if (this->val)
			delete this->val;
		if (this->operands.size()){
			std::vector<Expression *> full_expression;
			this->vectorize(full_expression);
			for (size_t a=0;a<full_expression.size();a++){
				if (full_expression[a]==this)
					continue;
				full_expression[a]->operands.clear();
				delete full_expression[a];
			}
		}
	}

	void Expression::vectorize(std::vector<Expression *> &dst){
		typedef std::pair<Expression *,ulong> context;
		std::vector<context> stack;
		stack.push_back(std::make_pair(this,0));
		while (stack.size()){
			if (!stack.back().first->op || !stack.back().first->operands.size()){
				dst.push_back(stack.back().first);
				stack.pop_back();
				while (stack.size()){
					if (++stack.back().second < stack.back().first->operands.size())
						break;
					dst.push_back(stack.back().first);
					stack.pop_back();
				}
			}else
				stack.push_back(std::make_pair(stack.back().first->operands[stack.back().second],0));
		}
	}

	ExpressionCompiler::ExpressionCompiler(const std::wstring &exp,NONS_VariableStore *store){
		std::wstringstream stream(exp);
		this->expr=0;
#ifdef _DEBUG
		if (expressionParser_yydebug)
			STD_COUT <<"-------------------------------------------------------------------------------\n"
				"["<<exp<<"]\n"
				"-------------------------------------------------------------------------------\n";
#endif
		switch (expressionParser_yyparse(&stream,store,this->expr,0)){
			case 0:
				this->error=NONS_NO_ERROR;
				break;
			case 1:
				this->error=NONS_SYNTAX_ERROR;
				break;
			case 2:
				this->error=NONS_UNDEFINED_ERROR;
				break;
		}
	}

	ExpressionCompiler::ExpressionCompiler(Expression *expr){
		this->expr=expr;
	}

	ExpressionCompiler::~ExpressionCompiler(){
		if (this->expr)
			delete this->expr;
	}

	ErrorCode ExpressionCompiler::run(NONS_VariableStore *store,bool invert_terms,const std::vector<Expression *> &full_expression,std::vector<Value *> &evaluation_stack,size_t n){
		for (ulong a=0;a<n;a++){
			Expression *expr=full_expression[a];
			if (!expr->op){
				evaluation_stack.push_back(new Value(*expr->val));
				if (evaluation_stack.back()->type==Value::INVALID){
					ErrorCode error=evaluation_stack.back()->error;
					freePointerVector(evaluation_stack);
					return error;
				}
			}else{
				if (expr->op==array_indexing)
					evaluation_stack.push_back(new Value(expr->operands.size()));
				Value *val=expr->op(evaluation_stack,store,invert_terms);
				if (val)
					evaluation_stack.push_back(val);
			}
		}
		if (n==full_expression.size() && evaluation_stack.size()>1){
			freePointerVector(evaluation_stack);
			return NONS_UNDEFINED_ERROR;
		}
		return NONS_NO_ERROR;
	}

	Value *ExpressionCompiler::evaluate(NONS_VariableStore *store,bool invert_terms){
		if (!this->expr)
			return makeError(this->error);

		std::vector<Expression *> full_expression;
		this->expr->vectorize(full_expression);

		std::vector<Value *> evaluation_stack;
		ErrorCode e=this->run(store,invert_terms,full_expression,evaluation_stack,full_expression.size());
		if (e!=NONS_NO_ERROR)
			return makeError(e);
		evaluation_stack[0]->negate(invert_terms);
		return evaluation_stack[0];
	}

	NONS_VariableMember *ExpressionCompiler::retrieve(NONS_VariableStore *store){
		if (!this->expr)
			return 0;
		this->error=NONS_UNDEFINED_ERROR;

		std::vector<Expression *> full_expression;
		this->expr->vectorize(full_expression);

		operator_f f=full_expression.back()->op;
		bool integer=(f==integer_dereference),
			string=(f==string_dereference),
			array=(f==array_indexing);
		if (!integer && !string && !array){
			this->error=NONS_NOT_A_DEREFERENCE;
			return 0;
		}

		std::vector<Value *> evaluation_stack;
		ErrorCode e=this->run(store,0,full_expression,evaluation_stack,full_expression.size()-1);
		if (e!=NONS_NO_ERROR){
			this->error=e;
			return 0;
		}
		if (integer || string){
			if (evaluation_stack.size()>1){
				freePointerVector(evaluation_stack);
				return 0;
			}
			if (evaluation_stack[0]->type!=Value::INTEGER){
				this->error=NONS_EXPECTED_INTEGRAL_VALUE;
				freePointerVector(evaluation_stack);
				return 0;
			}
			NONS_Variable *var=store->retrieve((Sint32)evaluation_stack[0]->integer,&this->error);
			freePointerVector(evaluation_stack);
			if (!var)
				return 0;
			return (integer)?var->intValue:var->wcsValue;
		}
		ulong indices=full_expression.back()->operands.size()-1,
			base=evaluation_stack.size()-indices-1;
		if (evaluation_stack.size()>indices+1){
			freePointerVector(evaluation_stack);
			return 0;
		}
		Sint32 arrayNo=0;
		std::vector<Sint32> v(indices);
		for (ulong a=base;a<base+indices+1;a++){
			if (evaluation_stack[a]->type!=Value::INTEGER){
				this->error=NONS_EXPECTED_INTEGRAL_VALUE;
				freePointerVector(evaluation_stack);
				return 0;
			}
			((a!=base)?v[a-base-1]:arrayNo)=evaluation_stack[a]->integer;
		}
		freePointerVector(evaluation_stack);
		ulong errored;
		NONS_VariableMember *member=store->retrieveFromArray(arrayNo,v,&e,&errored);
		if (e!=NONS_NO_ERROR){
			this->error=e;
			member=0;
			switch (e){
				case NONS_UNDEFINED_ARRAY:
					handleErrors(this->error,0,"ExpressionCompiler::retrieve",1);
					break;
				case NONS_ARRAY_INDEX_OUT_OF_BOUNDS:
					handleErrors(
						this->error,
						0,"ExpressionCompiler::retrieve",1,
						L"The index is: "+::itoaw(errored)+L" (contains "+::itoaw(v[errored])+L")"
					);
					break;
				case NONS_TOO_MANY_DIMENSIONS:
					handleErrors(this->error,0,"ExpressionCompiler::retrieve",1);
					break;
			}
		}
		return member;
	}
	ErrorCode ExpressionCompiler::getArrayDeclaration(std::vector<long> &dst,NONS_VariableStore *store){
		if (!this->expr)
			return this->error;

		std::vector<Expression *> full_expression;
		this->expr->vectorize(full_expression);

		operator_f f=full_expression.back()->op;
		if (f!=array_indexing)
			return NONS_UNDEFINED_ERROR;

		std::vector<Value *> evaluation_stack;
		ErrorCode e=this->run(store,0,full_expression,evaluation_stack,full_expression.size()-1);
		if (e!=NONS_NO_ERROR)
			return e;

		if (evaluation_stack.size()>full_expression.back()->operands.size()){
			freePointerVector(evaluation_stack);
			return NONS_UNDEFINED_ERROR;
		}
		dst.clear();
		for (ulong a=0;a<evaluation_stack.size();a++){
			Value *val=evaluation_stack[a];
			if (val->type!=Value::INTEGER){
				freePointerVector(evaluation_stack);
				return NONS_EXPECTED_INTEGRAL_VALUE;
			}
			dst.push_back(val->integer);
		}
		freePointerVector(evaluation_stack);
		return NONS_NO_ERROR;
	}
	std::wstring ExpressionCompiler::unparse(){
		if (!this->expr)
			return L"";

		std::vector<Expression *> full_expression;
		this->expr->vectorize(full_expression);

		std::vector<std::wstring> stack;
		std::vector<char> type_stack;
		for (size_t a=0;a<full_expression.size();a++){
			Expression *expr=full_expression[a];
			std::wstring push;
			char type;
			if (!expr->op){
				if (expr->val->type==Value::INTEGER){
					push=::itoaw(expr->val->integer);
					type='%';
				}else{
					push=L"e\"";
					std::wstring &s=expr->val->string;
					for (size_t b=0;b<s.size();b++){
						wchar_t c=s[b];
						switch (c){
							case '\\':
							case '\"':
								push.push_back('\\');
								push.push_back(c);
								break;
							case '\n':
								push.append(L"\\n");
								break;
							default:
								push.push_back(c);
						}
					}
					push.push_back('\"');
					type='$';
				}
			}else{
				size_t base=stack.size()-expr->operands.size();
				if (expr->op==eval){
					push=L"(eval("+stack[base]+L"))";
					type='%';
				}else if (expr->op==atoi){
					push=L"(_atoi("+stack[base]+L"))";
					type='%';
				}else if (expr->op==itoa){
					push=L"(_itoa("+stack[base]+L"))";
					type='$';
				}else if (expr->op==concat){
					push=L"("+stack[base]+L"+"+stack[base+1]+L")";
					type='$';
				}else if (expr->op==array_indexing){
					push=L"(?"+stack[base];
					for (size_t b=1;a<expr->operands.size();a++){
						push.push_back('[');
						push.append(stack[base+b]);
						push.push_back(']');
					}
					push.push_back(')');
					type='%';
				}else if (expr->op==add){
					push=L"("+stack[base]+L"+"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==sub){
					push=L"("+stack[base]+L"-"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==mul){
					push=L"("+stack[base]+L"*"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==div){
					push=L"("+stack[base]+L"/"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==and_operator){
					push=L"("+stack[base]+L"&"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==or_operator){
					push=L"("+stack[base]+L"|"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==neg){
					push=L"(-"+stack[base]+L")";
					type='%';
				}else if (expr->op==integer_dereference){
					push=L"%"+stack[base];
					type='%';
				}else if (expr->op==string_dereference){
					push=L"$"+stack[base];
					type='$';
				}else if (expr->op==equals){
					push=L"("+stack[base]+L"=="+stack[base+1]+L")";
					type='%';
				}else if (expr->op==nequals){
					push=L"("+stack[base]+L"!="+stack[base+1]+L")";
					type='%';
				}else if (expr->op==lower){
					push=L"("+stack[base]+L"<"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==greatereq){
					push=L"("+stack[base]+L">="+stack[base+1]+L")";
					type='%';
				}else if (expr->op==greater){
					push=L"("+stack[base]+L">"+stack[base+1]+L")";
					type='%';
				}else if (expr->op==lowereq){
					push=L"("+stack[base]+L"<="+stack[base+1]+L")";
					type='%';
				}else if (expr->op==fchk){
					push=L"(fchk("+stack[base]+L"))";
					type='%';
				}else if (expr->op==lchk){
					if (type_stack[base]=='%')
						push=L"(lchk(*"+stack[base]+L"))";
					else
						push=L"(lchk("+stack[base]+L"))";
					type='%';
				}
				for (size_t b=expr->operands.size();b;b--){
					stack.pop_back();
					type_stack.pop_back();
				}
			}
			stack.push_back(push);
			type_stack.push_back(type);
		}
		return stack.back();
	}
};

const std::wstring NONS_VariableMember::null;

NONS_VariableMember::NONS_VariableMember(yytokentype type){
	this->intValue=0;
	this->type=type;
	this->set_default_limits();
	this->constant=0;
	this->dimension=0;
	this->dimensionSize=0;
}

NONS_VariableMember::NONS_VariableMember(long value){
	this->intValue=value;
	this->type=INTEGER;
	this->set_default_limits();
	this->constant=0;
	this->dimension=0;
	this->dimensionSize=0;
}

NONS_VariableMember::NONS_VariableMember(const std::wstring &a){
	this->intValue=0;
	this->type=STRING;
	this->set_default_limits();
	this->constant=0;
	this->dimension=0;
	this->dimensionSize=0;
	this->set(a);
}

NONS_VariableMember::NONS_VariableMember(std::vector<long> &sizes,size_t startAt){
	this->intValue=0;
	this->set_default_limits();
	this->constant=0;
	if (startAt<sizes.size()){
		this->type=INTEGER_ARRAY;
		this->dimensionSize=sizes[startAt]+1;
		this->dimension=new NONS_VariableMember*[this->dimensionSize];
		for (ulong a=0;a<ulong(this->dimensionSize);a++)
			this->dimension[a]=new NONS_VariableMember(sizes,startAt+1);
	}else{
		this->type=INTEGER;
		this->dimension=0;
		this->dimensionSize=0;
	}
}

NONS_VariableMember::NONS_VariableMember(const NONS_VariableMember &b){
	this->constant=b.constant;
	this->intValue=b.intValue;
	this->_long_upper_limit=b._long_upper_limit;
	this->_long_lower_limit=b._long_lower_limit;
	this->wcsValue=b.wcsValue;
	this->type=b.type;
	this->dimensionSize=b.dimensionSize;
	if (this->type!=INTEGER_ARRAY)
		this->dimension=0;
	else{
		this->dimension=new NONS_VariableMember*[this->dimensionSize];
		for (ulong a=0;a<this->dimensionSize;a++)
			this->dimension[a]=new NONS_VariableMember(*b.dimension[a]);
	}
}

NONS_VariableMember::~NONS_VariableMember(){
	if (!!this->dimension){
		for (ulong a=0;a<this->dimensionSize;a++)
			delete this->dimension[a];
		delete[] this->dimension;
	}
}

void NONS_VariableMember::makeConstant(){
	this->constant=1;
}

bool NONS_VariableMember::isConstant(){
	return this->constant;
}

yytokentype NONS_VariableMember::getType(){
	return this->type;
}

void NONS_VariableMember::fixint(){
	if (this->intValue>this->_long_upper_limit)
		this->intValue=_long_upper_limit;
	else if (this->intValue<this->_long_lower_limit)
		this->intValue=_long_lower_limit;
}

long NONS_VariableMember::getInt(){
	if (this->type==INTEGER || this->type==INTEGER_ARRAY)
		return this->intValue;
	return 0;
}

const std::wstring &NONS_VariableMember::getWcs(){
	return this->type==STRING?this->wcsValue:this->null;
}

NONS_VariableMember *NONS_VariableMember::getIndex(ulong i){
	if (this->type==INTEGER_ARRAY && i<this->dimensionSize)
		return this->dimension[i];
	return 0;
}

void NONS_VariableMember::set(long a){
	if (this->constant)
		return;
	if (this->type==INTEGER){
		this->intValue=a;
		this->fixint();
	}
}

void NONS_VariableMember::atoi(const std::wstring &a){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue=::atol(a);
	this->fixint();
}

void NONS_VariableMember::set(const std::wstring &a){
	if (this->constant || this->type==INTEGER || this->type==INTEGER_ARRAY)
		return;
	if (this->type==STRING)
		this->wcsValue=a;
}

void NONS_VariableMember::inc(){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue++;
	this->fixint();
}

void NONS_VariableMember::dec(){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue--;
	this->fixint();
}

void NONS_VariableMember::add(long a){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue+=a;
	this->fixint();
}

void NONS_VariableMember::sub(long a){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue-=a;
	this->fixint();
}

void NONS_VariableMember::mul(long a){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue*=a;
	this->fixint();
}

void NONS_VariableMember::div(long a){
	if (this->constant || this->type!=INTEGER)
		return;
	if (a)
		this->intValue/=a;
	else
		this->intValue=0;
	this->fixint();
}

void NONS_VariableMember::mod(long a){
	if (this->constant || this->type!=INTEGER)
		return;
	this->intValue%=a;
	this->fixint();
}

void NONS_VariableMember::setlimits(long lower,long upper){
	this->_long_lower_limit=lower;
	this->_long_upper_limit=upper;
	this->fixint();
}

struct NONS_VariableMember_save_frame{
	NONS_VariableMember *_this;
	TiXmlElement *element;
	size_t i;
};

TiXmlElement *NONS_VariableMember::save(int index){
	assert(this->type==INTEGER_ARRAY);
	std::vector<NONS_VariableMember_save_frame> stack;
	NONS_VariableMember_save_frame current={
		this,
		new TiXmlElement("array"),
		0
	};
	current.element->SetAttribute("no",index);
	while (1){
		if (current._this->type==INTEGER)
			current.element->SetAttribute("int",current._this->intValue);
		else if (current.i<current._this->dimensionSize){
			stack.push_back(current);
			current._this=current._this->dimension[current.i];
			current.i=0;
			current.element=new TiXmlElement(current._this->type==INTEGER_ARRAY?"array":"variable");
			continue;
		}
		if (!stack.size())
			break;
		stack.back().element->LinkEndChild(current.element);
		current=stack.back();
		stack.pop_back();
		current.i++;
	}
	return current.element;
}

struct NONS_VariableMember_ctor_frame{
	NONS_VariableMember *_this;
	TiXmlElement *array,
		*child;
	std::vector<NONS_VariableMember *> v;
};

NONS_VariableMember::NONS_VariableMember(TiXmlElement *array){
	this->set_default_limits();
	this->type=INTEGER_ARRAY;
	std::vector<NONS_VariableMember_ctor_frame> stack(1);
	NONS_VariableMember_ctor_frame *current;
	current=&stack.back();
	current->_this=this;
	current->array=array->FirstChildElement();
	current->child=0;
	while (1){
		if (current->array){
			if (current->array->ValueStr()=="variable"){
				do
					current->v.push_back(new NONS_VariableMember(current->array->QueryIntAttribute("int")));
				while (current->array=current->array->NextSiblingElement());
			}else{
				stack.resize(stack.size()+1);
				NONS_VariableMember_ctor_frame *new_current=&stack.back();
				current=new_current-1;
				new_current->_this=new NONS_VariableMember(INTEGER_ARRAY);
				new_current->array=(current->child)?current->child->NextSiblingElement():current->array->FirstChildElement();
				new_current->child=0;
				current=new_current;
				continue;
			}
		}
		NONS_VariableMember *temp=current->_this;
		temp->dimensionSize=current->v.size();
		temp->dimension=new NONS_VariableMember *[current->_this->dimensionSize];
		memcpy(temp->dimension,&(current->v)[0],current->v.size()*sizeof(NONS_VariableMember *));
		if (stack.size()==1)
			break;
		stack.pop_back();
		current=&stack.back();
		current->v.push_back(temp);
		current->array=current->array->NextSiblingElement();
	}
}

NONS_Variable::NONS_Variable(){
	this->intValue=new NONS_VariableMember(INTEGER);
	this->wcsValue=new NONS_VariableMember(STRING);
}

NONS_Variable::NONS_Variable(const NONS_Variable &b){
	this->intValue=new NONS_VariableMember(*b.intValue);
	this->wcsValue=new NONS_VariableMember(*b.wcsValue);
}

NONS_Variable &NONS_Variable::operator=(const NONS_Variable &b){
	delete this->intValue;
	delete this->wcsValue;
	this->intValue=new NONS_VariableMember(*b.intValue);
	this->wcsValue=new NONS_VariableMember(*b.wcsValue);
	return *this;
}

NONS_Variable::~NONS_Variable(){
	delete this->intValue;
	delete this->wcsValue;
}

TiXmlElement *NONS_Variable::save(int index){
	TiXmlElement *variable=new TiXmlElement("variable");
	variable->SetAttribute("no",index);
	if (this->intValue->getInt())
		variable->SetAttribute("int",this->intValue->getInt());
	if (this->wcsValue->getWcs().size())
		variable->SetAttribute("str",this->wcsValue->getWcs());
	return variable;
}

NONS_Variable::NONS_Variable(TiXmlElement *variable){
	int i;
	this->intValue=(variable->QueryIntAttribute("int",&i)==TIXML_SUCCESS)
		?new NONS_VariableMember(i)
		:new NONS_VariableMember(INTEGER);
	std::string s;
	this->wcsValue=(variable->QueryStringAttribute("str",&s)==TIXML_SUCCESS)
		?new NONS_VariableMember(UniFromUTF8(s))
		:new NONS_VariableMember(INTEGER);
}

NONS_LabelLog labellog;

extern std::wstring save_directory;

NONS_VariableStore::NONS_VariableStore(){
	size_t l;
	this->commitGlobals=0;
#ifdef _DEBUG
	//expressionParser_yydebug=1;
#endif
	std::wstring dir=save_directory+L"global.sav";
	uchar *buffer=NONS_File::read(dir.c_str(),l);
	if (!buffer){
		buffer=NONS_File::read((std::wstring)L"gloval.sav",l);
		if (!buffer)
			return;
		for (ulong a=0,stackpos=settings.global_border.data;a<l;stackpos++){
			
			NONS_Variable *var=new NONS_Variable();
			var->intValue->set(readSignedDWord((char *)buffer,a));
			var->wcsValue->set(UniFromSJIS(readString((char *)buffer,a)));
			this->variables[stackpos]=var;
		}
	}else{
		if (begins_with((char *)buffer,"BZh")){
			uchar *temp=decompressBuffer_BZ2(buffer,l,l);
			delete[] buffer;
			buffer=(uchar *)temp;
		}
		ulong offset=0;
		Uint32 intervalsN=readDWord((char *)buffer,offset);
		std::vector<Sint32> intervals;
		for (Uint32 a=0;a<intervalsN;a++){
			Uint32 b=readDWord((char *)buffer,offset);
			if (b&0x80000000){
				b&=0x7FFFFFFF;
				intervals.push_back((Sint32)b);
				intervals.push_back(1);
			}else{
				intervals.push_back((Sint32)b);
				intervals.push_back(readSignedDWord((char *)buffer,offset));
			}
		}
		if (intervalsN){
			ulong currentInterval=0;
			while (offset<l){
				ulong a=intervals[currentInterval],
					b=intervals[currentInterval+1];
				currentInterval+=2;
				for (ulong c=0;c<b;c++){
					NONS_Variable *var=new NONS_Variable();
					var->intValue->set(readSignedDWord((char *)buffer,offset));
					var->wcsValue->set(UniFromUTF8(readString((char *)buffer,offset)));
					this->variables[a++]=var;
				}
			}
		}
	}
	delete[] buffer;
}

NONS_VariableStore::~NONS_VariableStore(){
	this->saveData();
	for (constants_map_T::iterator i=this->constants.begin();i!=this->constants.end();i++)
		delete i->second;
	this->reset();
}

void NONS_VariableStore::reset(){
	for (variables_map_T::iterator i=this->variables.begin();i!=this->variables.end();i++)
		delete i->second;
	this->variables.clear();
	for (arrays_map_T::iterator i=this->arrays.begin();i!=this->arrays.end();i++)
		delete i->second;
	this->arrays.clear();
}

void NONS_VariableStore::saveData(){
	if (!this->commitGlobals)
		return;
	std::vector<uchar> buffer;
	variables_map_T::iterator i=this->variables.find(settings.global_border.data);
	if (i==this->variables.end())
		i--;
	if (!this->variables.size() || i->first<settings.global_border.data)
		writeDWord(0,buffer);
	else{
		for (;i!=this->variables.end() && VARIABLE_HAS_NO_DATA(i->second);i++);
		if (i==this->variables.end())
			writeDWord(0,buffer);
		else{
			variables_map_T::iterator i2=i;
			std::vector<Sint32> intervals;
			Sint32 last=i->first;
			intervals.push_back(last++);
			for (i++;i!=this->variables.end();i++){
				if (VARIABLE_HAS_NO_DATA(i->second))
					continue;
				if (i->first!=last){
					intervals.push_back(last-intervals.back());
					last=i->first;
					intervals.push_back(last++);
				}else
					last++;
			}
			intervals.push_back(last-intervals.back());
			writeDWord(intervals.size()/2,buffer);
			for (ulong a=0;a<intervals.size();){
				if (intervals[a+1]>1){
					writeDWord(intervals[a++],buffer);
					writeDWord(intervals[a++],buffer);
				}else{
					writeDWord(intervals[a]|0x80000000,buffer);
					a+=2;
				}
			}
			for (i=i2;i!=this->variables.end();i++){
				if (VARIABLE_HAS_NO_DATA(i->second))
					continue;
				writeDWord(i->second->intValue->getInt(),buffer);
				writeString(i->second->wcsValue->getWcs(),buffer);
			}
		}
	}
	size_t l;
	uchar *writebuffer=compressBuffer_BZ2(&buffer[0],buffer.size(),l);
	std::wstring dir=save_directory+L"global.sav";
	NONS_File::write(dir.c_str(),writebuffer,l);
	delete[] writebuffer;
}

TiXmlElement *NONS_VariableStore::save_locals(){
	TiXmlElement *locals=new TiXmlElement("locals");
	{
		TiXmlElement *scalars=new TiXmlElement("scalars");
		locals->LinkEndChild(scalars);
		for (variables_map_T::iterator i=this->variables.begin();i!=this->variables.end() && i->first<settings.global_border.data;++i)
			if (!VARIABLE_HAS_NO_DATA(i->second))
				scalars->LinkEndChild(i->second->save((int)i->first));
	}
	{
		TiXmlElement *arrays=new TiXmlElement("arrays");
		locals->LinkEndChild(arrays);
		for (arrays_map_T::iterator i=this->arrays.begin();i!=this->arrays.end();i++)
			arrays->LinkEndChild(i->second->save((int)i->first));
	}

	return locals;
}

void NONS_VariableStore::load_locals(TiXmlElement *parent){
	TiXmlElement *locals=parent->FirstChildElement("locals");

	for (variables_map_T::iterator i=this->variables.begin();i!=this->variables.end() && i->first<settings.global_border.data;++i){
		delete i->second;
		i->second=0;
	}
	for (TiXmlElement *i=locals->FirstChildElement("scalars")->FirstChildElement();i;i=i->NextSiblingElement())
		this->variables[i->QueryIntAttribute("no")]=new NONS_Variable(i);

	for (arrays_map_T::iterator i=this->arrays.begin();i!=this->arrays.end();i++)
		delete i->second;
	this->arrays.clear();
	for (TiXmlElement *i=locals->FirstChildElement("arrays")->FirstChildElement();i;i=i->NextSiblingElement())
		this->arrays[i->QueryIntAttribute("no")]=new NONS_VariableMember(i);
}

NONS_Expression::Value *NONS_VariableStore::evaluate(const std::wstring &exp,bool invert_terms){
	NONS_Expression::ExpressionCompiler c(exp,this);
	return c.evaluate(this,invert_terms);
}

ErrorCode NONS_VariableStore::array_declaration(std::vector<long> &dst,const std::wstring &exp){
	NONS_Expression::ExpressionCompiler c(exp,this);
	return c.getArrayDeclaration(dst,this);
}

void NONS_VariableStore::push(Sint32 pos,NONS_Variable *var){
	variables_map_T::iterator i=this->variables.find(pos);
	if (i==this->variables.end())
		this->variables[pos]=var;
	else{
		delete i->second;
		i->second=var;
	}
}

NONS_VariableMember *NONS_VariableStore::retrieve(const std::wstring &name,ErrorCode *error){
	NONS_Expression::ExpressionCompiler c(name,this);
	NONS_VariableMember *ret=c.retrieve(this);
	if (!ret && error)
		*error=c.getError();
	return ret;
}

NONS_Variable *NONS_VariableStore::retrieve(Sint32 position,ErrorCode *error){
	if (position<indexLowerLimit || position>indexUpperLimit){
		if (!!error)
			*error=NONS_VARIABLE_OUT_OF_RANGE;
		return 0;
	}
	if (!!error)
		*error=NONS_NO_ERROR;
	variables_map_T::iterator i=this->variables.find(position);
	if (i==this->variables.end()){
		NONS_Variable *var=new NONS_Variable();
		this->variables[position]=var;
		return var;
	}
	if (!i->second)
		i->second=new NONS_Variable();
	return i->second;
}

ErrorCode NONS_VariableStore::getWcsValue(const std::wstring &str,std::wstring &value,bool invert_terms){
	NONS_Expression::ExpressionCompiler c(str,this);
	return this->getWcsValue(&c,value,invert_terms);
}

ErrorCode NONS_VariableStore::getWcsValue(NONS_Expression::ExpressionCompiler *ec,std::wstring &value,bool invert_terms){
	NONS_Expression::Value *val=ec->evaluate(this,invert_terms);
	ErrorCode e=val->error;
	switch (val->type){
		case NONS_Expression::Value::INVALID:
			delete val;
			return e;
		case NONS_Expression::Value::INTEGER:
			delete val;
			return NONS_EXPECTED_STRING_VALUE;
		case NONS_Expression::Value::STRING:
			value=val->string;
			delete val;
			return NONS_NO_ERROR;
	}
	return NONS_INTERNAL_ERROR;
}

ErrorCode NONS_VariableStore::getIntValue(const std::wstring &str,long &value,bool invert_terms){
	NONS_Expression::ExpressionCompiler c(str,this);
	return this->getIntValue(&c,value,invert_terms);
}

ErrorCode NONS_VariableStore::getIntValue(NONS_Expression::ExpressionCompiler *ec,long &value,bool invert_terms){
	NONS_Expression::Value *val=ec->evaluate(this,invert_terms);
	ErrorCode e=val->error;
	switch (val->type){
		case NONS_Expression::Value::INVALID:
			delete val;
			return e;
		case NONS_Expression::Value::INTEGER:
			value=val->integer;
			delete val;
			return NONS_NO_ERROR;
		case NONS_Expression::Value::STRING:
			delete val;
			return NONS_EXPECTED_INTEGRAL_VALUE;
	}
	return NONS_INTERNAL_ERROR;
}

NONS_VariableMember *NONS_VariableStore::getArray(Sint32 arrayNo){
	arrays_map_T::iterator i=this->arrays.find(arrayNo);
	if (i==this->arrays.end())
		return 0;
	return i->second;
}

NONS_VariableMember *NONS_VariableStore::retrieveFromArray(Sint32 array,const std::vector<Sint32> &v,ErrorCode *error,ulong *erroredIndex){
	NONS_VariableMember *ret=this->getArray(array);
	if (!ret){
		if (error)
			*error=NONS_UNDEFINED_ARRAY;
		return 0;
	}
	for (ulong a=0;a<v.size();a++){
		if (ret->getType()==INTEGER_ARRAY){
			long b=v[a];
			if (!(ret=ret->getIndex(b))){
				if (error)
					*error=NONS_ARRAY_INDEX_OUT_OF_BOUNDS;
				if (erroredIndex)
					*erroredIndex=a;
				return 0;
			}
		}else{
			if (error)
				*error=NONS_TOO_MANY_DIMENSIONS;
			return 0;
		}
	}
	if (error)
		*error=NONS_NO_ERROR;
	return ret;
}

NONS_VariableMember *NONS_VariableStore::getConstant(const std::wstring &name){
	constants_map_T::iterator i=this->constants.find(name);
	if (i==this->constants.end())
		return 0;
	return i->second;
}

Sint32 NONS_VariableStore::getVariableIndex(NONS_VariableMember *var){
	if (var->getType()==INTEGER){
		for (variables_map_T::iterator i=this->variables.begin(),end=this->variables.end();i!=end;i++)
			if (i->second->intValue==var)
				return i->first;
	}else{
		for (variables_map_T::iterator i=this->variables.begin(),end=this->variables.end();i!=end;i++)
			if (i->second->wcsValue==var)
				return i->first;
	}
	return 0;
}
