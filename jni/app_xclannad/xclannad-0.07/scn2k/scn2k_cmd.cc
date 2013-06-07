/*
 * Copyright (c) 2004-2006  Kazunori "jagarl" Ueno
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include"scn2k.h"

#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<string>
#include"system/file.h"
#include <ctype.h>

using namespace std;


// #define SCN_DUMP
/* 注意点： @@@ で表記 */



//bool debug_flag = true;
bool debug_flag = false;
void dprintf(const char* fmt, ...) {
	if (debug_flag) {
		va_list ap; va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
}


void eprintf(const char* fmt, ...) {
	va_list ap; va_start(ap, fmt);
//	vprintf(fmt, ap);
	va_end(ap);
}

/**************************************************************
**	Flag
*/

Flags::Flags(void) {
	int i,j;
	for (i=0; i<=TYPE_VARMAX; i++) {
		for (j=0; j<2000; j++) {
			var[i][j] = 0;
		}
	}
	sys = 0;
}

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
	return sys; // rand() % 10000;
}

int Flags::operator() (VarInfo info) const {
	return Get(info.type, info.number);
}

int Flags::Get(int type, int number) const {
	int index = type % 26;
	type /= 26;
	if (index == 25) {
		if (var[7][number] != 0) return var[7][number];
		if (cgm_data.find(number) == cgm_data.end()) return 0;
		else return 1;
	}
	if (index == 10) index = 8;
	if (index == 11) index = 9;
	if (index > TYPE_VARMAX || uint(type) > 4) return 0;
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
	if (index == 25) {
		if (uint(info.number) >= 2000) return;
		if (value == 0)
			cgm_data.erase(info.number);
		else
			cgm_data.insert(info.number);
		index = 7;
	}
	if (index == 10) index = 8;
	if (index == 11) index = 9;
	if (index < 0 || index > TYPE_VARMAX) {
		fprintf(stderr,"Error: invalid access to Var<%d>[%d]\n",info.type,info.number);
	}
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
void Flags::SetStr(VarInfo info, string val) {
	switch(info.type) {
	case TYPE_VARLOCSTR:
		if (info.number >= 3) return;
		loc_str[info.number] = val;
		break;
	case TYPE_VARSYSSTR:
		if (info.number >= 2000) return;
		sys_str[info.number] = val;
		break;
	case TYPE_VARSTR:
		if (info.number >= 2000) return;
		str[info.number] = val;
		break;
	}
	return;
}
void Flags::Str(int type, unsigned int number, char* buf, int sz) const {
	if (sz <= 0) return;
	buf[0] = 0;
	const string* sptr;
	switch(type) {
	case TYPE_VARLOCSTR:
		if (number >= 3) return;
		sptr = &loc_str[number];
		break;
	case TYPE_VARSYSSTR:
		if (number >= 2000) return;
		sptr = &sys_str[number];
		break;
	case TYPE_VARSTR:
		if (number >= 2000) return;
		sptr = &str[number];
		break;
	}

	int len = sptr->length();
	if (sz-1 > len) sz = len;
	sptr->copy(buf, sz, 0);
	buf[sz] = 0;
	return;
}
string Flags::Str(int type, unsigned int number) const {
	switch(type) {
	case TYPE_VARLOCSTR:
		if (number >= 3) return "";
		return loc_str[number];
	case TYPE_VARSYSSTR:
		if (number >= 2000) return "";
		return sys_str[number];
	case TYPE_VARSTR:
		if (number >= 2000) return "";
		return str[number];
	}
	return "";
}

void Flags::Save(string& save) {
	char buf[1024];
	save = "\n[Flags]\n";
	int i,j;
	for (i=0; i<=TYPE_NONSYSVARMAX; i++) {
		for (j=0; j<2000; j++) {
			if (var[i][j] != 0) {
				sprintf(buf, "V<%d>[%04d]=%d\n",i,j,var[i][j]);
				save += buf;
			}
		}
	}
	for (j=0; j<2000; j++) {
		if (str[j].length() != 0) {
			sprintf(buf, "V<C>[%04d]=%s\n", j, str[j].c_str());
			save += buf;
		}
	}
}
void Flags::Load(const char* save) {
	int i,j;
	for (i=0; i<=TYPE_NONSYSVARMAX; i++) {
		for (j=0; j<2000; j++) {
			var[i][j] = 0;
		}
	}
	sys = 0;
	for (j=0; j<2000; j++) {
		str[j] = "";
	}

	save = strstr(save, "\n[Flags]\n");

	if (save) {
		save += strlen("\n[Flags]\n");
		do {
			if (save[0] == '[') break; // next section
			if (strncmp(save, "V<",2) == 0) {
				if (strncmp(save, "V<C>[",5) == 0) { // string
					char buf[1024];
					int n;
					if (sscanf(save, "V<C>[%04d]=",&n) == 1) {
						char* s = strchr(save, '=');
						s++;
						char* send = strchr(s, '\n');
						int slen = send - s;
						strncpy(buf, s, slen);
						buf[slen] = 0;
						if (n >= 0 && n < 2000) str[n] = buf;
					}
				} else if (save[2] >= '0' && save[2] <= '9') {
					int c,n,v;
					if (sscanf(save, "V<%d>[%04d]=%d\n",&c,&n,&v) == 3) {
						if (c >= 0 && c <= TYPE_NONSYSVARMAX && n >= 0 && n < 2000)
							var[c][n] = v;
					}
				}
			}
			save = strchr(save, '\n');
			if (save) save++;
		} while (save);
	}
	return;
}

void Flags::SaveSys(string& save) {
	char buf[1024];
	int j;
	save = "\n[Flags]\n";
		for (j=0; j<2000; j++) {
		if (var[6][j] != 0) {
			sprintf(buf, "V<6>[%04d]=%d\n",j,var[6][j]);
				save += buf;
			}
		}
	for (j=0; j<2000; j++) {
		if (var[7][j] != 0) {
			sprintf(buf, "V<25>[%04d]=%d\n",j,var[7][j]);
			save += buf;
		}
	}
	for (j=0; j<2000; j++) {
		if (sys_str[j].length() != 0) {
			sprintf(buf, "V<M>[%04d]=%s\n", j, sys_str[j].c_str());
			save += buf;
		}
	}
}
void Flags::LoadSys(const char* save) {
	int i,j;
	for (i=6; i<=7; i++) {
		for (j=0; j<2000; j++) {
			var[i][j] = 0;
		}
	}
	for (j=0; j<2000; j++) {
		sys_str[j] = "";
	}
	sys = 0;

	save = strstr(save, "\n[Flags]\n");

	if (save) {
		save += strlen("\n[Flags]\n");
		do {
			if (save[0] == '[') break; // next section
			if (strncmp(save, "V<",2) == 0) {
				if (strncmp(save, "V<M>[",5) == 0) { // string
					char buf[1024];
					int n;
					if (sscanf(save, "V<M>[%04d]=",&n) == 1) {
						char* s = strchr(save, '=');
						s++;
						char* send = strchr(s, '\n');
						int slen = send - s;
						strncpy(buf, s, slen);
						buf[slen] = 0;
						if (n >= 0 && n < 2000) sys_str[n] = buf;
					}
				} else if (save[2] >= '0' && save[2] <= '9') {
					int c,n,v;
					if (sscanf(save, "V<%d>[%04d]=%d\n",&c,&n,&v) == 3) {
						if (c == 6 && n >= 0 && n < 2000)
							var[6][n] = v;
						else if (c == 25 && n >= 0 && n < 2000)
							var[7][n] = v;
					}
				}
			}
			save = strchr(save, '\n');
			if (save) save++;
		} while (save);
	}
	return;
}

bool Flags::Exec(Cmd& cmd) {
	if (cmd.cmd_type == CMD_FLAGS) { // 代入演算
		if (cmd.args.size() != 2) return false;
		Set(cmd.args[0], cmd.args[1].value);
		cmd.clear();
		return true;
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x0a) { // 文字列演算
		VarInfo arg1 = cmd.args[0];
		switch(cmd.cmd3) {
		case 0:
			if (cmd.cmd4 == 0) {
				SetStr(arg1, cmd.Str(cmd.args[1]));
			} else if (cmd.cmd4 == 1) {
				string s = cmd.Str(cmd.args[1]);
				const char* sc = s.c_str();
				int len = cmd.args[2].value;
				int i;for (i=0; i<sc[i]!=0 && len != 0; i++, len--) {
					if (sc[i]<0 && sc[i+1]!=0) i++;
				}
				s.erase(i); // 全角で len 文字まで切り詰める
				SetStr(arg1, s);
// fprintf(stderr,"Set[%d,%d]<-%s\n",arg1.type,arg1.number,s.c_str());
			} else break;
			cmd.clear();
			break;
		case 1:
			if (cmd.cmd4 == 0) {
				SetStr(arg1, "");
				cmd.clear();
			} else if (cmd.cmd4 == 1) {
				// 領域指定で文字列クリア
				VarInfo v1 = cmd.args[0];
				VarInfo v2 = cmd.args[1];
				eprintf("memclear(str). Var[%d]<%d> - Var[%d]<%d>\n",v1.type, v1.number, v2.type, v2.number);
				if (v1.type != v2.type || (v1.type != TYPE_VARSTR && v1.type != TYPE_VARSYSSTR && v1.type != TYPE_VARLOCSTR)) {
					eprintf("   error: bad args\n");
				} else {
					if (v1.number < 0) v1.number = 0;
					if (v2.number > 2000) v2.number = 2000;
					for (; v1.number <= v2.number; v1.number++) {
						SetStr(v1, "");
					}
				}
				cmd.clear();
			}
		case 2:
			SetStr(arg1, Str(arg1.type,arg1.number) + cmd.Str(cmd.args[1]));
// fprintf(stderr,"Append[%d,%d]<-%s(%d:%d)\n",arg1.type,arg1.number,Str(arg1.type,arg1.number).c_str(),cmd.args[1].type,cmd.args[1].number);
			cmd.clear();
			break;
		case 3:
			SetSys(strlen(cmd.Str(cmd.args[0])));
			cmd.clear();
			break;
		case 4:
			{ int v = strcmp(cmd.Str(cmd.args[0]), cmd.Str(cmd.args[1]));
// string s1=cmd.Str(cmd.args[0]);
// string s2=cmd.Str(cmd.args[1]);
// fprintf(stderr,"Cmp %s(%d:%d):%s(%d:%d):%d\n",s1.c_str(),cmd.args[0].type,cmd.args[0].number,s2.c_str(),cmd.args[1].type,cmd.args[1].number,v);
			if (v < 0) SetSys(-1);
			else if (v > 0) SetSys(1);
			else SetSys(0);
			cmd.clear();
			break; }
		case 5: // substring, index from left
		case 6: // substring, index from right
			// 全角対応らしい
			{ int offset = cmd.args[2].value;
			int len = strlen(cmd.Str(cmd.args[1]));
			string str = cmd.Str(cmd.args[1]);
			const char* s = str.c_str();
			if (cmd.cmd3 == 6) offset = len - offset;
			if (offset < 0) offset = 0;			
			// 先頭 N 文字を読み飛ばす
			int i;
			int offset_top = 0;
			for (i=0; i<offset && s[offset_top] != 0; i++) {
				if (s[offset_top] < 0 && s[offset_top+1] != 0) offset_top += 2;
				else offset_top += 1;
			}
			if (s[offset_top] == 0) {
				SetStr(arg1, "");
			} else if (cmd.cmd4 == 0) { // 長さ制限なし
				SetStr(arg1, string(s, offset_top));
			} else { // cmd.cmd4 == 1
				int slen = cmd.args[3].value;
				int offset_end = offset_top;
				for (i=0; i<slen && s[offset_end] != 0; i++) {
					if (s[offset_end] < 0 && s[offset_end]+1 != 0) offset_end += 2;
					else offset_end += 1;
				}
				string result(s, offset_top, offset_end-offset_top);
				SetStr(arg1, result);
			}
			cmd.clear();
			break; }
		case 7: {// strlen w/ kanji
			const char* s = cmd.Str(cmd.args[0]); int i;
			for (i=0; *s != 0; i++) {
				if (*s < 0 && s[1] != 0) s += 2;
				else s++;
			}
			SetSys(i);
			cmd.clear();
			break; }
		case 8: // 文字列を切って短くする
			if (cmd.args[1].value <= 0) {
				SetStr(arg1, "");
			} else if (cmd.args[1].value < strlen(cmd.Str(cmd.args[1]))) {
				Str(arg1.type,arg1.number).erase(cmd.args[1].value);
			}
			cmd.clear();
			break;
		case 0x0e: // 漢字モードでitoa
			{
			int arg1 = cmd.args[0].value;
			string result;
			char wc[3]; wc[2]=0;
			char buf[20];
			if (cmd.cmd4 == 0) {
				sprintf(buf, "%d", arg1);
			} else { // cmd.cmd4 == 1
				char fmt[20];
				sprintf(fmt, "%%%dd", cmd.args[2].value);
				sprintf(buf, fmt, arg1);
			}
			int i;
			for (i=0; buf[i] != 0; i++) {
				if (buf[i] == ' ') {
					wc[0] = 0x81;
					wc[0] = 0x40;
				} else if (buf[i] == '-') {
					wc[0] = 0x81;
					wc[0] = 0x7c;
				} else if (isdigit(buf[i])) {
					wc[0] = 0x82;
					wc[1] = buf[i] - '0' + 0x4f;
				} else {
					continue;
				}
				result += wc;
			}
			SetStr(cmd.args[1], result);
			cmd.clear();
			}
			break;
		case 0x0f: case 0x11: // itoa (0x11 の方は zero padding するっぽい)
			if (cmd.cmd4 == 0) {
				int arg1 = cmd.args[0].value;
				char buf[1024]; sprintf(buf, "%d", arg1);
				SetStr(cmd.args[1], buf);
				cmd.clear();
			} else if (cmd.cmd4 == 1) {
				// 漢字(SJIS) : 82 [4f+N]
				// やはり漢字じゃない？
				int arg1 = cmd.args[0].value;
				char buf[1024]; char fmt[1024];
				if (cmd.cmd3 == 0x0f) {
					sprintf(fmt, "%%%dd",cmd.args[2].value); /* 空白でパディング */
				} else {
					sprintf(fmt, "%%0%dd",cmd.args[2].value);
				}
				sprintf(buf, fmt, arg1);
				SetStr(cmd.args[1], buf);
				cmd.clear();
			}
			break;
		case 0x64: // 文字列の表示 : 引数をテキストウィンドウに表示
			if (cmd.cmd4 == 1) {
				char buf[256];
				snprintf(buf, 255, "%d", Get(cmd.args[0].type, cmd.args[0].number));
				cmd.args[0].type = TYPE_STR;
				cmd.args[0].value = cmd.AddStr(buf);
				cmd.cmd4 = 0;
			}

#if 0
@@@
save 27
ともよメガネのところ
- オブジェクト関連：seen9061:0 呼び出しで黒い背景画をかさねるところ、変になる
@@@
％Ｘで置換する名前の設定。0x51e で読みだし。セーブファイルごとに保存されるはずなので実装を考えること
％は0-3 (4 以降は使ってない）で、渚、秋生、渚、伊吹先生、など
StrVar を拡張して代入すること
初期値はこの辺
Text側に納め、セーブファイルでも同じようにすべきだろうなあ
        args:0,"渚"
        args:1,"秋生"
        args:2,"渚"
        args:3,"伊吹先生"
        args:4,"朋也くん"
        args:5,"岡崎さん"


106737 :  0x23 - cmd 01-04:051f:00[ 2] 
        args:0,"古河"
106758 : line 1712
106761 :  0x23 - cmd 01-04:051f:00[ 2] 
        args:2,"古河"
106782 : line 1713
106785 :  0x23 - cmd 01-04:051f:00[ 2] 
        args:4,"岡崎さん"

47382 :  0x23 - cmd 01-04:051e:00[ 2] 
        args:4,V<18>[0](=0)

47408 :  0x23 - cmd 01-0a:0004:00[ 2] 
        args:V<18>[0](=0),"岡崎さん"
47437 : expr: V<0>[1000](=0)=V<sys>
47451 :  0x23 - cmd 01-0a:0004:00[ 2] 
        args:V<18>[0](=0),"朋也くん"
47480 : expr: V<0>[1001](=0)=V<sys>
47494 :         V<0>[1000](=0)==0(=true)-> 47589
47526 :  0x23 - cmd 01-04:0514:00[ 2] 
        args:0,V<18>[0](=0)		/* NAME.A を帰す */
47552 :  0x23 - cmd 01-0a:0002:00[ 2] 
        args:V<18>[0](=0),"さん"
47577 :         jmp -> 47672
47589 :         V<0>[1001](=0)==0(=true)-> 47672
47621 :  0x23 - cmd 01-04:0514:00[ 2] 
        args:1,V<18>[0](=0)		/* NAME.B を帰す */
47647 :  0x23 - cmd 01-0a:0002:00[ 2] 
        args:V<18>[0](=0),"くん"
47672 : pos. 279
47675 :  0x23 - cmd 01-0a:0064:00[ 1] 
        args:V<18>[0](=0)

#endif
			cmd.cmd_type = CMD_TEXT;
			break;
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x0b) { // 数値変数演算
		if (cmd.cmd3 == 0 && cmd.cmd4 == 0) {
			/* 複数の変数をセット */
			VarInfo v1 = cmd.args[0];
			eprintf("set multiple-var Var[%d]<%d> <- ",v1.type, v1.number);
			int i;
			if (cmd.args.size() < cmd.argc) {
				eprintf("   error: argsize changed %d -> %d\n",cmd.argc, cmd.args.size());
				cmd.argc = cmd.args.size();
			}
			for (i=0; i<cmd.argc; i++) {
				eprintf("%d, ",cmd.args[i+1].value);
				Set(v1, cmd.args[i+1].value);
				v1.number++;
			}
			eprintf("\n");
			cmd.clear();
		} else if (cmd.cmd3 == 1 && cmd.cmd4 == 0) {
			/* 領域指定で変数をクリア */
			VarInfo v1 = cmd.args[0];
			VarInfo v2 = cmd.args[1];
			eprintf("memclear. Var[%d]<%d> - Var[%d]<%d>\n",v1.type, v1.number, v2.type, v2.number);
			if (v1.type != v2.type || !IsInt(v1.type)) eprintf("   error: bad args\n");
			else {
				if (v1.number < 0) v1.number = 0;
				if (v2.number > MaxIndex(v2.type)) v2.number = MaxIndex(v2.type);
				for (; v1.number <= v2.number; v1.number++)
					Set(v1, 0);
			}
			cmd.clear();
		} else if (cmd.cmd3 == 1 && cmd.cmd4 == 1) {
			/* 領域指定で変数をセット */
			VarInfo v1 = cmd.args[0];
			VarInfo v2 = cmd.args[1];
			int value = cmd.args[2].value;
			eprintf("memset. Var[%d]<%d> - Var[%d]<%d> <- %d\n",v1.type, v1.number, v2.type, v2.number, value);
			if (v1.type != v2.type || !IsInt(v1.type)) eprintf("   error: bad args\n");
			else {
				if (v1.number < 0) v1.number = 0;
				if (v2.number > MaxIndex(v2.type)) v2.number = MaxIndex(v2.type);
				for (; v1.number <= v2.number; v1.number++)
					Set(v1, value);
			}
			cmd.clear();
		} else if (cmd.cmd3 == 4 && cmd.cmd4 == 1) { // 領域クリア(sysfunc.txt)
			VarInfo v1 = cmd.args[0];
			int step = cmd.args[1].value;
			int deal = cmd.args[2].value;
			int val = cmd.args[3].value;
			eprintf("memclear. Var[%d]<%d> step %d deal %d <- val %d\n",v1.type, v1.number, step, deal, val);
			int i; for (i=0; i<deal; i++) {
				Set(v1, val);
				v1.number += step;
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x64 && cmd.cmd4 == 0) { //領域で数値を合計する
			VarInfo v1 = cmd.args[0];
			VarInfo v2 = cmd.args[1];
			eprintf("sum var. Var[%d]<%d> - Var[%d]<%d>\n",v1.type, v1.number, v2.type, v2.number);
			int sum = 0;
			if (v1.type != v2.type || !IsInt(v1.type)) eprintf("   error: bad args\n");
			else {
				if (v1.number < 0) v1.number = 0;
				if (v2.number > MaxIndex(v2.type)) v2.number = MaxIndex(v2.type);
				for (; v1.number <= v2.number; v1.number++)
					sum += (*this)(v1);
			}
			eprintf("    ret %d\n",sum);
			cmd.SetSysvar(sum);
		}
	}
	return false;
}

/*********************************************************************
**   Cmd
*/

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
		if (info.type == TYPE_VARSTR || info.type == TYPE_VARSYSSTR || info.type == TYPE_VARLOCSTR) {
			value = 0;
			info.value = StrVar(info.type, info.number);
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
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 10, 10};

inline int op_pri(int op) {
	if (op > 11) return 10;
	return op_pri_tbl[op];
}
inline int op_pri_cond(int op) {
	if (op <= 11) return op_pri_tbl[op];
	else if (op < 50) return 7;
	else if (op == 60) return 8;
	else if (op == 61) return 8;
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
	while(*d == 0x29 && stack_count > 0 && op_stack[stack_count-1] == OP_LB) {
		d++;
		dprintf(")");
		stack_count--;
	}
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
			dprintf("*%d*",d[1]);
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
		if (var == i) jumpto = j;
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
	string text = "";
	int cond_result = false;
	int sel_no = 0;
	while(*d != 0x7d) {
		if (d[0] == 0x0a || d[0] == 0x40) {
			int var;
			if (system_version == 0) { var = read_little_endian_int(d+1); d += 5;}
			else { var = read_little_endian_short(d+1); d += 3;}
			dprintf("Line %d; ",var);
			if (text.length() != 0) {
				if (cond_result) ; // 条件節が true なら表示しない
				else {
					const char* str = text.c_str();
					VarInfo var;
					var.type = TYPE_STR;
					var.value = CopyString(str);
					args.push_back(var);
					var.type = TYPE_VAL;
					var.value = sel_no;
					args.push_back(var);
				}
				sel_no++;
			}
			text = "";
			cond_result = false;
		} else if (d[0] == 0x2c) {
			dprintf(":comma:");
		} else if (d[0] == 0x28) {
			dprintf(":cond:");
			d++;
			while(d[0] != 0x29) {
				int result = GetExpressionCond(d); // PRINT- 節でないばあい、条件表示。次は文字節、またはPRINT節のはず
				if (*d == 0x32) { // 0x32 なら、現在の条件節を表示しない
					d++; dprintf("##");
					cond_result = result;
				} else if (*d == 0x31) { // 0x31 なら、現在の条件節を表示する
						// Little Busters! : この条件で正しいかは未検証
					d++; dprintf("***");
					cond_result = !result;
				}
				dprintf(":");
			}
			d++;
		} else if (StrType(d)) {
			int strpt = GetString(d);
			text += strheap + strpt;
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
				// 数値を全角文字に変換して登録
				char str[10], str2[20]; // itoa
				sprintf(str, "%d", info.value);
				int i; for (i=0; str[i] != 0; i++) {
					str2[i*2] = 0xa3;
					str2[i*2+1] = 0xb0 + str[i]-'0';
				}
				str2[i*2] = 0;
				text += str2;
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

static char* op_str3[11] = { "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "="};
void Cmd::GetCmd(Flags& flags_orig, const char*& d ) {
	if (d == 0) { SetError(); return;}
	if (cmd_type != CMD_NOP) return;

	cmdstr[0] = 0;
	rawdata = d;
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
			sprintf(cmdstr, "%02x-%02x:%04x:%02x",cmd1,cmd2,cmd3,cmd4);
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
					if (cmd3 == 0)
						dprintf("\tjmp -> %d\n", jump_arg);
					else /* cmd3 == 5 */
						dprintf("\tcall -> %d\n", jump_arg);
					is_special = 1;
				} else if (cmd3 == 1 || cmd3 == 2) {
					/* conditional jump (if / unless) */
					if (*d++ != 0x28) { SetError(); return;}
					dprintf("\t");
					int cond = GetExpressionCond(d);
					if (cmd3 == 1) cond = !cond; // 逆になる
					if (*d++ != 0x29) { SetError(); return; }
					int jumpto = read_little_endian_int(d);
					d += 4;
					dprintf("-> %d\n", jumpto);
					if (! cond) jump_arg = jumpto; /* condition が満たされない場合、ジャンプ */
					is_special = 1;
				} else if (cmd3 == 4) {
					/* switch to */
					jump_arg = GetSwitch(d);
					is_special = 1;
				} else if (cmd3 == 8 || cmd3 == 3) {
					/* switch to */
					jump_arg = GetSimpleSwitch(d);
					is_special = 1;
				} else if (cmd3 == 16) { // call with parameters
					GetArgs(d);
					jump_arg = read_little_endian_int(d);
					d += 4;
					is_special = 1;
				} else goto retry;
				if (jump_arg == -1) {
					cmd_type = CMD_NOP;
				}
				else {
					cmd_type = CMD_JMP;
					args.push_back(VarInfo(jump_arg));
				}
			} else if (cmd2 == 2 && (cmd3 == 0 || cmd3 == 1 || cmd3 == 2 || cmd3 == 3 || cmd3 == 0x0d) ) {
				/* selection */
				GetSelection(d);
				is_special = 1;
			}
		}
retry:
		/* 一般引数のもの */
		if (!is_special) {
			dprintf(" 0x23 - cmd %02x-%02x:%04x:%02x[%2d] \n",cmd1,cmd2,cmd3,cmd4,argc);
			dprintf("\t");
			if (cmd1 == 1 && cmd2 == 0x22 && (cmd3 == 0xc1c || cmd3 == 0x835)) GetArgsSpecial(3, d);
			else if (cmd1 == 1 && cmd2 == 0x0b && cmd3 == 0x65) GetArgsSpecial(0, d);
			else if (cmd1 == 1 && cmd2 == 0x15 && cmd3 == 0x28) GetArgsSpecial(0, d);
			else if (cmd1 == 1 && cmd2 == 4 && (cmd3 == 0x26c || cmd3 == 0x26d || cmd3 == 0x270 || cmd3 == 0x276)) GetArgsSpecial(0, d);
			else if (cmd1 == 1 && cmd2 == 4 && cmd3 == 0x586) GetArgsSpecial(1, d);
			else if (cmd1 == 1 && (cmd2 == 0x21 && cmd3 == 0x4b) || (cmd2 == 0x28 && cmd3 == 0x64)) GetArgsSpecial(2,d);
			else GetArgs(d);
			dprintf("\n");

		}
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
		dprintf("\n");
	} else if (*d == 0x0a || *d == 0x40 || *d == 0x21) { /* デバッグ用データと既読フラグ */
		cmd_type = CMD_NOP;
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
void Cmd::clear(void) {
	cmd_type = CMD_NOP;
	ResetString();
	args.clear();
	errorflag = false;
	pos = -1;
}

char Cmd::strtype[256] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +00 */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* +10 */ // 0123456789ABCDEF
	1,0,3,0, 0,0,0,1, 0,0,0,0, 0,1,1,0, /* +20 */ //  !"#$%&'()*+,-./
	1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,1, /* +30 */ // 0123456789:;<=>?
	0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, /* +40 */ // @ABCDEFGHIJKLMNO
	1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,1, /* +50 */ // PQRSTUVWXYZ[\]^_
	0,0,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, /* +60 */ // `abcdefghijklmno
	1,1,1,1, 1,1,1,1, 1,1,1,1, 0,0,0,0, /* +70 */ // pqrstuvwxyz{|}~
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
	bool quote_flag = false;
	int stype;
	retnum = strend;
	while(1) {
		if (*d == '\\') {
			d++;
			strheap[strend++] = *d++;
		} else if (*d == '"') {
			if (quote_flag) quote_flag = false;
			else quote_flag = true;
			d++;
		} else if (quote_flag) {
			strheap[strend++] = *d++;
		} else if (stype = StrType(d)) {
			strheap[strend++] = *d++;
			if (stype == 2) strheap[strend++] = *d++;
		} else break;
	}
	strheap[strend++] = 0;
	dprintf("\"%s\"", strheap + retnum);
	if (strend >= STRHEAP_SIZE) {
		dprintf("Error: string heap overflow\n");
	}
	return retnum;
}

int Cmd::CopyString(const char* d) {
	int retnum = strend;
	int len = strlen(d);
	memcpy(strheap+strend, d, len+1);
	strend += len+1;
	d += len+1;
	return retnum;
}

int Cmd::StrVar(int type, int var_num) {
	int retnum = strend;
	flags.Str(type, var_num, strheap+strend, STRHEAP_SIZE-strend);
	strend += strlen(strheap+strend)+1;
	return retnum;
}

void Cmd::SetSysvar(int n, int val) {
	VarInfo info;
	if (cmd_type != CMD_SYSVAR) {
		args.clear();
	}
	cmd_type = CMD_SYSVAR;

	info.type = TYPE_SYS;
	info.number = n;
	info.value = val;
	args.push_back(info);
}
void Cmd::SetFlagvar(VarInfo info, int val) {
	if (cmd_type != CMD_SYSVAR) {
		args.clear();
	}
	cmd_type = CMD_SYSVAR;

	info.value = val;
	args.push_back(info);
}

void Cmd::SetStrvar(VarInfo info, const string& s) {
	if (cmd_type != CMD_SYSVAR) {
		args.clear();
	}

	cmd_type = CMD_SYSVAR;
	const char* ss = s.c_str();
	info.value = CopyString(ss);
	args.push_back(info);
}

void Cmd::read(const CmdSimplified& from) {
	errorflag = false;
	ResetString();

	cmd_type = Cmdtype(from.type);
	cmd1 = from.cmd1;
	cmd2 = from.cmd2;
	cmd3 = from.cmd3;
	cmd4 = from.cmd4;
	argc = from.argc;
	/* args の読み込み */
	args.clear();
	char* d = from.args;
	if (d == 0) return;
	while(*d != TYPE_END) {
		VarInfo info;
		switch(*d) {
		case TYPE_VAL:
			info.type = TYPE_VAL;
			info.number = 0;
			info.value = read_little_endian_int(d+1);
			d += 5;
			args.push_back(info);
			break;
		case TYPE_STR:
			info.type = TYPE_STR;
			info.number = 0;
			d++;
			info.value = CopyString( d);
			d += strlen(d)+1;
			args.push_back(info);
			break;
		default:
			fprintf(stderr,"Cmd::read: Invalid Load Data\n");
			*d = TYPE_END;
		}
	}
	return;
}
void Cmd::write(CmdSimplified& to, char*& buffer) const {
/*
	if (cmd_type != CMD_OTHER) {
		fprintf(stderr,"Cmd::write: Invalid Cmd during Saving Data\n");
		to.cmd1 = 0; to.cmd2 = 0; to.cmd3 = 0; to.cmd4 = 0; to.argc = 0; to.args = 0;
		return;
	}
*/
	to.type = cmd_type;
	to.cmd1 = cmd1;
	to.cmd2 = cmd2;
	to.cmd3 = cmd3;
	to.cmd4 = cmd4;
	to.argc = argc;
	/* args の書き込み */
	if (args.empty()) {
		to.args = 0;
	} else {
		to.args = buffer;
		char* d = to.args;
		vector<VarInfo>::const_iterator it;
		for (it = args.begin(); it != args.end(); it++) {
			int type = it->type;
			if ( (type >= 0 && type < 7) || type == TYPE_VAL || type == char(TYPE_SYS)) { // digits
				*d++ = TYPE_VAL;
				write_little_endian_int(d, it->value);
				d += 4;
			} else if (type == TYPE_VARSTR || type == TYPE_VARSYSSTR || type == TYPE_VARLOCSTR || type == TYPE_STR) { // string
				*d++ = TYPE_STR;
				const char* s = Str(*it);
				int len = strlen(s);
				memcpy(d, s, len+1);
				d += len+1;
			} else {
				fprintf(stderr,"Cmd::write: Invalid Cmd args during Saving Data\n");
			}
		}
		*d++ = TYPE_END;
		buffer = d;
	}
}
void CmdSimplified::copy(const CmdSimplified& from, char*& args_buffer) {
	*this = from;
	if (args == 0) return;
	char* args_old = from.args;
	/* args のコピー */
	while(*args_old != TYPE_END) {
		if (*args_old == TYPE_VAL) {
			args_old += 5;
		} else { /* TYPE_STR */
			args_old += strlen(args_old)+1;
		}
	}
	args_old++;
	int args_len = args_old - from.args;
	memmove(args_buffer, from.args, args_len);
	args = args_buffer;
	args_buffer += args_len;
}
void CmdSimplified::Save(string& saveret) {
	char buf[1024];
	sprintf(buf, "%02x-%02x:%04x:%02x(%02d),", cmd1, cmd2, cmd3, cmd4, argc);
	saveret += buf;
	
	/* args のコピー */
	char* d = args;
	while(d && *d != TYPE_END) {
		if (*d == TYPE_VAL) {
			d++;
			sprintf(buf, "%d,", read_little_endian_int(d));
			d += 4;
		} else { /* TYPE_STR と仮定 */
			d++;
			if (strlen(d) > 1000) d[1000] = 0; // ありえない・・・
			int i; int cnt = 0;
			buf[cnt++] = '"';
			for (i=0; d[i] != 0; i++) {
				if (d[i] == '"') buf[cnt++] = '"';
				buf[cnt++] = d[i];
			}
			buf[cnt++]='"';
			buf[cnt++] = ',';
			buf[cnt++] = 0;
			d += strlen(d)+1;
		}
		saveret += buf;
	}
	saveret += 'E';
}

void CmdSimplified::Load(const char* save, char*& args_buffer) {
	args = args_buffer;

	type = CMD_OTHER;
	sscanf(save, "%02x-%02x:%04x:%02x(%02d),", &cmd1, &cmd2, &cmd3, &cmd4, &argc);
	save = strchr(save, ',');
	if (save == 0) {
		*args_buffer++ = TYPE_END;
		return;
	}
	save++;
	while(*save != 'E' && *save != '\n' && *save != '\0') {
		if (isdigit(*save)) {
			int v;
			sscanf(save,"%d,",&v);
			*args_buffer++ = TYPE_VAL;
			write_little_endian_int(args_buffer, v);
			args_buffer+= 4;
			save = strchr(save, ',');
			if (save) save++;
		} else { // *save == '"'
			save++;
			*args_buffer++ = TYPE_STR;
			while(1) {
				if (*save == 0) break;
				if (*save == '"') {
					if (save[1] != '"') break;
					save++;
				}
				*args_buffer++ = *save++;
			}
			save += 2;
			*args_buffer++ = 0;
		}
	}
	*args_buffer++ = TYPE_END;
	return;
}

#ifdef SCN_DUMP
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
	int system_version = 0;
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
		Flags flags;
		/* 最初から最後までコマンド取得 -> 出力を繰り返す */
		while(dcur<dend) {
			const char* dprev = dcur;
			Cmd cmd(flags, system_version); cmd.ClearError();

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
#endif

