/*
 *
 *  Copyright (C) 2002-   Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<string>
#include<vector>
#include<map>

using namespace std;

#include"system/file.h"
#include"system/file_impl.h"

/* 注意点： @@@ で表記 */

struct VarInfo {
#define TYPE_VARSTR 18
#define TYPE_VARMAX 7
#define TYPE_STR 58
#define TYPE_VAL 68
#define TYPE_SYS 0xc8
	int type;
	int number;
	int value;
	VarInfo() { type = TYPE_VAL; value = 0;}
	VarInfo(int n) { type = TYPE_VAL; value = n;}
	VarInfo(const VarInfo& i) { type = i.type; number = i.number; value = i.value;}
};
class Flags {
/* flag:
**  type 0-5 : ローカル整数、各2000個
**  type 6, 25 : グローバル整数、2000個
**	type 12 : グローバル文字列、2000個 (今は無視しても良いが)
**	type 18 : ローカル文字列、2000個
**	type 25: システム変数（マウス座標など？） 1000 個？
**  type 26-32, 51 : 1-bit access to 0-6, 25
**  type 52-58, 77 : 2-bit access to 0-6, 25
**  type 78-84, 103 : 4-bit access to 0-6, 25
**  type 104-110, 129 : 8-bit access to 0-6, 25
*/
	typedef unsigned int uint;
	int sys;
	int var[8][2000];
	string str[2000];
public:
	int operator () () const;
	int operator () (VarInfo info) const;
	void Str(unsigned int number, char* buf, int sz) const;

	bool IsInt(int type) const;
	int MaxIndex(int type) const;

	void Set(VarInfo info, int value);
 	int Get(int type, int number) const;
	void SetSys(int value);
	void SetStr(unsigned int number, string val);
};

bool Flags::IsInt(int type) const {
	int v = type % 26;
	return v >= 0 && v < 7 || v == 25;
}

int Flags::MaxIndex(int type) const {
	switch (type / 26) {
		case 1:
			return 63999;
		case 2:
			return 31999;
		case 3:
			return 15999;
		case 4:
			return 7999;
		default:
			return 1999;
	}
}

int Flags::operator()() const {
	return rand() % 10000;
}
int Flags::operator() (VarInfo info) const {
	return Get(info.type, info.number);
}
 int Flags::Get(int type, int number) const {
	int index = type % 26;
	type /= 26;
	if (index == 25) index = 7;
	if (index > 7 || uint(type) > 4) return 0;
	if (type == 0) {
		// A[]..G[], Z[] を直に読む
		if (uint(number) >= 2000) return 0;
		return var[index][number];
	} else {
		// Ab[]..G4b[], Z8b[] などを読む
		int factor = 1 << (type - 1);
		int eltsize = 32 / factor;
		if (uint(number) >= (64000 / factor)) return 0;
		return (var[index][number / eltsize] >> ((number % eltsize) * factor)) & ((1 << factor) - 1);
	}
 }

void Flags::Set(VarInfo info, int value) {
	int type = info.type / 26;
	int index = info.type % 26;
	if (index == 25) index = 7;
	if (type == 0) {
		// A[]..G[], Z[] を直に書く
		if (uint(info.number) >= 2000) return;
		var[index][info.number] = value;
	} else {
		// Ab[]..G4b[], Z8b[] などを書く
		int factor = 1 << (type - 1);
		int eltsize = 32 / factor;
		int eltmask = (1 << factor) - 1;
		int shift = (info.number % eltsize) * factor;
		if (uint(info.number) >= (64000 / factor)) return;
		var[index][info.number / eltsize] =
			(var[index][info.number / eltsize] & ~(eltmask << shift))
		  | (value & eltmask) << shift;
	}
}

void Flags::SetSys(int value) {
	sys = value;
}
void Flags::SetStr(unsigned int number, string val) {
	if (number >= 2000) return;
	str[number] = val;
}
void Flags::Str(unsigned int number, char* buf, int sz) const {
	if (number >= 2000) {if(sz>0) buf[0] = 0; return;}
	const string& s = str[number];
	int len = s.length();
	if (sz-1 > len) sz = len;
	s.copy(buf, sz, 0);
	buf[sz] = 0;
	return;
}

/* commands */
enum Cmdtype { CMD_FLAGS, CMD_JMP, CMD_TEXT, CMD_OTHER};
class Cmd {
	Cmdtype cmd_type;
	int cmd1, cmd2, cmd3, cmd4;
	int argc;
	bool errorflag;
	char cmdstr[1024];
	const Flags& flags;
	vector<VarInfo> args;

	int GetArgs(const char*& d);
	int GetArgsSpecial(int normal_args,const char*& d);
	void GetSelection(const char*& d);
	int GetSwitch(const char*& d);
	int GetSimpleSwitch(const char*& d);
	int GetExpression(const char*& d, struct VarInfo* info = 0);
	int GetExpressionCond(const char*& d);
	int GetLeftToken(const char*& d, struct VarInfo& info);
	static int GetString(const char*& d);
	int StrVar(int number);
	static char strtype[256];
	static int StrType(const char* d) { return strtype[*(unsigned const char*)d];}
	int AddStr(char* s) {
		// 1-0a-0064 はこういうものが必要らしい
		int start = strend;
		while (*s) strheap[strend++] = *s++;
		strheap[strend++] = 0;
		return start;
	}
#define STRHEAP_SIZE 10000
	static char strheap[STRHEAP_SIZE];
	static int strend;
	void SetError(void) { errorflag = true;}
	static void ResetString(void) {
		strend = 0;
	}
	static map<int, struct CmdDescrItem*> cmd_descr;
	const char* CmdDescr(int cmd1, int cmd2, int cmd3, int cmd4);
public:
	void GetCmd(Flags& f, const char*& d);
	bool IsError() { return errorflag;}
	bool ClearError() { errorflag = false;}
	Cmd(const Flags& f) : flags(f) { argc = 0; errorflag = false; cmdstr[0] = 0;}

};

