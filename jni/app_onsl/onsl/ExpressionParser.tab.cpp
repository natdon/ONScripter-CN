/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         expressionParser_yyparse
#define yylex           expressionParser_yylex
#define yyerror         expressionParser_yyerror
#define yylval          expressionParser_yylval
#define yychar          expressionParser_yychar
#define yydebug         expressionParser_yydebug
#define yynerrs         expressionParser_yynerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 29 "ExpressionParser.ypp"

#include "ExpressionParser.tab.hpp"
#include "ScriptInterpreter.h"
#include "VariableStore.h"
#include "IOFunctions.h"


/* Line 189 of yacc.c  */
#line 88 "ExpressionParser.tab.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

/* "%code requires" blocks.  */

/* Line 209 of yacc.c  */
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



/* Line 209 of yacc.c  */
#line 129 "ExpressionParser.tab.cpp"

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

/* Line 214 of yacc.c  */
#line 84 "ExpressionParser.ypp"

	NONS_Expression::Expression *obj;
	ulong position;



/* Line 214 of yacc.c  */
#line 180 "ExpressionParser.tab.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

/* "%code provides" blocks.  */

/* Line 261 of yacc.c  */
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



/* Line 261 of yacc.c  */
#line 217 "ExpressionParser.tab.cpp"

