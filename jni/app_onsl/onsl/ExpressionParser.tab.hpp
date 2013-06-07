/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* "%code requires" blocks.  */

/* Line 1685 of yacc.c  */
#line 47 "ExpressionParser.ypp"

	#include <set>
	#include <vector>
	#include "Common.h"
	struct NONS_VariableStore;
	namespace NONS_Expression{
		struct Expression;
	}
	struct wstrCmp;
	struct PreParserData;
	#ifdef ERROR
	#undef ERROR
	#endif



/* Line 1685 of yacc.c  */
#line 56 "ExpressionParser.tab.hpp"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FOR = 258,
     FOR_TO = 259,
     IF = 260,
     FOR_STEP = 261,
     INTEGER = 262,
     STRING = 263,
     INTEGER_ARRAY = 264,
     ERROR = 265,
     IF_THEN = 266,
     OR = 267,
     AND = 268,
     GREATEREQ = 269,
     GREATER = 270,
     LOWEREQ = 271,
     LOWER = 272,
     NEQ = 273,
     EQUALS = 274,
     LCHK = 275,
     FCHK = 276,
     POS = 277,
     NEG = 278,
     ITOA = 279,
     ATOI = 280,
     EVAL = 281,
     STRING_CONCATENATION = 282
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 84 "ExpressionParser.ypp"

	NONS_Expression::Expression *obj;
	ulong position;



/* Line 1685 of yacc.c  */
#line 107 "ExpressionParser.tab.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




/* "%code provides" blocks.  */

/* Line 1685 of yacc.c  */
#line 61 "ExpressionParser.ypp"

	#include <sstream>
	extern int expressionParser_yydebug;
	int expressionParser_yyparse(
		std::wstringstream *stream,
		NONS_VariableStore *store,
		NONS_Expression::Expression *&res,
		PreParserData *ppd
	);
	int expressionParser_yylex(
		YYSTYPE *yylval,
		std::wstringstream *stream,
		NONS_VariableStore *store,
		PreParserData *ppd
	);
	void expressionParser_yyerror(
		std::wstringstream *,
		NONS_VariableStore *,
		NONS_Expression::Expression *&,
		PreParserData *ppd,
		char const *
	);



/* Line 1685 of yacc.c  */
#line 147 "ExpressionParser.tab.hpp"