bool debug_flag = false;
void dprintf(const char* fmt, ...) {
	if (debug_flag) {
		va_list ap; va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
}


#define SCN_DUMP

int system_version = 0;
bool ruby_flag = false;
bool ret_flag = false;
bool text_flag = false;
bool selection_flag = false;
char Cmd::strheap[STRHEAP_SIZE];
int Cmd::strend = 0;

/* 数値 num := 0x24 0xff <int num> */
/* 変数 var := 0x24 <uchar type> 0x5b <exp> 0x5d */
/* 項 token := num | var | 0x28 <exp> 0x29 | <plus|minus> token */

int Cmd::GetLeftToken(const char*& d, VarInfo& info) {
	bool var_flag = true;
	int minus_flag = 0;
	int value = 0;
	if (d[0] == 0x5c && (d[1] == 1 || d[1] == 0) ) {
		if (d[1] == 1)	{dprintf("minus-"); minus_flag ^= 1;}
		else dprintf("plus-");
		d += 2;
		var_flag = false;
	}
	if (d[0] == 0x24 && ((unsigned const char*)d)[1] == 0xff) {
	// if ( (d[0] == 0x30 || d[0] == 0x31) && d[1] == 0x24 && ((unsigned const char*)d)[2] == 0xff) 	/* @@@ not supported; selection 内で、0x30|0x31 が付随することがある */
		// numerical atom
		d += 6;
		value = read_little_endian_int(d-4);
		dprintf("%d",value);
		var_flag = false;
	} else if (d[0] == 0x24 && *(unsigned char*)(d+1) == 0xc8) {
		dprintf("V<sys>");
		d += 2;
		info.type = TYPE_SYS; info.number = 0;
		value = info.value =  flags();
	} else if (d[0] == 0x24 && d[2] == 0x5b) {
		// 0x24,<type>,0x5b,<expr>,0x5d-terminated term
		info.type = *(unsigned char*)(d+1);
		d += 3;
		dprintf("V<%d>[",info.type);
		info.number = GetExpression(d);
		dprintf("]");
		if (*d == 0x5d) d++;
		else SetError();
		if (info.type == TYPE_VARSTR) {
			value = 0;
			info.value = StrVar(info.number);
		} else {
			value = info.value = flags(info);
		}
		dprintf("(=%d)",value);
	} else SetError();

	if (minus_flag) value = -value;
	if (!var_flag) {
		info.type = TYPE_VAL;
		info.value = value;
	}
	return value;
}

static char* op_str[70] = {
//	 0      1      2      3      4      5      6      7      8     9
	"+",   "-",   "*",   "/",   "%",   "&",   "|",   "^",   "<<",  ">>",	// +00
	"err.","err.","err.","err.","err.","err.","err.","err.","err.","err.",	// +10
	"+=",  "-=",  "*=",  "/=",  "%=",  "&=",  "|=",  "^=",  "<<=", ">>=",	// +20
	"=",   "err.","err.","err.","err.","err.","err.","err.","err.","err.",	// +30
	"==",  "!=",  "<=",  "<",   ">=",  ">",   "err.","err.","err.","err.",	// +40
	"err.","err.","err.","err.","err.","err.","err.","err.","err.","err.",	// +50
	"&&",  "||",  "err.","err.","err.","err.","err.","err.","err.","err.",	// +60
};

static int op_pri_tbl[12] = {
//	+  -  *  /  %  &  |  ^ << >>
	1, 1, 0, 0, 0, 3, 5, 4, 2, 2, 10, 10};

inline int op_pri(int op) {
	if (op > 11) return 10;
	return op_pri_tbl[op];
}
inline int op_pri_cond(int op) {
	if (op <= 11) return op_pri_tbl[op];
	else if (op < 50) return 7;
	else if (op == 60) return 8;
	else if (op == 61) return 9;
	else return 10;
}


inline int eval(int v1, int op, int v2) {
	switch(op) {
		case 0: return v1+v2;
		case 1: return v1-v2;
		case 2: return v1*v2;
		case 3: return v2!=0 ? v1/v2 : v1;
		case 4: return v2!=0 ? v1%v2 : v1;
		case 5: return v1&v2;
		case 6: return v1|v2;
		case 7: return v1^v2;
		case 8: return v1<<v2;
		case 9: return v1>>v2;
		case 40: return v1 == v2;
		case 41: return v1 != v2;
		case 42: return v1 <= v2;
		case 43: return v1 <  v2;
		case 44: return v1 >= v2;
		case 45: return v1 >  v2;
		case 60: return v1 && v2;
		case 61: return v1 || v2;
	}
	return v2;
}

/* 演算子 op := 0x5c <uchar op> */
/* 数式 exp: [op] <token> [op <token> [...]] */
int Cmd::GetExpression(const char*& d, VarInfo* info_ptr) {
#define STACK_DEPTH 1024
#define OP_LB 11
	char op_stack[STACK_DEPTH];
	int val_stack[STACK_DEPTH];
	int stack_count = 0;
	
	// 第一項の読み込み
	while(*d == 0x28) {
		d++;
		dprintf("(");
		op_stack[stack_count++] = OP_LB;
	}
	VarInfo info;
	int value = GetLeftToken(d, info);
	
	while(*d == 0x29 && stack_count > 0 && op_stack[stack_count-1] == OP_LB) {
		d++;
		dprintf(")");
		stack_count--;
	}
	
	if (*d != 0x5c && stack_count == 0) {
		if (info_ptr) *info_ptr = info;
		return value; // 単純なleft-termはここで終了。有効なinfo_ptrを帰す（可能性がある）
	}
	
	while(*d == 0x5c) {
		int op_type = *(unsigned char*)(d+1);
		d += 2;
		if (op_type < 70) dprintf("%s",op_str[op_type]);
		else dprintf("err.");
		if (op_type >= 10) SetError();
		int cur_pri = op_pri(op_type);
		while(stack_count != 0 && op_pri(op_stack[stack_count-1]) <= cur_pri) {
			// 優先順位の高い、先行する演算を行う
			value = eval(val_stack[stack_count-1], op_stack[stack_count-1], value);
			stack_count--;
		}
		val_stack[stack_count] = value;
		op_stack[stack_count++] = op_type;
		while(*d == 0x28) {
			d++;
			dprintf("(");
			op_stack[stack_count++] = OP_LB;
		}
		if (stack_count >= STACK_DEPTH) SetError();
		value = GetLeftToken(d, info);

		while (*d != 0x5c && stack_count > 0) {
			// 未実行の演算を終わらせる
			if (op_stack[stack_count-1] != OP_LB) {
				value = eval(val_stack[stack_count-1], op_stack[stack_count-1], value);
				stack_count--;
			} else if (*d == 0x29) { /* op_stack == OP_LB */
			// bracket 終端があれば、閉じておく
				d++;
				dprintf(")");
				stack_count--;
			} else break; // error
		}
	}
	if (stack_count) SetError(); // unbalanced bracket
	dprintf("(=%d)",value);
	if (info_ptr) {
		info_ptr->type = TYPE_VAL;
		info_ptr->value = value;
	}
	return value;
}

// 条件分岐専用に、条件演算と算術演算の混合を検知できる専用ルーチン（本来はGetExpressionで差し支えない)
int Cmd::GetExpressionCond(const char*& d) {
	char op_stack[STACK_DEPTH];
	int val_stack[STACK_DEPTH];
	int valattr_stack[STACK_DEPTH];
#define ATTR_VAL 0
#define ATTR_FLAG 1
	int stack_count = 0;
	
	// 第一項の読み込み
	while(*d == 0x28) {
		d++;
		dprintf("(");
		op_stack[stack_count++] = OP_LB;
	}
	VarInfo info;
	int value = GetLeftToken(d, info);
	bool valattr = ATTR_VAL;
	
	while(*d == 0x5c) {
		int op_type = *(unsigned char*)(d+1);
		d += 2;
		if (op_type < 70) dprintf("%s",op_str[op_type]);
		else dprintf("err.");
		int cur_pri = op_pri_cond(op_type);
		while(stack_count != 0 && op_pri_cond(op_stack[stack_count-1]) <= cur_pri) {
			// 優先順位の高い、先行する演算を行う
			if (op_stack[stack_count-1] >= 60) {
				if (valattr_stack[stack_count-1] != ATTR_FLAG || valattr != ATTR_FLAG) SetError();
			} else {
				if (valattr_stack[stack_count-1] != ATTR_VAL || valattr != ATTR_VAL) SetError();
			}
			value = eval(val_stack[stack_count-1], op_stack[stack_count-1], value);
			if (op_stack[stack_count-1] >= 40) valattr = ATTR_FLAG;
			stack_count--;
		}
		val_stack[stack_count] = value;
		valattr_stack[stack_count] = valattr;
		op_stack[stack_count++] = op_type;
		while(*d == 0x28) {
			d++;
			dprintf("(");
			op_stack[stack_count++] = OP_LB;
		}
		if (stack_count >= STACK_DEPTH) SetError();
		value = GetLeftToken(d, info);
		valattr = ATTR_VAL;

		while (*d != 0x5c && stack_count > 0) {
			// 未実行の演算を終わらせる
			if (op_stack[stack_count-1] != OP_LB) {
				if (op_stack[stack_count-1] >= 60) {
					if (valattr_stack[stack_count-1] != ATTR_FLAG || valattr != ATTR_FLAG) SetError();
				} else {
					if (valattr_stack[stack_count-1] != ATTR_VAL || valattr != ATTR_VAL) SetError();
				}
				value = eval(val_stack[stack_count-1], op_stack[stack_count-1], value);
				if (op_stack[stack_count-1] >= 40) valattr = ATTR_FLAG;
				stack_count--;
			// bracket 終端があれば、閉じておく
			} else if (*d == 0x29) { /* op_stack == OP_LB */
				d++;
				dprintf(")");
				stack_count--;
			} else break; // error
		}
	}
	if (stack_count) SetError(); // unbalanced bracket
	if (value) dprintf("(=true)");
	else dprintf("(=false)");
	return value;
}


/*
str = 
arg = 
args = 0x28 <exp> [[0x2c] <exp> [[0x2c] <exp> [...] ]]
*/

int Cmd::GetArgs(const char*& d) {
	if (*d != 0x28) return 0; /* 引数なし */
	d++;
	dprintf("args:");
	VarInfo var;
	int i; for (i=0; i<100 ; i++) {
		/* number, variable, string の種別なく値を得る */
		if (*d == 0x61) { // よくわからない(智代アフター)
			dprintf("@%d",d[1]);
			d += 2;
			if (*d == 0x28) {
				dprintf("{");
				GetArgs(d); // (A,B,C)節が含まれることがある
				dprintf("}");
			} else {
				dprintf("{}");
			}
		} else if (d[0] == 0x0a || d[0] == 0x40) { // よくわからない (Little Busters!)
			int var;
			if (system_version == 0) { var = read_little_endian_int(d+1); d += 5;}
			else { var = read_little_endian_short(d+1); d += 3;}
			dprintf("line %d; ",var);
		} else if (*d == 0x24 || (*d == 0x5c && (d[1] == 1 || d[1] == 0)) || *d == 0x28) {
			GetExpression(d, &var);
			args.push_back(var);
		} else if (StrType(d)) {
			var.type = TYPE_STR;
			var.value = GetString(d);
			args.push_back(var);
		} else SetError();
		if (*d == 0x29) break;
		if (*d == 0x2c) {d++;} // 次の arg が演算子で始まる、などがなければ存在しない
		dprintf(",");
	}
	if (*d == 0x29) d++;
	else SetError();
	return i;
}

int Cmd::GetArgsSpecial(int normal_args,const char*& d) {
	if (*d != 0x28) return 0; /* 引数なし */
	d++;
	dprintf("args:");
	int i; for (i=0; i<normal_args; i++) {
		/* number, variable, string の種別なく値を得る */
		if (*d == 0x24 || (*d == 0x5c && (d[1] == 1 || d[1] == 0)) || *d == 0x28) {
			GetExpression(d);
		} else if (StrType(d)) {
			GetString(d);
		} else SetError();
		if (*d == 0x29) break;
		if (*d == 0x2c) {d++;} // 次の arg が演算子で始まる、などがなければ存在しない
		dprintf(",");
	}
	for (i=0; i<argc ; i++) {
		if (*d == 0x28) {
/*
** cmd 01-22:0c1c, 01-22:0835
** Princess Bride のカードが落ちるアニメの場面
** なお、_PBCARDANM* の画像はこのコマンドでのみ使われているので、特殊処理として無視することも可能
**
** cmd 01-04:0276, 026c, 0270
** 複数の enum が args の数だけ続く処理。特殊処理として分離する
*/
dprintf("enum.<");
			/* (...) は列挙型 or 構造体の可能性がある */
			const char* d_orig = d;
			int pt = args.size(); args.push_back(VarInfo(0));
			int count = GetArgs(d);
			args[pt] = VarInfo(count);
dprintf(">");
		} else if (*d == 0x61 && (d[1] >= 0x00 && d[1] <= 0x04) && d[2] == 0x28 ) {
			/* 使われるコマンドは 01-21:004b, 01-28:0064 のいずれか（R,C,PB,LO)
			** それらのコマンドは
			** arg1: 画像ファイル名
			** arg2 : Sel 番号
			** らしく、arg3 以降が 0x61 <00-04> (a,b,c,...) となる（ダンプ上は enum と表記される)
			** () 内の引数はさまざまで、a のみ（画像ファイル名）、
			** a,b b=SEL?
			** a,b,c (b,c)=座標？
			** a,(b,c,d,e,f,g) b-g = src / dest?
			** らしい
			*/
			dprintf("kasane. #%d <",d[1]);
			d += 2;
			int pt = args.size(); args.push_back(VarInfo(0));
			int count = GetArgs(d);
			args[pt] = VarInfo(count);
			dprintf(">");
		} else if (*d == 0x24 || (*d == 0x5c && (d[1] == 1 || d[1] == 0))) {
			/* cmd 01-15:0028 ; 始めに 0x24 節があり、続いて 0x28 節になる */
			VarInfo var;
			GetExpression(d, &var);
			args.push_back(var);
			i--; // この引数はargc の数には入らない
		} else SetError();
		if (d[0] == 0x0a || d[0] == 0x40) {
			/* cmd 01-15:0028 ; 0x28 節の後に毎回 0x0a 節が来る */
			int var;
			if (system_version == 0) { var = read_little_endian_int(d+1); d += 5;}
			else { var = read_little_endian_short(d+1); d += 3;}
			dprintf("line %d; ",var);
		}
		if (*d == 0x29) break;
		if (*d == 0x2c) {d++;} // 次の arg が演算子で始まる、などがなければ存在しない
		dprintf(",");
	}
	if (*d == 0x29) d++;
	else SetError();
	return 0;
}

/* switch
	<exp>
	0x7b
		<exp> <int>
		...
	0x7d
*/

int Cmd::GetSwitch(const char*& d) {
	if (*d != 0x28) {SetError(); return -1;}
	d++;
	dprintf("switch. ");
	int var = GetExpression(d);
	if (*d != 0x29) {SetError(); return -1;}
	d++;
	dprintf("->\n");
	if (*d == 0x7b) {
		d++;
	} else SetError();

	int default_jmp = -1; int jmpto = -1;
	int i; for (i=0; i<argc; i++) {
		dprintf("\t");
		if (*d++ != 0x28) {SetError(); return -1;}
		int item = -1; // default
		if (*d != 0x29) {
			int item = GetExpression(d);
			if (*d++ != 0x29) {SetError(); return -1;}
			int jmp = read_little_endian_int(d);
			if (var == item) {
				dprintf("(selected)");
				jmpto = jmp;
			}
			dprintf(" -> %d\n", jmp);
		} else {
			d++;
			default_jmp = read_little_endian_int(d);
		}
		d += 4;
	}
	if (default_jmp != -1) {
		dprintf("default -> %d\n",default_jmp);
		if (jmpto == -1) jmpto = default_jmp;
	}
	if (*d == 0x7d) {
		d++;
	} else SetError();
	return jmpto;
}
/* simple switch
	<exp>
	0x7b
		<int>
		...
	0x7d
*/
int Cmd::GetSimpleSwitch(const char*& d) {
	if (*d != 0x28) {SetError(); return -1;}
	d++;
	dprintf("simple switch. ");
	int var = GetExpression(d);
	if (*d != 0x29) {SetError(); return -1;}
	d++;
	dprintf(" ->\n");
	int jumpto = -1;
	if (*d == 0x7b) {
		d++;
	} else SetError();
	int i; for (i=0; i<argc; i++) {
		int j = read_little_endian_int(d);
		d += 4;
		dprintf("\t%d -> %d\n", i+1, j);
		if (var == i+1) jumpto = j;
	}
	if (*d == 0x7d) {
		d++;
	} else SetError();
	return jumpto;
}

/*
selection
	? <exp>
	0x7b
		<0x0a|0x40> <ushort | uint> 
*/
void Cmd::GetSelection(const char*& d) {
	dprintf("selection. ");
	if (*d == 0x28) {
		d++;
		GetExpression(d);
		if (*d != 0x29) { SetError(); return;}
		d++;
	}
	if (*d == 0x7b) {
		d++;
		dprintf("{\n\t");
	} else SetError();
	int arg_count = 0;
	while(*d != 0x7d) {
		if (d[0] == 0x0a || d[0] == 0x40) {
			int var;
			if (system_version == 0) { var = read_little_endian_int(d+1); d += 5;}
			else { var = read_little_endian_short(d+1); d += 3;}
			dprintf("line %d; ",var);
		} else if (d[0] == 0x2c) {
			dprintf(":comma:");
		} else if (d[0] == 0x28) {
			dprintf(":cond:");
			d++;
			while(d[0] != 0x29) {
				GetExpressionCond(d); // PRINT- 節でないばあい、条件表示。次は文字節、またはPRINT節のはず
				if (IsError()) break;
				if (*d == 0x32) { d++; dprintf("##");} // 0x32 なら、現在の条件節を表示しない
				if (*d == 0x31) { d++; dprintf("**");} // 0x31 なら、現在の条件節を表示する(Little Busters! : 処理が正しいかは分からない)
				dprintf(":");
			}
			d++;
		} else if (StrType(d)) {
			GetString(d);
			arg_count++;
			dprintf("\n\t");
		} else if (*d == 0x23 && strncmp(d,"###PRINT",8) == 0) {
			d += 8;
			if (d[0] != 0x28) SetError();
			else { // 文字変数の内容の表示
				d++;
				dprintf("Print.");
				VarInfo info;
				GetLeftToken(d, info);
				if (d[0] != 0x29 || info.type == -1) SetError();
				d++;
				dprintf(";");
			}
		} else { SetError(); break;}
	}
	d++;
	/* @@@ */
	/* 一致しない場合があるのでコメントアウト */
	// if (arg_count != argc) SetError();
	dprintf("\n}\n");
	return;
}

char* op_str3[11] = { "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "="};
void Cmd::GetCmd(Flags& flags_orig, const char*& d ) {
	ResetString();

	cmdstr[0] = 0;
	debug_flag = true;
	if (*d == 0x23) { /* コマンド */
		cmd_type = CMD_OTHER;
		cmd1 = *(unsigned const char*)(d+1);
		cmd2 = *(unsigned const char*)(d+2);
		cmd3 = read_little_endian_short(d+3);
		argc = read_little_endian_short(d+5);
		cmd4 = *(unsigned const char*)(d+7);
		d += 8;
		/* verbose */
			// dprintf(" 0x23 - cmd %02x-%02x:%04x:%02x[%2d] \n",cmd1,cmd2,cmd3,cmd4,argc);
			sprintf(cmdstr, "%02x-%02x:%04x:%02x  : %s",cmd1,cmd2,cmd3,cmd4,CmdDescr(cmd1,cmd2,cmd3,cmd4));
		/* 引数を得る */
		/* 特殊引数のもの */
		int is_special = 0;
		if (cmd1 == 0) {
			if (cmd2 == 1) {
				int jump_arg = -1;
				if (cmd3 == 0 || cmd3 == 5) {
					/* gosub / goto */
					jump_arg =read_little_endian_int(d);
					d += 4;
					dprintf("\tjmp -> %d\n", jump_arg);
					is_special = 1;
				} else if (cmd3 == 1 || cmd3 == 2) {
					/* conditional jump (if / unless) */
					if (*d++ != 0x28) { SetError(); return;}
					dprintf("\t");
					int cond = GetExpressionCond(d);
					if (IsError()) return;
					if (*d++ != 0x29) { SetError(); return; }
					int jumpto = read_little_endian_int(d);
					d += 4;
					dprintf("-> %d\n", jumpto);
					if (cond) jump_arg = jumpto;
					is_special = 1;
				} else if (cmd3 == 4) {
					/* switch to */
					jump_arg = GetSwitch(d);
					is_special = 1;
				} else if (cmd3 == 16) {
					dprintf("local call with paramters;\n");
					GetArgs(d);
					int jumpto = read_little_endian_int(d);
					d += 4;
					dprintf("\tjmp -> %d\n",jumpto);
					is_special = 1;
				} else if (cmd3 == 8 || cmd3 == 3) {
					/* switch to */
					jump_arg = GetSimpleSwitch(d);
					dprintf("\tjmp -> %d\n",jump_arg);
					is_special = 1;
				}
				cmd_type = CMD_OTHER;
				args.push_back(VarInfo(jump_arg));
			} else if (cmd2 == 2 && (cmd3 == 0 || cmd3 == 1 || cmd3 == 2 || cmd3 == 3 || cmd3 == 0x0d) ) {
				/* selection */
				GetSelection(d);
				is_special = 1;
			}
		}
		/* 一般引数のもの */
		if (!is_special) {
			dprintf(" 0x23 - cmd %02x-%02x:%04x:%02x[%2d]  : %s\n",cmd1,cmd2,cmd3,cmd4,argc,CmdDescr(cmd1,cmd2,cmd3,cmd4));
			dprintf("\t");
			if (cmd1 == 1 && cmd2 == 0x22 && (cmd3 == 0xc1c || cmd3 == 0x835)) GetArgsSpecial(3, d);
			else if (cmd1 == 1 && cmd2 == 0x0b && cmd3 == 0x65) GetArgsSpecial(0, d);
			else if (cmd1 == 1 && cmd2 == 0x15 && cmd3 == 0x28) GetArgsSpecial(0, d);
			else if (cmd1 == 1 && cmd2 == 4 && (cmd3 == 0x26c || cmd3 == 0x26d || cmd3 == 0x270 || cmd3 == 0x276)) GetArgsSpecial(0, d);
			else if (cmd1 == 1 && (cmd2 == 0x21 && cmd3 == 0x4b) || (cmd2 == 0x28 && cmd3 == 0x64)) GetArgsSpecial(2,d);
			else GetArgs(d);
			dprintf("\n");

		}
		if (cmd2 == 3 && cmd3 == 0x78 && cmd4 == 0) ruby_flag = true;
		if (cmd2 == 3 && cmd3 == 0x11) ret_flag = true;
	} else if (*d == 0x24) { /* 代入演算 */
		if (d[1] == 0x12 || d[2] != 0x5b) SetError();
		dprintf("expr: ");
		sprintf(cmdstr, "expr");

		VarInfo info;
		int value = GetLeftToken(d, info);
		if (d[0] != 0x5c) SetError();
		int type = d[1];
		if (type < 20 || type > 30) SetError();
		else dprintf("%s",op_str[type]);
		d += 2;
		int value2 = GetExpression(d);
		// 代入情報を埋め込む
		if (type != 30) value2 = eval(value, type-20, value2);
		cmd_type = CMD_FLAGS;
		args.push_back(info);
		args.push_back(value2);
		dprintf("\n");
	} else if (StrType(d)) { /* 文字出力 */
		VarInfo info;
		info.type = TYPE_STR;
		info.value = GetString(d);
		args.push_back(info);
		cmd_type = CMD_TEXT;
		text_flag = true;
		dprintf("\n");
	} else if (*d == 0x0a || *d == 0x40 || *d == 0x21) { /* デバッグ用データと既読フラグ */
		cmd_type = CMD_OTHER;
		if (*d == 0x0a) {
			dprintf("line ");
			d++;
			int l;
			if (system_version == 0) {
				l = read_little_endian_int(d);
				d += 4;
			} else {
				l = read_little_endian_short(d);
				d += 2;
			}
			dprintf("%d\n", l);
		} else { /* 0x40, 0x21 */
			// 既読マーカーらしい。エントリーポイントとセーブポイントも使われる。
			// RealLive 1.2.5から、0x40はセーブポイント、0x21はエントリーポイント。
			// 1.2.5以前、どちらも0x40が使われる。
			int kidoku_index;
			d++;
			if (system_version == 0) {
				kidoku_index = read_little_endian_int(d);
				d += 4;
			} else {
				kidoku_index = read_little_endian_short(d);
				d += 2;
			}
			dprintf("kidoku marker %d\n", kidoku_index);
			// text_readflagは、このkidoku_indexを使ったら良いかな。
		}
	} else if (*d == 0x2c) { /* ??? */
		dprintf("commd;0x2c\n"); // conditional jump の行き先によくあるらしい（常に、かはわからない）
		d++;
	} else { 
		SetError();
	}
	return;
}

char Cmd::strtype[256] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +00 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +10 */
	0,0,3,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +20 */
	1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,1, /* +30 */
	0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, /* +40 */
	1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,1, /* +50 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +60 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +70 */
	2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2, /* +80 */
	2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2, /* +90 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +A0 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +B0 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +C0 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +D0 */
	2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2, /* +E0 */
	2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,0,0  /* +F0 */
};

int Cmd::GetString(const char*& d) {
	int retnum = -1;
	while(1) {
		if (*d == '\\') {
			d++;
			strheap[strend++] = *d++;
		} else if (*d == '"') {
			d++;
			if (retnum == -1) retnum = strend;
			while(*d != '"') strheap[strend++] = *d++;
			d++;
		} else if (StrType(d)) {
			if (retnum == -1) retnum = strend;
			int stype;
			while( (stype = StrType(d)) ) {
				if (stype == 3) break; // 文中に '"' が現れた場合
				strheap[strend++] = *d++;
				if (stype == 2) strheap[strend++] = *d++;
			}
		} else break;
	}
	if (retnum != -1) strheap[strend++] = 0;
	dprintf("\"%s\"", strheap + retnum);
	if (strend >= STRHEAP_SIZE) {
		dprintf("Error: string heap overflow\n");
	}
	return retnum;
}

int Cmd::StrVar(int var_num) {
	int retnum = strend;
	flags.Str(var_num, strheap+strend, STRHEAP_SIZE-strend);
	strend += strlen(strheap+strend)+1;
	return retnum;
}

void usage(void) {
	fprintf(stderr,"usage : scn2kdump [inputfile] [outputfile]\n");
	fprintf(stderr,"  inputfile:  seen.txt(default)\n");
	fprintf(stderr,"  outputfile: seen.txt_out(default)\n");
	exit(-1);
}
int main(int argc, char** argv) {
	/* determine file names */
	bool verbose = false;
	char* inname = "seen.txt";
	char* outname = 0;
	if (argc > 2 && strcmp(argv[1],"-v") == 0) {
		int i; for (i=1; i<argc; i++) argv[i] = argv[i+1];
		argc--;
		verbose = true;
	}
	switch(argc) {
	case 1: break;
	case 2: inname = argv[1]; break;
	case 3: inname = argv[1]; outname = argv[2]; break;
	default: usage();
	}
	/* open output file */
	FILE* outstream = stdout;
	/* create archive instance */
	SCN2kFILE archive(inname);
	archive.Init();
	if (archive.Deal() == 0) {
		fprintf(stderr,"Cannot open / Invalid archive file %s\n",inname);
		usage();
	}
	/* dump files */
	archive.InitList();
	char* fname;
	fprintf(stderr,"Dump start\n");
	while( (fname = archive.ListItem()) != 0) {
		ARCINFO* info = archive.Find(fname,"");
		if (info == 0) continue;
		char* data = info->CopyRead();
		char* d = data;
		char* dend = d + info->Size();
		/* version 確認 */
		if (read_little_endian_int(d) == 0x1cc) {
			system_version = 0;
		} else if (read_little_endian_int(d) == 0x1d0) {
			system_version = 1;
		} else {
			continue;
		}
		if (read_little_endian_int(d+4) == 0x1adb2) ; // little busters!
		else if (read_little_endian_int(d+4) != 0x2712) continue;
		int header_size;
		if (system_version == 0) {
			header_size = 0x1cc + read_little_endian_int(d+0x20) * 4;
		} else {
			header_size = read_little_endian_int(d+0x20);
		}
		d += header_size;

		const char* dcur = d;
		const char* dstart = d;
		fprintf(stderr,"Dumping %s\n",fname);
		fprintf(stdout,"Dumping %s\n",fname);
{ int i; for (i=0; i<100; i++) {
		int n = read_little_endian_int(data + 0x34 + i*4);
		if (n != 6) fprintf(stdout,"subroutine table %2d: %6d\n",i,n);
}}
		Flags flags;
		/* 最初から最後までコマンド取得 -> 出力を繰り返す */
		while(dcur<dend) {
			const char* dprev = dcur;
			Cmd cmd(flags); cmd.ClearError();

			/* end? */
			if (*dcur == -1) {
				/* 0xff x 32byte + 0x00 : end sign */
				int i; for (i=0; i<0x20; i++)
					if (dcur[i] != -1) break;
				if (i == 0x20 && dcur[i] == 0) break;
			}
			dprintf("%d : ",dcur-dstart);
			cmd.GetCmd(flags, dcur);
			if (cmd.IsError()) {
				fprintf(outstream, "Error at %6d\n",dprev-dstart);
				while(dcur < dend) {
					if (*dcur == 0x29 && dcur[1] == 0x0a) {dcur++;break;}
					if (*dcur == 0 && dcur[1] == 0x0a) {dcur++;break;}
					if (*dcur == 0 && dcur[1] == 0x23) {dcur++;break;}
					dcur++;
				}
				dprev -= 2*16;
				int ilen = (dcur-dprev+15)/16;
				int i; for (i=0; i<ilen; i++) {
					fprintf(outstream, "%6d: ",dprev-dstart);
					int j; for (j=0; j<16; j++) {
						if (dprev >= dend) break;
						if (dprev < data) continue;
						fprintf(outstream, "%02x ",*(unsigned char*)(dprev));
						dprev++;
					}
					fprintf(outstream, "\n");
				}
			}
		}
		delete info;
	}
	return 0;
}

/*
SetStr
	0x23 - cmd 01-0a:0000:00[ 2] 
	args:V<18>[17],"PB47"
CatStr
	0x23 - cmd 01-0a:0002:00[ 2] 
	args:V<18>[17],V<18>[20]

WaitClick
	0x23 - cmd 00-03:0011:00[ 0] 
	
ChangeFaceGraphics
	0x23 - cmd 00-03:03e8:00[ 1] 
	args:V<18>[17]
DeleteFaceGraphics
	0x23 - cmd 00-03:03e9:01[ 0] 
KoePlay
	0x23 - cmd 01-17:0000:01[ 2] 
	args:100000026,5
DrawGraphics(前景画あり）
	0x23 - cmd 01-21:004b:00[ 1] 
	args:V<18>[1],10,kasane. #1 <args:V<18>[17],11>

DrawGraphics(背景のみ)
	0x23 - cmd 01-21:0049:00[ 2] 
	args:V<18>[1],10
	
Ruby
	0x23 - cmd 00-03:0078:01[ 0] 
	"理由"
	0x23 - cmd 00-03:0078:00[ 1] 
	"わけ"
SetTitle
	0x23 - cmd 01-04:0000:00[ 1] 
	args:"Long Long Time Ago..."
WaitTime
	0x23 - cmd 01-14:0069:00[ 1] 
	args:3000
ChangeBGM	数値引数はフェードアウト、インの時間と推測
0x23 - cmd 01-14:0000:02[ 3] 
	args:"BGM18",700,700
*/

struct CmdDescrItem {
	CmdDescrItem* next;
	int cmd4;
	int cmd1;
	int cmd2;
	int cmd3;
	const char* cmd_descr;
};
CmdDescrItem cmd_descr_orig[] = {
	// scn2k_impl.cc; Scn2k::SysExec()
	{0,0,0x00,0x01,0x0a, "local return"},
	{0,0,0x00,0x01,0x0b, "global jump"},
	{0,0,0x00,0x01,0x0c, "global call"},
	{0,0,0x00,0x01,0x0d, "global return"},
	{0,0,0x00,0x01,0x12, "global call"},
	{0,0,0x00,0x01,0x13, "global return(?)"},
	{0,0,0x00,0x04,0x0d, "Menu_return"},
	{0,0,0x01,0x04,0x00, "SetWindowCaption"},
	{0,0,0x01,0x04,0x82, "ClearMousePress"},
	{0,0,0x01,0x04,0x83, "GetMouse(2)"},
	{0,0,0x01,0x04,0x85, "GetMouse"},
	{0,0,0x01,0x04,0x4b0,"QuitGame"},
	{0,0,0x01,0x04,0x58d,"PrevSaveNumber"},
	{0,0,0x01,0x04,0x585,"SavedDate"},
	{0,0,0x01,0x04,0xc23,"Save"},
	{0,0,0x01,0x04,0xc25,"Load"},
	{0,0,0x01,0x04,0x4b1,"GoMenu"},
	{0,0,0x01,0x04,0x4b3,"GoMenu_Badend"},
	{0,0,0x01,0x04,0xcc, "ShowMouseCursor"},
	{0,0,0x01,0x04,0xcd, "HideMouseCursor"},
	{0,0,0x01,0x04,0xcf, "SetCursorType"},
	// scn2k_cmd.cc; Cmd::GetCmd()
	{0,0,0x00,0x01,0,    "local jump"},
	{0,0,0x00,0x01,1,    "local jump-if"},
	{0,0,0x00,0x01,2,    "local jump-unless"},
	{0,0,0x00,0x01,3,    "local jump-switch??"},
	{0,0,0x00,0x01,4,    "local switch"},
	{0,0,0x00,0x01,5,    "local call"},
	{0,0,0x00,0x01,8,    "local switch(simple form)"},
	{0,0,0x01,0x0b,0,    "set multiple variables"},
	{0,0,0x01,0x0b,1,    "set variables in a range"},
	{0,0,0x01,0x0b,4,    "clear variables in a range"},
	{0,0,0x01,0x0b,0x64, "get summation of variables in a range"},
	// scn2k_cmd.cc; Flags::Exec()
	{0,0,0x01,0x0a,0,    "SetStr"},
	{0,0,0x01,0x0a,1,    "ClearStr"},
	{0,0,0x01,0x0a,2,    "AppendStr"},
	{0,0,0x01,0x0a,3,    "StrLen"},
	{0,0,0x01,0x0a,4,    "StrCmp"},
	{0,0,0x01,0x0a,5,    "SubStrL"},
	{0,0,0x01,0x0a,6,    "SubStrR"},
	{0,0,0x01,0x0a,7,    "StrLenWideChar"},
	{0,0,0x01,0x0a,8,    "TrimStr"},
	{0,0,0x01,0x0a,0x0f, "IntToStr"},
	{0,0,0x01,0x0a,0x11, "IntToStr_Fill"},
	{0,0,0x01,0x0a,0x64, "ShowStr"},
	// scn2k_text.cc; TextImpl::Exec()
	{0,0,0x01,0x21,0x49, "SetFaceGraphic"},
	{0,0,0x01,0x21,0x4b, "SetFaceGraphic"},
	{0,0,0x01,0x21,0x4c, "SetFaceGraphic"},
	{0,0,0x00,0x03,0x97, "CloseTextWindow"},
	{0,0,0x00,0x03,0x11, "WaitText"},
	{0,0,0x00,0x03,0x03, "TextReturn"},
	{0,0,0x00,0x03,0xc9, "TextReturn"},
	{0,0,0x00,0x03,0x3e8,"SetFaceGraphic"},
	{0,0,0x00,0x03,0x3e9,"SetFaceGraphic"},
	{0,0,0x00,0x03,0x78, "TextRuby"},
	{0,0,0x00,0x03,0x66, "SetTextWindowType"},
	{0,0,0x00,0x03,0x67, "OpenTextWindow"},
	{0,0,0x00,0x03,0x98, "ClearTextWindow"},
	{0,0,0x00,0x03,0x68, "ShowText"},
	{0,0,0x00,0x02,0x01, "Select"},
	{0,0,0x00,0x02,0x03, "Select"},
	{0,0,0x00,0x04,0x44c,"TextSkipStart"},
	{0,0,0x00,0x04,0x3e8,"CloseTextWindow"},
	{0,0,0x01,0x04,0x64, "WaitTime"},
	{0,0,0x01,0x04,0x6f, "WaitTime"},
	{0,0,0x01,0x04,0x79, "WaitTime"},
	{0,0,0x01,0x04,0x65, "WaitTime w/ Cancel"},
	{0,0,0x01,0x04,0x70, "WaitTime w/ Cancel"},
	{0,0,0x01,0x04,0x1fe,"GetTimer"},
	{0,0,0x01,0x04,0x201,"ResetTimer (unsupported; see rldev)"},
	{0,0,0x01,0x04,0x202,"ResetTimerAll (unsupported; see rldev)"},
	{0,0,0x01,0x04,0x72, "GetTimer"},
	{0,0,0x01,0x04,0x7c, "GetTimer(2)"},
	{0,0,0x01,0x04,0x6e, "ClearTimer"},
	{0,0,0x01,0x04,0x78, "ClearTimer(2)"},
	{0,0,0x01,0x04,0x26c,"ClearTimer(multi)"},
	{0,0,0x01,0x04,0x270,"ClearTimer(multi)"},
	{0,0,0x01,0x04,0x276,"GetTimer(multi)"},
	{0,0,0x01,0x04,0x1f4,"SetTimer"},
	{0,0,0x01,0x04,0x3e8,"rand(x,y)"},
	{0,0,0x01,0x04,0x3ec,"min(x,y)"},
	{0,0,0x01,0x04,0x3ef,"min(x,y)"},
	{0,0,0x01,0x04,0x320,"range conversion(V,?,ResultMin,ValMin,ValMax,ResultMax,?)"},
	{0,0,0x01,0x04,0x3f1,"in_range(x,y,a)"},
	{0,0,0x01,0x04,0x16c,"SetCursorType?"},
	{0,0,0x01,0x04,0xbc1,"LoadFromMenu"},
	{0,0,0x01,0x04,0x8d4,"SetTextWindowColor"},
	{0,0,0x01,0x04,0x8d5,"SetTextWindowColor"},
	{0,0,0x01,0x04,0x8d6,"SetTextWindowColor"},
	{0,0,0x01,0x04,0x8d7,"SetTextWindowColor"},
	{0,0,0x01,0x04,0x8d8,"SetTextWindowColor"},
	{0,0,0x01,0x04,0x8db,"SetTextWindowColor"},
	{0,0,0x01,0x04,0x93f,"SetTextWindowColor"},
	{0,0,0x01,0x04,0xa39,"SetTextWindowColor"},
	{0,0,0x01,0x04,0xa28,"Get #INIT_MESSAGE_SPEED (original) from gameexe.ini"},
	{0,0,0x01,0x04,0xa29,"Get #INIT_MESSAGE_SPEED (original) from gameexe.ini"},
	{0,0,0x01,0x04,0xa2c,"Get #MESSAGE_KEY_WAIT_USE (original) from gameexe.ini"},
	{0,0,0x01,0x04,0xa2d,"Get #INIT_MESSAGE_SPEED (original) from gameexe.ini"},
	{0,0,0x01,0x04,0xa2e,"Get #MESSAGE_KEY_WAIT_TIME (original) from gameexe.ini"},
	{0,0,0x01,0x04,0x913,"Get #INIT_MESSAGE_SPEED"},
	{0,0,0x01,0x04,0x914,"Get #INIT_MESSAGE_SPEED_MOD"},
	{0,0,0x01,0x04,0x92e,"Get #MESSAGE_KEY_WAIT_USE"},
	{0,0,0x01,0x04,0x92f,"Get #INIT_MESSAGE_SPEED_MOD"},
	{0,0,0x01,0x04,0x930,"Get #MESSAGE_KEY_WAIT_TIME"},
	{0,0,0x01,0x04,0x8af,"Set #INIT_MESSAGE_SPEED"},
	{0,0,0x01,0x04,0x8b0,"Set #INIT_MESSAGE_SPEED_MOD"},
	{0,0,0x01,0x04,0x8ca,"Set #MESSAGE_KEY_WAIT_USE"},
	{0,0,0x01,0x04,0x8cb,"Set #INIT_MESSAGE_SPEED_MOD"},
	{0,0,0x01,0x04,0x8cc,"Set #MESSAGE_KEY_WAIT_USE"},
	{0,0,0x01,0x04,0x51f,"Set Name Text"},
	{0,0,0x01,0x04,0x51e,"Get Name Text"},
	{0,0,0x01,0x04,0x514,"Get Name Text"},
	// scn2k_grp.cc; GrpImpl::Exec()
	// music commands
	{0,0,0x01,0x14,0,    "PlayBGM"},
	{0,0,0x01,0x14,2,    "PlayBGM"},
	{0,0,0x01,0x14,0x05, "StopBGM"},
	{0,0,0x01,0x14,0x69, "FadeBGM"},
	{0,0,0x01,0x15,0,    "PlaySE"},
	{0,0,0x01,0x15,2,    "PlaySE"},
	{0,0,0x01,0x17,0,    "PlayKoe"},
	{0,0,0x01,0x1a,1,    "PlayMovie"},
	{0,0,0x01,0x1a,0x14, "PlayMovie"},
	// graphic commands
	{0,0,0x01,0x1e,0,    "GraphicStackClear"},
	{0,0,0x01,0x1f,0,    "GraphicStackClear"},
	{0,0,0x01,0x21,0x46, "LoadSurface"},
	{0,0,0x01,0x21,0x49, "LoadBackSurface"},
	{0,0,0x01,0x21,0x4b, "LoadForeSurface"},
	{0,0,0x01,0x21,0x4c, "LoadSurface"},
	{0,0,0x01,0x21,0x64, "CopySurface"},
	{0,0,0x01,0x21,0x4b1,"ClearSurface"},
	{0,0,0x01,0x21,0x44c,"AlphaCopy"},
	{0,0,0x01,0x21,0x640,"SaturateCopy"},
	{0,0,0x01,0x21,0x196,"??? grp"},
	{0,0,0x01,0x22,0xc30,"ScrollEffect (Princess Bride)"},
	{0,0,0x01,0x22,0xc1c,"FallEffect (Princess Bride)"},
	{0,0,0x01,0x22,0x835,"FallEffect (Princess Bride)"},
	// grphic object commands
	{0,0,0x01,0x04,0xd2, "??? grp"},
	{0,0,0x01,0x04,0xd3, "??? grp"},
	{0,0,0x01,0x04,0xd7, "??? grp"},
	{0,0,0x01,0x04,0xd8, "??? grp"},
	{0,0,0x01,0x04,0x5e0,"GetShownGrpFlag"},
	{0,0,0x01,0x3d,0x0a, "ClearGrpObj"},
	{0,0,0x01,0x3d,0x0b, "ClearGrpObj"},
	{0,0,0x01,0x3e,0x0a, "ClearGrpObj"},
	{0,0,0x01,0x3e,0x0a, "ClearGrpObj"},
	{0,0,0x01,0x3c,0x01, "??? grp (CLANNAD)"},
	{0,0,0x01,0x47,0x3e8,"SetGrpObj_Fname"},
	{0,0,0x01,0x47,0x3eb,"SetGrpObj_GANname"},
	{0,0,0x01,0x47,0x4b0,"SetGrpObj_Text"},
	{0,0,0x01,0x48,0x3e8,"SetGrpObj_ForeGrp?"},
	{0,0,0x01,0x49,0,    "StopAnimation"},
	{0,0,0x01,0x49,3,    "QueryExecAnimation"},
	{0,0,0x01,0x49,0x7d3,"SetGrpObj_GAN?"},
	{0,0,0x01,0x49,0xbb9,"StartAnimation"},
	{0,0,0x01,0x49,0xbbb,"StartAnimation"},
	{0,0,0x01,0x49,0xbbd,"StartAnimation"},
	{0,0,0x01,0x51,0x3e8,"SetGrpObj_xy"},
	{0,0,0x01,0x51,0x3e9,"SetGrpObj_x"},
	{0,0,0x01,0x51,0x3ea,"SetGrpObj_y"},
	{0,0,0x01,0x51,0x3eb,"SetGrpObj_alpha"},
	{0,0,0x01,0x51,0x3ec,"SetGrpObj_visible"},
	{0,0,0x01,0x51,0x3ee,"SetGrpObj_xy?"},
	{0,0,0x01,0x51,0x3fd,"SetGrpObj_centering?"},
	{0,0,0x01,0x51,0x401,"SetGrpObj_textsize"},
	{0,0,0x01,0x51,0x40a,"SetGrpObj_clipregion"},
	{0,0,0x01,0x51,0x40f,"SetGrpObj_surfacenum"},
	{0,0,0x01,0x51,0x416,"SetGrpObj_expand"},
	{0,0,0x01,0x51,0x419,"SetGrpObj_rotate"},
	{0,0,0x01,0x52,0x3e8,"SetGrpObj_xy(2)"},
	{0,0,0x01,0x52,0x3ea,"SetGrpObj_y(2)"},
	{0,0,0x01,0x52,0x3eb,"SetGrpObj_alpha(2)"},
	{0,0,0x01,0x52,0x3ec,"SetGrpObj_visible(2)"},
	{0,0,0x01,0x52,0x3ee,"SetGrpObj_xy?(2)"},
	{0,0,0x01,0x52,0x3fd,"SetGrpObj_centering?(2)"},
	{0,0,0x01,0x52,0x401,"SetGrpObj_textsize(2)"},
	{0,0,0x01,0x52,0x408,"SetGrpObj_order (not supported)"},
	{0,0,0x01,0x52,0x40a,"SetGrpObj_clipregion(2)"},
	{0,0,0x01,0x52,0x40f,"SetGrpObj_surfacenum(2)"},
	{0,0,0x01,0x52,0x416,"SetGrpObj_expand(2)"},
	{0,0,0x01,0x52,0x419,"SetGrpObj_rotate(2)"},
	{0,0,0x01,0x54,0x3e8,"GetGrpObj_xy"},
	{0,0,0x01,0x54,0x44c,"GetGrpObj_wh"},

	{0,0,0x02,0x3d,0x0a, "ClearGrpObj(2)"},
	{0,0,0x02,0x3d,0x0b, "ClearGrpObj(2)"},
	{0,0,0x02,0x3e,0x0a, "ClearGrpObj(2)"},
	{0,0,0x02,0x3e,0x0a, "ClearGrpObj(2)"},
	{0,0,0x02,0x3c,0x01, "??? grp (CLANNAD)(2)"},
	{0,0,0x02,0x47,0x3e8,"SetGrpObj_Fname(2)"},
	{0,0,0x02,0x47,0x3eb,"SetGrpObj_GANname(2)"},
	{0,0,0x02,0x47,0x4b0,"SetGrpObj_Text(2)"},
	{0,0,0x02,0x48,0x3e8,"SetGrpObj_ForeGrp?(2)"},
	{0,0,0x02,0x49,0,    "StopAnimation(2)"},
	{0,0,0x02,0x49,3,    "QueryExecAnimation(2)"},
	{0,0,0x02,0x49,0x7d3,"SetGrpObj_GAN?(2)"},
	{0,0,0x02,0x49,0xbb9,"StartAnimation(2)"},
	{0,0,0x02,0x49,0xbbb,"StartAnimation(2)"},
	{0,0,0x02,0x49,0xbbd,"StartAnimation(2)"},
	{0,0,0x02,0x51,0x3e8,"SetGrpObj_xy(2)"},
	{0,0,0x02,0x51,0x3ea,"SetGrpObj_y(2)"},
	{0,0,0x02,0x51,0x3eb,"SetGrpObj_alpha(2)"},
	{0,0,0x02,0x51,0x3ec,"SetGrpObj_visible(2)"},
	{0,0,0x02,0x51,0x3ee,"SetGrpObj_xy?(2)"},
	{0,0,0x02,0x51,0x3fd,"SetGrpObj_centering?(2)"},
	{0,0,0x02,0x51,0x401,"SetGrpObj_textsize(2)"},
	{0,0,0x02,0x51,0x40a,"SetGrpObj_clipregion(2)"},
	{0,0,0x02,0x51,0x40f,"SetGrpObj_surfacenum(2)"},
	{0,0,0x02,0x51,0x416,"SetGrpObj_expand(2)"},
	{0,0,0x02,0x51,0x419,"SetGrpObj_rotate(2)"},
	{0,0,0x02,0x52,0x3e8,"SetGrpObj_xy(2)(2)"},
	{0,0,0x02,0x52,0x3ea,"SetGrpObj_y(2)(2)"},
	{0,0,0x02,0x52,0x3eb,"SetGrpObj_alpha(2)(2)"},
	{0,0,0x02,0x52,0x3ec,"SetGrpObj_visible(2)(2)"},
	{0,0,0x02,0x52,0x3ee,"SetGrpObj_xy?(2)(2)"},
	{0,0,0x02,0x52,0x3fd,"SetGrpObj_centering?(2)(2)"},
	{0,0,0x02,0x52,0x401,"SetGrpObj_textsize(2)(2)"},
	{0,0,0x02,0x52,0x40a,"SetGrpObj_clipregion(2)(2)"},
	{0,0,0x02,0x52,0x40f,"SetGrpObj_surfacenum(2)(2)"},
	{0,0,0x02,0x52,0x416,"SetGrpObj_expand(2)(2)"},
	{0,0,0x02,0x52,0x419,"SetGrpObj_rotate(2)(2)"},
	{0,0,0x02,0x54,0x3e8,"GetGrpObj_xy(2)"},
	{0,0,0x02,0x54,0x44c,"GetGrpObj_wh(2)"},
	{0,0,0,0,0,0}
};
map<int, CmdDescrItem*> Cmd::cmd_descr;
const char* Cmd::CmdDescr(int cmd1, int cmd2, int cmd3, int cmd4) {
	if (cmd_descr.empty()) {
		int i;
		for (i=0; cmd_descr_orig[i].cmd_descr != 0; i++) {
			CmdDescrItem& cur = cmd_descr_orig[i];
			int item_num = cur.cmd1*1000000+cur.cmd2*10000+cur.cmd3;
			map<int,CmdDescrItem*>::iterator it = cmd_descr.find(item_num);
			if (it == cmd_descr.end()) cmd_descr[item_num] = &cur;
			else {
				cur.next = it->second;
				it->second = &cur;
			}
		}
	}
	int item_num = cmd1*1000000+cmd2*10000+cmd3;
	map<int,CmdDescrItem*>::iterator it = cmd_descr.find(item_num);
	if (it == cmd_descr.end()) return "No descr (unsupported)";
	CmdDescrItem* cur = it->second;
	do {
		if (cur->cmd1 == cmd1 && cur->cmd2 == cmd2 && cur->cmd3 == cmd3) {
			return cur->cmd_descr;
		}
		cur = cur->next;
	} while(cur != 0);
	return "No descr (unsupported)";
}