/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 223 "ExpressionParser.tab.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  45
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   290

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  39
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  10
/* YYNRULES -- Number of rules.  */
#define YYNRULES  44
/* YYNRULES -- Number of states.  */
#define YYNSTATES  94

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   282

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    35,    34,     2,     2,
      37,    38,    24,    22,     2,    23,     2,    25,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    36,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    28,     2,    29,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    26,    27,    30,
      31,    32,    33
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    19,
      26,    35,    37,    42,    47,    50,    53,    58,    62,    64,
      67,    72,    76,    80,    82,    84,    88,    92,    96,   100,
     104,   108,   112,   116,   120,   124,   127,   131,   134,   138,
     142,   145,   148,   152,   154
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      40,     0,    -1,    41,    -1,    42,    -1,    43,    -1,    47,
      -1,    46,    -1,    10,    -1,     5,    47,    11,    -1,     3,
      47,    19,    47,     4,    47,    -1,     3,    47,    19,    47,
       4,    47,     6,    47,    -1,     7,    -1,    32,    37,    46,
      38,    -1,    31,    37,    46,    38,    -1,    34,    48,    -1,
      36,    48,    -1,    45,    28,    47,    29,    -1,    36,     1,
      28,    -1,     8,    -1,    35,    48,    -1,    30,    37,    47,
      38,    -1,    37,    46,    38,    -1,    46,    22,    46,    -1,
      44,    -1,    45,    -1,    47,    22,    47,    -1,    47,    23,
      47,    -1,    47,    24,    47,    -1,    47,    25,    47,    -1,
      47,    19,    47,    -1,    47,    18,    47,    -1,    47,    17,
      47,    -1,    47,    14,    47,    -1,    47,    15,    47,    -1,
      47,    16,    47,    -1,    21,    46,    -1,    20,    24,     7,
      -1,    20,    46,    -1,    47,    12,    47,    -1,    47,    13,
      47,    -1,    22,    47,    -1,    23,    47,    -1,    37,    47,
      38,    -1,    44,    -1,    37,    47,    38,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   112,   112,   113,   114,   117,   120,   123,   128,   135,
     141,   150,   153,   156,   159,   164,   167,   171,   177,   180,
     183,   186,   189,   194,   197,   200,   203,   206,   209,   212,
     215,   218,   221,   224,   227,   230,   234,   237,   240,   243,
     246,   249,   252,   256,   259
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FOR", "FOR_TO", "IF", "FOR_STEP",
  "INTEGER", "STRING", "INTEGER_ARRAY", "ERROR", "IF_THEN", "OR", "AND",
  "GREATEREQ", "GREATER", "LOWEREQ", "LOWER", "NEQ", "EQUALS", "LCHK",
  "FCHK", "'+'", "'-'", "'*'", "'/'", "POS", "NEG", "'['", "']'", "ITOA",
  "ATOI", "EVAL", "STRING_CONCATENATION", "'%'", "'$'", "'?'", "'('",
  "')'", "$accept", "begin", "eval", "parse_if", "parse_for", "integer",
  "integer_array", "string", "expr", "dereference_parameter", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,    43,    45,    42,    47,   277,   278,    91,    93,
     279,   280,   281,   282,    37,    36,    63,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    39,    40,    40,    40,    41,    41,    41,    42,    43,
      43,    44,    44,    44,    44,    45,    45,    45,    46,    46,
      46,    46,    46,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    48,    48
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     3,     6,
       8,     1,     4,     4,     2,     2,     4,     3,     1,     2,
       4,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     2,     3,     3,
       2,     2,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,    11,    18,     7,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     2,     3,
       4,    23,    24,     6,     5,     0,     0,     0,     0,     0,
      37,    35,    40,    41,     0,     0,     0,     0,    43,    14,
      19,     0,    15,     0,     0,     1,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     8,    36,     0,     0,     0,     0,    17,    21,    42,
       0,    22,    38,    39,    32,    33,    34,    31,    30,    29,
      25,    26,    27,    28,    29,    20,    13,    12,    44,    16,
       0,     9,     0,    10
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    22,    43,    44,    39
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -28
static const yytype_int16 yypact[] =
{
      64,     8,     8,   -28,   -28,   -28,    98,   104,     8,     8,
     -27,   -23,    31,    73,    73,     4,    95,    70,   -28,   -28,
     -28,   -28,    -3,    53,   238,     8,   252,   224,    69,   104,
      53,    53,   -28,   -28,     8,   104,   104,     8,   -28,   -28,
     -28,    49,   -28,   -18,   130,   -28,     8,   104,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,   -28,   -28,   147,     5,    25,   164,   -28,   -28,   -28,
     205,   -28,   265,   176,    -6,    -6,    -6,    -6,    -6,    -6,
     -12,   -12,   -28,   -28,    -1,   -28,   -28,   -28,   -28,   -28,
       8,   191,     8,   238
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -28,   -28,   -28,   -28,   -28,    51,   -28,    26,     0,    -8
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      24,    26,    27,    90,    47,    41,    40,    42,    32,    33,
      34,     3,    58,    59,    35,     3,    56,    57,    58,    59,
      68,    56,    57,    58,    59,    46,    23,    47,     6,     7,
       8,     9,    30,    31,    63,    11,    12,    66,    13,    11,
      12,    37,    13,    86,    15,    25,    70,    47,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    64,    65,    87,    38,    38,    38,     1,    36,     2,
      45,     3,     4,    71,     5,    47,    62,    67,     0,     0,
       3,     0,     0,     0,     6,     7,     8,     9,     0,     0,
      91,     0,    93,     0,    10,    11,    12,     0,    13,    14,
      15,    16,     3,     4,    11,    12,     4,    13,     0,     0,
      37,     0,     4,     0,     0,     6,     7,     8,     9,     0,
       0,     0,    28,     0,     0,    10,    11,    12,    10,    13,
      14,    15,    16,    14,    10,    29,     0,     0,     0,    14,
       0,    29,    48,    49,    50,    51,    52,    53,    54,    55,
       0,     0,    56,    57,    58,    59,     0,     0,     0,    48,
      49,    50,    51,    52,    53,    54,    55,     0,    69,    56,
      57,    58,    59,     0,     0,     0,    48,    49,    50,    51,
      52,    53,    54,    55,     0,    85,    56,    57,    58,    59,
      50,    51,    52,    53,    54,    55,     0,    92,    56,    57,
      58,    59,    88,    48,    49,    50,    51,    52,    53,    54,
      55,     0,     0,    56,    57,    58,    59,    48,    49,    50,
      51,    52,    53,    54,    55,     0,     0,    56,    57,    58,
      59,     0,     0,     0,    89,    61,    48,    49,    50,    51,
      52,    53,    54,    55,     0,     0,    56,    57,    58,    59,
      48,    49,    50,    51,    52,    53,    54,    55,     0,     0,
      56,    57,    58,    59,    48,    49,    50,    51,    52,    53,
      54,    60,     0,     0,    56,    57,    58,    59,    49,    50,
      51,    52,    53,    54,    55,     0,     0,    56,    57,    58,
      59
};

static const yytype_int8 yycheck[] =
{
       0,     1,     2,     4,    22,     1,    14,    15,     8,     9,
      37,     7,    24,    25,    37,     7,    22,    23,    24,    25,
      38,    22,    23,    24,    25,    28,     0,    22,    20,    21,
      22,    23,     6,     7,    34,    31,    32,    37,    34,    31,
      32,    37,    34,    38,    36,    37,    46,    22,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    35,    36,    38,    13,    14,    15,     3,    37,     5,
       0,     7,     8,    47,    10,    22,     7,    28,    -1,    -1,
       7,    -1,    -1,    -1,    20,    21,    22,    23,    -1,    -1,
      90,    -1,    92,    -1,    30,    31,    32,    -1,    34,    35,
      36,    37,     7,     8,    31,    32,     8,    34,    -1,    -1,
      37,    -1,     8,    -1,    -1,    20,    21,    22,    23,    -1,
      -1,    -1,    24,    -1,    -1,    30,    31,    32,    30,    34,
      35,    36,    37,    35,    30,    37,    -1,    -1,    -1,    35,
      -1,    37,    12,    13,    14,    15,    16,    17,    18,    19,
      -1,    -1,    22,    23,    24,    25,    -1,    -1,    -1,    12,
      13,    14,    15,    16,    17,    18,    19,    -1,    38,    22,
      23,    24,    25,    -1,    -1,    -1,    12,    13,    14,    15,
      16,    17,    18,    19,    -1,    38,    22,    23,    24,    25,
      14,    15,    16,    17,    18,    19,    -1,     6,    22,    23,
      24,    25,    38,    12,    13,    14,    15,    16,    17,    18,
      19,    -1,    -1,    22,    23,    24,    25,    12,    13,    14,
      15,    16,    17,    18,    19,    -1,    -1,    22,    23,    24,
      25,    -1,    -1,    -1,    29,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    -1,    -1,    22,    23,    24,    25,
      12,    13,    14,    15,    16,    17,    18,    19,    -1,    -1,
      22,    23,    24,    25,    12,    13,    14,    15,    16,    17,
      18,    19,    -1,    -1,    22,    23,    24,    25,    13,    14,
      15,    16,    17,    18,    19,    -1,    -1,    22,    23,    24,
      25
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     5,     7,     8,    10,    20,    21,    22,    23,
      30,    31,    32,    34,    35,    36,    37,    40,    41,    42,
      43,    44,    45,    46,    47,    37,    47,    47,    24,    37,
      46,    46,    47,    47,    37,    37,    37,    37,    44,    48,
      48,     1,    48,    46,    47,     0,    28,    22,    12,    13,
      14,    15,    16,    17,    18,    19,    22,    23,    24,    25,
      19,    11,     7,    47,    46,    46,    47,    28,    38,    38,
      47,    46,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    38,    38,    38,    38,    29,
       4,    47,     6,    47
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (stream, store, res, ppd, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, stream, store, ppd)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, stream, store, res, ppd); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, std::wstringstream *stream, NONS_VariableStore *store, NONS_Expression::Expression *&res, PreParserData *ppd)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, stream, store, res, ppd)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    std::wstringstream *stream;
    NONS_VariableStore *store;
    NONS_Expression::Expression *&res;
    PreParserData *ppd;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (stream);
  YYUSE (store);
  YYUSE (res);
  YYUSE (ppd);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, std::wstringstream *stream, NONS_VariableStore *store, NONS_Expression::Expression *&res, PreParserData *ppd)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, stream, store, res, ppd)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    std::wstringstream *stream;
    NONS_VariableStore *store;
    NONS_Expression::Expression *&res;
    PreParserData *ppd;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, stream, store, res, ppd);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, std::wstringstream *stream, NONS_VariableStore *store, NONS_Expression::Expression *&res, PreParserData *ppd)
#else
static void
yy_reduce_print (yyvsp, yyrule, stream, store, res, ppd)
    YYSTYPE *yyvsp;
    int yyrule;
    std::wstringstream *stream;
    NONS_VariableStore *store;
    NONS_Expression::Expression *&res;
    PreParserData *ppd;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , stream, store, res, ppd);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, stream, store, res, ppd); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, std::wstringstream *stream, NONS_VariableStore *store, NONS_Expression::Expression *&res, PreParserData *ppd)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, stream, store, res, ppd)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    std::wstringstream *stream;
    NONS_VariableStore *store;
    NONS_Expression::Expression *&res;
    PreParserData *ppd;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (stream);
  YYUSE (store);
  YYUSE (res);
  YYUSE (ppd);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 7: /* "INTEGER" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1267 "ExpressionParser.tab.cpp"
	break;
      case 8: /* "STRING" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1279 "ExpressionParser.tab.cpp"
	break;
      case 9: /* "INTEGER_ARRAY" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1291 "ExpressionParser.tab.cpp"
	break;
      case 44: /* "integer" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1303 "ExpressionParser.tab.cpp"
	break;
      case 45: /* "integer_array" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1315 "ExpressionParser.tab.cpp"
	break;
      case 46: /* "string" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1327 "ExpressionParser.tab.cpp"
	break;
      case 47: /* "expr" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1339 "ExpressionParser.tab.cpp"
	break;
      case 48: /* "dereference_parameter" */

/* Line 1009 of yacc.c  */
#line 105 "ExpressionParser.ypp"
	{
	if ((yyvaluep->obj))
		delete (yyvaluep->obj);
};

/* Line 1009 of yacc.c  */
#line 1351 "ExpressionParser.tab.cpp"
	break;

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (std::wstringstream *stream, NONS_VariableStore *store, NONS_Expression::Expression *&res, PreParserData *ppd);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (std::wstringstream *stream, NONS_VariableStore *store, NONS_Expression::Expression *&res, PreParserData *ppd)
#else
int
yyparse (stream, store, res, ppd)
    std::wstringstream *stream;
    NONS_VariableStore *store;
    NONS_Expression::Expression *&res;
    PreParserData *ppd;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:

/* Line 1464 of yacc.c  */
#line 117 "ExpressionParser.ypp"
    {
		res=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 120 "ExpressionParser.ypp"
    {
		res=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 123 "ExpressionParser.ypp"
    {
		YYABORT;
	;}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 128 "ExpressionParser.ypp"
    {
		ppd->res.push_back((yyvsp[(2) - (3)].obj));
		ppd->then_position=(yyvsp[(3) - (3)].position);
		YYACCEPT;
	;}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 135 "ExpressionParser.ypp"
    {
		ppd->res.push_back((yyvsp[(2) - (6)].obj));
		ppd->res.push_back((yyvsp[(4) - (6)].obj));
		ppd->res.push_back((yyvsp[(6) - (6)].obj));
		YYACCEPT;
	;}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 141 "ExpressionParser.ypp"
    {
		ppd->res.push_back((yyvsp[(2) - (8)].obj));
		ppd->res.push_back((yyvsp[(4) - (8)].obj));
		ppd->res.push_back((yyvsp[(6) - (8)].obj));
		ppd->res.push_back((yyvsp[(8) - (8)].obj));
		YYACCEPT;
	;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 150 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 153 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::eval,(yyvsp[(3) - (4)].obj),0);
	;}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 156 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::atoi,(yyvsp[(3) - (4)].obj),0);
	;}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 159 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::integer_dereference,(yyvsp[(2) - (2)].obj),0);
	;}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 164 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::array_indexing,(yyvsp[(2) - (2)].obj),0);
	;}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 167 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(1) - (4)].obj);
		(yyval.obj)->operands.push_back((yyvsp[(3) - (4)].obj));
	;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 171 "ExpressionParser.ypp"
    {
		(yyval.obj)=0;
		YYABORT;
	;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 177 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 180 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::string_dereference,(yyvsp[(2) - (2)].obj),0);
	;}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 183 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::itoa,(yyvsp[(3) - (4)].obj),0);
	;}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 186 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(2) - (3)].obj);
	;}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 189 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::concat,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 194 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 197 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 200 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::add,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 203 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::sub,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 206 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::mul,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 209 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::div,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 212 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::equals,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 215 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::nequals,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 218 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::lower,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 221 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::greatereq,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 224 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::greater,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 227 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::lowereq,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 230 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::fchk,(yyvsp[(2) - (2)].obj),0);
	;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 234 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::lchk,(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 237 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::lchk,(yyvsp[(2) - (2)].obj),0);
	;}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 240 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::or_operator,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 243 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::and_operator,(yyvsp[(1) - (3)].obj),(yyvsp[(3) - (3)].obj),0);
	;}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 246 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(2) - (2)].obj);
	;}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 249 "ExpressionParser.ypp"
    {
		(yyval.obj)=new NONS_Expression::Expression(NONS_Expression::neg,(yyvsp[(2) - (2)].obj),0);
	;}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 252 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(2) - (3)].obj);
	;}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 256 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(1) - (1)].obj);
	;}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 259 "ExpressionParser.ypp"
    {
		(yyval.obj)=(yyvsp[(2) - (3)].obj);
	;}
    break;



/* Line 1464 of yacc.c  */
#line 2029 "ExpressionParser.tab.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (stream, store, res, ppd, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (stream, store, res, ppd, yymsg);
	  }
	else
	  {
	    yyerror (stream, store, res, ppd, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, stream, store, res, ppd);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, stream, store, res, ppd);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (stream, store, res, ppd, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, stream, store, res, ppd);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, stream, store, res, ppd);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1684 of yacc.c  */
#line 262 "ExpressionParser.ypp"


#define DOUBLEOP(character,ret_value) if (c==(character)){ \
	stream->get();                                         \
	if ((wchar_t)stream->peek()==(character)){             \
		stream->get();                                     \
		return (ret_value);                                \
	}                                                      \
	return (ret_value);                                    \
}

template <typename T>
NONS_Expression::Expression *makeValue(const T &a){
	return new NONS_Expression::Expression(new NONS_Expression::Value(a));
}

int expressionParser_yylex(YYSTYPE *yylval,std::wstringstream *stream,NONS_VariableStore *store,PreParserData *ppd){
	if (ppd && ppd->trigger){
		ppd->trigger=0;
		if (ppd->mode==1)
			return IF;
		if (ppd->mode==2)
			return FOR;
	}
	int c;
	while (!stream->eof() && iswhitespace(wchar_t(c=(wchar_t)stream->peek())) && c<0x80)
		stream->get();
	if (stream->eof())
		return 0;
	c=(wchar_t)stream->peek();
	if (ppd && ppd->mode==1){
		if (multicomparison((wchar_t)c,L"\\@!#") || c>0x7F){
			if (c=='!'){
				stream->get();
				c=(wchar_t)stream->peek();
				if (stream->eof())
					stream->clear();
				stream->putback('!');
				if (c=='='){
					c='!';
					goto expressionParser_yylex_000;
				}
			}
			yylval->position=stream->tellg();
			stream->get();
			return IF_THEN;
		}
		if (c=='`'){
			
		}
	}
expressionParser_yylex_000:
	if (NONS_isdigit(c)){
		std::wstring temp;
		while (NONS_isdigit(c=(wchar_t)stream->peek()))
			temp.push_back((wchar_t)stream->get());
		//Handles the case "\*[ \t]*[_A-Za-z0-9]*". This is the kind of thing
		//that gives me omnicidal rages. Note: under no other circumstance does
		//it make sense that an integer be followed immediately by an alphabetic
		//character or an underscore.
		if (NONS_isid1char(c)){
			while (NONS_isidnchar((wchar_t)stream->peek()))
				temp.push_back((wchar_t)stream->get());
			yylval->obj=makeValue(temp);
			return STRING;
		}
		yylval->obj=makeValue(atol(temp));
		return INTEGER;
	}
	if (c=='#'){
		stream->get();
		std::wstring temp;
		while (NONS_ishexa((wchar_t)stream->peek()))
			temp.push_back((wchar_t)stream->get());
		if (temp.size()<6)
			return ERROR;
		long a=0;
		for (size_t b=0;b<temp.size();b++)
			a=(a<<4)+HEX2DEC(temp[b]);
		yylval->obj=makeValue(a);
		return INTEGER;
	}
	if (c=='\"' || c=='`' || NONS_tolower(c)=='e'){
		yylval->position=stream->tellg();
		c=(wchar_t)stream->get();
		bool cont=0,
			useEscapes=0;
		if (NONS_tolower(c)=='e'){
			if ((wchar_t)stream->peek()!='\"'){
				stream->putback(c);
				cont=1;
			}else{
				c=(wchar_t)stream->get();
				useEscapes=1;
			}
		}
		if (!cont){
			std::wstring temp;
			while ((wchar_t)stream->peek()!=c && !stream->eof()){
				wchar_t character=(wchar_t)stream->get();
				if (character=='\\' && useEscapes){
					character=(wchar_t)stream->get();
					switch (character){
						case '\\':
						case '\"':
							temp.push_back(character);
							break;
						case 'n':
						case 'r':
							temp.push_back('\n');
							break;
						case 't':
							temp.push_back('\t');
							break;
						case 'x':
							{
								std::wstring temp2;
								for (ulong a=0;NONS_ishexa((wchar_t)stream->peek()) && a<4;a++)
									temp2.push_back((wchar_t)stream->get());
								if (temp2.size()<4)
									return ERROR;
								wchar_t a=0;
								for (size_t b=0;b<temp2.size();b++)
									a=(a<<4)+HEX2DEC(temp2[b]);
								temp.push_back(a?a:32);
							}
							break;
						default:
							return ERROR;
					}
				}else
					temp.push_back(character);
			}
			if ((wchar_t)stream->peek()!=c){
				if (!ppd || ppd->mode!=1 || c!='`')
					handleErrors(NONS_UNMATCHED_QUOTES,0,"yylex",1);
				else
					return IF_THEN;
			}else
				stream->get();
			yylval->obj=makeValue(temp);
			return STRING;
		}
	}
	if (c=='*'){
		std::vector<wchar_t> backup;
		backup.push_back((wchar_t)stream->get());
		while (iswhitespace((wchar_t)stream->peek()))
			stream->get();
		std::wstring identifier;
		c=(wchar_t)stream->peek();
		if (NONS_isidnchar(c)){
			while (NONS_isidnchar(c=(wchar_t)stream->peek())){
				identifier.push_back(c);
				backup.push_back((wchar_t)stream->get());
			}
			if (gScriptInterpreter->script->blockFromLabel(identifier)){
				yylval->obj=makeValue(identifier);
				return STRING;
			}
		}
		if (stream->eof())
			stream->clear();
		while (backup.size()){
			stream->putback(backup.back());
			backup.pop_back();
		}
		c=(wchar_t)stream->peek();
	}
	if (NONS_isid1char(c)){
		std::wstring temp;
		yylval->position=stream->tellg();
		temp.push_back((wchar_t)stream->get());
		while (NONS_isidnchar((wchar_t)stream->peek()))
			temp.push_back((wchar_t)stream->get());
		if (!stdStrCmpCI(temp,L"fchk"))
			return FCHK;
		if (!stdStrCmpCI(temp,L"lchk"))
			return LCHK;
		if (!stdStrCmpCI(temp,L"_eval"))
			return EVAL;
		if (!stdStrCmpCI(temp,L"_itoa"))
			return ITOA;
		if (!stdStrCmpCI(temp,L"_atoi"))
			return ATOI;
		if (!stdStrCmpCI(temp,L"then")){
			while (iswhitespace(wchar_t(c=(wchar_t)stream->peek())) && c<0x80)
				stream->get();
			yylval->position=stream->tellg();
			return IF_THEN;
		}
		if (!stdStrCmpCI(temp,L"to"))
			return FOR_TO;
		if (!stdStrCmpCI(temp,L"step"))
			return FOR_STEP;
		
		NONS_VariableMember *member=store->getConstant(temp);
		if (!member)
			return IF_THEN;
		if (member->getType()==INTEGER){
			yylval->obj=makeValue(member->getInt());
			return INTEGER;
		}else{
			yylval->obj=makeValue(member->getWcs());
			return STRING;
		}
	}
	DOUBLEOP('=',EQUALS)
	DOUBLEOP('&',AND)
	DOUBLEOP('|',OR)
	if (c=='!'){
		stream->get();
		if ((wchar_t)stream->peek()=='='){
			stream->get();
			return NEQ;
		}
		return c;
	}
	if (c=='<'){
		stream->get();
		if ((wchar_t)stream->peek()=='='){
			stream->get();
			return LOWEREQ;
		}
		if ((wchar_t)stream->peek()=='>'){
			stream->get();
			return NEQ;
		}
		return LOWER;
	}
	if (c=='>'){
		stream->get();
		if ((wchar_t)stream->peek()=='='){
			stream->get();
			return GREATEREQ;
		}
		return GREATER;
	}
	if (!multicomparison((wchar_t)c,"+-*/[]%$?()")){
		wchar_t temp[]={c,0};
		handleErrors(NONS_UNRECOGNIZED_OPERATOR,0,"yylex",1,temp);
	}
	return (wchar_t)stream->get();
}

void expressionParser_yyerror(std::wstringstream *,NONS_VariableStore *,NONS_Expression::Expression *&,PreParserData *,char const *s){
	handleErrors(NONS_UNDEFINED_ERROR,0,"yyparse",1,UniFromISO88591((std::string)s));
}

