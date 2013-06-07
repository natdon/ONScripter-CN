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

#include <stdexcept>
#include"scn2k_impl.h"
#include"system/file.h"
#include"system/system_config.h"
#include"window/picture.h"
#include"window/system.h"

// #define DEBUG 1

using namespace std;

/**********************************************
** Scn2k
*/

void kconv(const unsigned char* src, unsigned char* dest);
void kconv_rev(const unsigned char* src, unsigned char* dest);
string kconv(const string& s);
string kconv_rev(const string& s);

Scn2k::Scn2k(Event::Container& _event, PicContainer& _parent, class MuSys& mu, AyuSysConfig& _config) :
	Event::Time(_event),
	event(_event),
	parent(_parent),
	config(_config),
	text_exec(_event, _parent, config),
	grp_exec(_event, _parent, flag, flag.cgm_data, mu, config)
{
	system_version = 0;
	skip_mode = SKIP_NO;

	script_start = 0;
	script = 0;
	script_end = 0;

	backlog_script_scn = -1;
	backlog_script_start = 0;
	backlog_script_end = 0;

	save_scn = 0;
	save_point = 0;
	scn_number = 0;
	scn_point = 0;
	cmd_stack_str = cmd_stack_str_orig;

	dialog = 0;
	dialog_type = CMD_NOP;
	menu = 0;
	menu_mouseshown = false;

	/* マウスカーソルを作成 */
	mouse_type = 0;
	mouse_surface = 0;
	mouse_pressed = 0;
	ShowCursor();

	LoadSys();
	text_exec.InitWindow();
	grp_exec.InitSel(config);
}

Scn2k::~Scn2k() {
	HideCursor();
	SaveSys();
}

char* Scn2k::OpenScript(int new_scn_number, char*& end, int* call_vec, int& system_version) {
	char fname[1024];
	const char* data;
	char* ret_data;
	int offset = 0;
	int scenario_magic;

	sprintf(fname, "SEEN%04d.TXT", new_scn_number);
	ARCINFO* info = file_searcher.Find(FILESEARCH::SCN, fname, "");
	if (info == 0) goto err;
	data = info->Read();

	/* version 確認 */
	scenario_magic = read_little_endian_int(data + 4);
	if (scenario_magic != 0x2712 && scenario_magic != 0x1adb2) {
		fprintf(stderr,"Invalid scenario header : scenario number %d\n",new_scn_number);
		goto err;
	}
	if (read_little_endian_int(data) == 0x1cc) {
		system_version = 0;
		offset = 0x1cc + read_little_endian_int(data+0x20) + 4;
	} else if (read_little_endian_int(data) == 0x1d0) {
		system_version = 1;
		offset = read_little_endian_int(data + 0x20);
	} else {
		fprintf(stderr,"Invalid scenario header : scenario number %d\n",new_scn_number);
		goto err;
	}
	/* header から subroutine number とりだし */
	if (call_vec) {
		int i;
		for (i=0; i<100; i++) {
			call_vec[i] = read_little_endian_int(data + 0x34 + i * 4);
		}
	}
	ret_data = new char[info->Size() - offset + 1024];
	memcpy(ret_data, data+offset, info->Size()-offset);
	memset(ret_data+info->Size()-offset, 0, 1024);
	end = ret_data + info->Size() - offset;
	delete info;
	return ret_data;

err:
	delete info;
	fprintf(stderr,"Cannot open scenario number %d\n",new_scn_number);
	throw std::invalid_argument("Scn2k::OpenScript");

	return false;
}
bool Scn2k::ChangeScript(int new_scn_number, int call_no) {
	int old_scn_number = scn_number;
	int old_scn_pt = script - script_start;
	int scn_pt = 0;

	if (script_start) delete[] script_start;
	script_start = 0;
	script = 0;
	script_end = 0;

	int call_vec[100];

	try {
		script_start = OpenScript(new_scn_number, script_end, call_vec, system_version);
	} catch(...) {
		fprintf(stderr,"\tFrom script %d pt %d\n",old_scn_number, old_scn_pt);
		throw;
	}
	if (call_no > 0 && call_no < 100) {
		scn_pt = call_vec[call_no];
		if (scn_pt == 0) {
			fprintf(stderr,"Invalid subroutine number: scn %d sub %d\n",new_scn_number, call_no);
			scn_pt = 0;
		}
	} else if (call_no < 0) {
		scn_pt = -call_no; // デバッグ用
	}

	scn_number = new_scn_number;
	scn_point = scn_pt;
	script = script_start + scn_pt;
	if (script < script_start || script >= script_end) 
		fprintf(stderr,"scn %d pt %d: Cannot jump to %d:%d; fall back to the top\n",old_scn_number, old_scn_pt, scn_number, scn_pt);
	return true;
}
bool Scn2k::ReadCmdAt(Cmd& cmd, int scn, int pt) {
	const char* d;
	if (scn ==scn_number) {
		d = script_start + pt;
		if (d < script_start || d >= script_end) {
			fprintf(stderr,"Cannot read script at current scn %d pt %d\n", scn, pt);
			return false;
		}
	} else {
		if (backlog_script_scn != scn) {
			if (backlog_script_start) delete[] backlog_script_start;
			backlog_script_start = OpenScript(scn, backlog_script_end, 0, system_version);
		}
		d = backlog_script_start + pt;
		if (d < backlog_script_start || d >= backlog_script_end) {
			fprintf(stderr,"Cannot read script at scn %d pt %d\n", scn, pt);
			return false;
		}
	}
	
	cmd.GetCmd(flag, d);
	return true;
}

extern bool save_req, load_req; // キーボードからセーブ・ロードできるように
extern bool pressAreq;

void Scn2k::Elapsed(unsigned int current_time) {
	SetWakeup(current_time + 10); // 10msに一回シナリオスクリプト解釈
	if (script == 0) return;
//VarInfo info; info.type = 6; info.number = 0; // PB の「一回ゲームを開始したことがある」フラグ
//flag.Set(info,1);
//info.type = 0; info.number = 604; // Princess Bride: クリア対象設定フラグ (聖)
//flag.Set(info, 1); 


	Cmd cmd(flag, system_version);
	int cnt1;
	int cnt2 = 1000; // flag / jump / flag 系コマンドの最大実行回数

	/* XXX */
	if (save_req) {
		save_req = false;
		load_req = false;
		cmd.cmd_type = CMD_SAVEREQ;
	} else if (load_req) {
		load_req = false;
		save_req = false;
		cmd.cmd_type = CMD_LOADREQ;
	}
	if (pressAreq) {
		pressAreq = false;
		LoadRollback(cmd);
		return;
	}

	/* キー入力などに対応 */
	// メニュー内以外で shift キーが押されたらスキップ開始
	if ( (skip_mode&SKIP_IN_MENU) == 0) {
		if (event.pressed(KEY_SHIFT)) {
			if (skip_mode & SKIP_TEXT) {
				; // スキップ中ならなにもしない
			} else {
				SetSkipMode(SkipMode(SKIP_TEXT | SKIP_GRP_NOEFFEC | SKIPEND_KEY));
			}
		} else {
			if ( skip_mode & SKIPEND_KEY) {
				if ( (skip_mode & SKIPEND_TEXT) && (skip_mode & SKIP_TEXT)) {
					SkipMode new_skip_mode = SkipMode(skip_mode & (~SKIPEND_KEY));
					if ( (new_skip_mode & SKIP_GRP_FAST) || (new_skip_mode & SKIP_GRP_NODRAW)) {
						new_skip_mode = SkipMode(skip_mode & (~SKIP_GRP_NOEFFEC));
					}
					SetSkipMode(new_skip_mode);
				} else {
					SetSkipMode(SKIP_NO);
				}
			}
		}
	}

	for (cnt1=0; cnt1<20; cnt1++) { // 一回につき 20 個のコマンド実行
		// 他のコマンド実行中なら終了
		if ( (cmd.cmd_type == CMD_NOP && SysWait(cmd)) ||
		     // (cmd.cmd_type == CMD_NOP && text_exec.Wait(current_time, cmd)) ||
		     // (cmd.cmd_type == CMD_NOP && grp_exec.Wait(current_time, cmd))) {
		     (cmd.cmd_type == CMD_NOP && grp_exec.Wait(current_time, cmd)) ||
		     (cmd.cmd_type == CMD_NOP && text_exec.Wait(current_time, cmd))) {
			break;
		}
		// コマンド読み込み
		for (; cnt2 > 0; cnt2--) {
			scn_point = script - script_start;
			eprintf("%d / %d :", script - script_start, script_end-script_start);
// fprintf(stderr,"%d: %d / %d :",scn_number,  script - script_start, script_end-script_start);
			cmd.GetCmd(flag, script);
//		if (cmd.cmd_type != CMD_NOP) {
if (0) {
			fprintf(stderr,"%d / %d : 0x23 - cmd %02x-%02x:%04x:%02x[%2d] \n",
				scn_point, script_end-script_start,
				cmd.cmd1,cmd.cmd2,cmd.cmd3,cmd.cmd4,cmd.argc);
			int i; for (i=0; i<cmd.args.size(); i++) {
				if (i == 0) fprintf(stderr,"\t");
				VarInfo info = cmd.args[i];
				if (info.type == TYPE_STR || info.type == TYPE_VARSTR)
					fprintf(stderr,"\"%s\",", cmd.Str(info));
				else
					fprintf(stderr,"%d,",info.value);
			}
			fprintf(stderr,"\n");
		}
			cmd.scn = scn_number;
			cmd.pos = scn_point;
			if (cmd.IsError()) break;
			if (cmd.cmd_type == CMD_NOP) continue;
			if (cmd.cmd_type == CMD_JMP) {
				// local jump
				if (cmd.cmd1 == 0 && cmd.cmd2 == 1 && cmd.cmd3 == 16) {
					int i;
					for (i=0; i<cmd.args.size()-1; i++) {
						VarInfo var;
						var.type = 11;
						var.number = i;
						flag.Set(var, cmd.args[i].value);
					}
					cmd.args[0].value = cmd.args[i].value;
				}
				if ( cmd.cmd1 == 0 && cmd.cmd2 == 1 && (cmd.cmd3 == 5 || cmd.cmd3 == 8 || cmd.cmd3 == 16) ) { // local call / simple switch
					int scn_pt = script - script_start;
// fprintf(stderr,"\nlocal call %d:%d from %d\n",scn_number,cmd.args[0].value,scn_pt);
					stack.push_back(StackItem(-1, scn_pt));
				}
if (cmd.cmd1 == 0 && cmd.cmd2 == 1 && cmd.cmd3 == 1) {
	fprintf(stderr,"***  unsupported: cond 1\n");
}
				script = script_start + cmd.args[0].value;
				if (script < script_start || script >= script_end) {
					fprintf(stderr,"scn %d pt %d: Cannot jump to %d; fall back to the top\n",scn_number, cmd.args[0].value);
					script = script_start;
				}
				cmd.clear();
				continue;
			}
			if (flag.Exec(cmd)) continue;
			break;
		}
		if (cmd.IsError()) {
fprintf(stderr,"cmd error occured: scn %d pt %d / cur %d",scn_number,scn_point,script-script_start);
			while(script < script_end) {
				if (*script == 0x29 && script[1] == 0x0a) {script++;break;}
				if (*script == 0 && script[1] == 0x0a) {script++;break;}
				if (*script == 0 && script[1] == 0x23) {script++;break;}
				script++;
fprintf(stderr," -> fall back to %d\n",script-script_start);
			}
			const char* dprev = script - 0x60;
			if (dprev < script_start) dprev = script_start;
			int ilen = (script-dprev+65)/16;
			int i; for (i=0; i<ilen; i++) {
				fprintf(stderr, "%6d: ",dprev-script_start);
				int j; for (j=0; j<16; j++) {
					if (dprev >= script_end) break;
					fprintf(stderr, "%02x ",*(unsigned char*)(dprev));
					dprev++;
				}
				fprintf(stderr, "\n");
			}
			break;
		}
		if (cmd.cmd_type == CMD_NOP) continue;

		if (cmd.cmd_type == CMD_TEXT && cmd.pos != -1) {
			set<int>& readflag = text_readflag[scn_number];
			if (readflag.find(cmd.pos) == readflag.end()) { // 未読テキスト発見
				readflag.insert(cmd.pos);
				if (skip_mode & SKIPEND_TEXT) {
					if (!(skip_mode & SKIPEND_KEY)) SetSkipMode(SKIP_NO);
				}
			}
		}
		text_exec.Exec(cmd);
		grp_exec.Exec(cmd);
		SysExec(cmd);
		if (cmd.cmd_type == CMD_WAITFRAMEUPDATE) {
			SetWakeup(Event::Time::FRAME_UPDATE);
			break;
		} else if (cmd.cmd_type != CMD_NOP) {
#if DEBUG
			fprintf(stderr,"%d-%d / %d : unsupported command; 0x23 - cmd %02x-%02x:%04x:%02x[%2d] \n",
				cmd.scn, script - script_start, script_end-script_start,
				cmd.cmd1,cmd.cmd2,cmd.cmd3,cmd.cmd4,cmd.argc);
			int i; for (i=0; i<cmd.args.size(); i++) {
				if (i == 0) fprintf(stderr,"\t");
				VarInfo info = cmd.args[i];
				if (info.type == TYPE_STR || info.type == TYPE_VARSTR)
					fprintf(stderr,"\"%s\",", cmd.Str(info));
				else
					fprintf(stderr,"%d,",info.value);
			}
			fprintf(stderr,"\n");
#endif
			cmd.clear();
		}
	}
	return;
}

void Scn2k::ShowCursor(void) {
	HideCursor();
	char key[1024];
	sprintf(key, "#MOUSE_CURSOR.%03d.NAME",mouse_type);
	const char* name = config.GetParaStr(key);
	if (name == 0 || name[0] == 0) mouse_surface = DEFAULT_MOUSECURSOR;
	else {
		mouse_surface = parent.Root().NewSurface(name, COLOR_MASK);
	}
	if (mouse_surface == 0) mouse_surface = DEFAULT_MOUSECURSOR;
	System::Main::SetCursor(mouse_surface, Rect(8, 8, 8+32, 8+32));
}

void Scn2k::HideCursor(void) {
	if (mouse_surface) {
		System::Main::SetCursor(0, Rect(0,0));
		if (mouse_surface != DEFAULT_MOUSECURSOR)
			parent.Root().DeleteSurface(mouse_surface);
		mouse_surface = 0;
	}
	return;
}

bool Scn2k::SysWait(Cmd& cmd) {

	if (menu) {
		menu->Exec(cmd);
		if (menu->status & Scn2kMenu::MENU_DELETE || menu->pimpl == 0) {
			delete menu;
			menu = 0;
			if (! menu_mouseshown) HideCursor();
			else ShowCursor();
			SetSkipMode(SkipMode(skip_mode & (~SKIP_IN_MENU) ));
		}
		if (cmd.cmd_type == CMD_NOP) return true;
		else return false; /* exec command */
	}
	return false;
}

void DllCall_LB(Cmd& cmd, Flags& flags);
void Scn2k::SysExec(Cmd& cmd) {
	if (cmd.cmd_type == CMD_SYSVAR) {
		int i;
		for (i=0; i<cmd.args.size(); i++) {
			if (cmd.args[i].type == TYPE_SYS) {
				if (cmd.args[i].number == TYPE_SYS_SYS) {
					flag.SetSys(cmd.args[i].value);
				} else if (cmd.args[i].number == TYPE_SYS_SKIPMODE) {
					SetSkipMode(SkipMode(cmd.args[i].value));
				}
			} else if (cmd.args[i].type == TYPE_VARSTR) {
				flag.SetStr(cmd.args[i].number, cmd.Str(cmd.args[i]));
			} else {
				flag.Set(cmd.args[i], cmd.args[i].value);
			}
		}
		cmd.clear();
	}
	if (cmd.cmd_type == CMD_SAVEPOINT || cmd.cmd_type == CMD_ROLLBACKPOINT) {
		if (text_exec.backlog_item.scn != -1) {
			text_exec.backlog.push_back(text_exec.backlog_item);
			text_exec.backlog_item.Clear();
		}
		save_scn = scn_number;
		save_point = scn_point;
		if (!new_rollback_save.empty()) {
			rollback_save.push_back(new_rollback_save);
			new_rollback_save = "";
		}
		if (cmd.cmd_type == CMD_ROLLBACKPOINT) SaveRollback();
		cmd.clear();
	}
	if (cmd.cmd_type == CMD_SAVEREQ || cmd.cmd_type == CMD_SAVE) {
		Save(cmd);
		return;
	}
	if (cmd.cmd_type == CMD_LOADREQ || cmd.cmd_type == CMD_LOAD) {
		Load(cmd);
		return;
	}
	if (cmd.cmd_type == CMD_BACKLOGREQ || cmd.cmd_type == CMD_BACKLOGREQ_FWD) {
		if (menu) {
			fprintf(stderr,"BACKLOG_REQ requested!!!\n");
			return;
		}
		if (cmd.cmd_type == CMD_BACKLOGREQ_FWD) {
			cmd.clear(); // backlog mode 以外で fwd を押されてもなにもしない
			return;
		}
		SetSkipMode(SKIP_IN_MENU); // テキストスキップ等はここで中断
		menu = new Scn2kMenu(Scn2kMenu::MENU_BACKLOG, *this, flag, text_exec, system_version);
		menu->InitPanel(event, parent);
		menu->InitTitle(Scn2kSaveTitle(*this));
		if (mouse_surface) menu_mouseshown = true;
		else menu_mouseshown = false;
		ShowCursor();
		return;
	}
	if (cmd.cmd_type == CMD_MENUREQ) {
		int scn=0, pt=0;
		config.GetParam("#CANCELCALL", 2, &scn, &pt);
		if (scn) {
			// 右クリックされたら global call を行う
			cmd.cmd_type = CMD_OTHER;
			cmd.cmd1 = 0;
			cmd.cmd2 = 1;
			cmd.cmd3 = 0x0c;
			cmd.cmd4 = 1;
			cmd.args.clear();
			cmd.args.push_back(VarInfo(SCN_INFO_MENU));
			cmd.args.push_back(0);
			SetSkipMode(SKIP_IN_MENU); // テキストスキップ等はここで中断
		}
	}
	if (cmd.cmd_type == CMD_SAVECMDGRP || cmd.cmd_type == CMD_SAVECMDGRP_START || cmd.cmd_type == CMD_SAVECMDGRP_ONCE || cmd.cmd_type == CMD_SAVECMD_ONCE) {
		// 画像コマンド等はスタックに保存し、セーブ時に保存できるようにする
		if (cmd.cmd_type == CMD_SAVECMDGRP_START) {
			vector<CmdSimplified>::iterator it, cur;
			cur = cmd_stack.begin();
			cmd_stack_str = cmd_stack_str_orig;
			/* 画像関連コマンド以外を別にする */
			for (it=cmd_stack.begin(); it != cmd_stack.end(); it++) {
				if (it->type != CMD_SAVECMDGRP && it->type != CMD_SAVECMDGRP_START && it->type != CMD_SAVECMDGRP_ONCE) {
					cur->copy(*it, cmd_stack_str);
					cur++;
				}
			}
			cmd_stack.erase(cur, cmd_stack.end());
		}
		if (cmd.cmd_type == CMD_SAVECMD_ONCE || cmd.cmd_type == CMD_SAVECMDGRP_ONCE) { // 同じコマンドがあれば削除する
			vector<CmdSimplified>::iterator it;
			for (it = cmd_stack.end(); it != cmd_stack.begin(); ) {
				--it;
				if (it->cmd1 == cmd.cmd1 && it->cmd2 == cmd.cmd2 && it->cmd3 == cmd.cmd3 && it->cmd4 == cmd.cmd4) {
					cmd_stack.erase(it);
					break;
				}
			}
		}
		CmdSimplified cmd_item;
		cmd.write(cmd_item, cmd_stack_str);
		cmd_stack.push_back(cmd_item);
		cmd.clear();
		if (cmd_stack_str > cmd_stack_str_orig + 30000) { // char cmd_stack_str_orig[32768]
			fprintf(stderr,"Error in Scn2k::SysExec: too long cmdstack (%d): stack string overflow\n",cmd_stack.size());
			cmd_stack_str = cmd_stack_str_orig;
			cmd_stack.clear();
		}
	}
	if (cmd.cmd_type != CMD_OTHER) return;
	if (cmd.cmd1 == 0 && cmd.cmd2 == 1) {
		if (cmd.cmd3 == 0x0b) { // global jump
			eprintf("global jump to %d\n",cmd.args[0].value);
			if (! ChangeScript(cmd.args[0].value, 0)) return; // 読み込めない; abort.
			cmd.clear();
		} else if (cmd.cmd3 == 0x0c || cmd.cmd3 == 0x12) { // call (0x12 の方は微妙)
			int new_scn = cmd.args[0].value;
			int new_pt = 0;
			if (cmd.args.size() >= 1) { // subroutine number が付く
					// 引数が付くのもあるらしい
				new_pt = cmd.args[1].value;
			}
			if (new_scn == SCN_INFO_MENU) { // menu call
				config.GetParam("#CANCELCALL", 2, &new_scn, &new_pt);
				stack.push_back(StackItem(SCN_INFO, SCN_INFO_MENU)); // menu call を示す特殊な記号
			} else {
				int i;
				VarInfo var;
				// ローカル変数を伴う subroutine call
				var.type = 11;
				var.number = 0;
				int saved_vars = 0;
				for (i=0; i<40; i++) {
					int val = flag.Get(var.type, i);
					if (val != 0) {
						stack.push_back(StackItem(SCN_INFO_LOCALS + i, val));
						saved_vars++;
					}
				}
				var.type = TYPE_VARLOCSTR;
				for (i=0; i<3; i++) {
					string s = flag.Str(var.type, i);
					if (s.size()) {
						int sp = stack_strbuffer.size();
						stack.push_back(StackItem(SCN_INFO_LOCALSTR+i, sp));
						stack_strbuffer.push_back(s);
						saved_vars++;
					}
				}
				stack.push_back(StackItem(SCN_INFO, SCN_INFO_LOCALS + saved_vars));
					
				var.type = 11;
				var.number = 0;
				// 特殊な subroutine call なので、余計な情報を引数に渡す
				for (i=2; i<cmd.args.size(); i++) {
					flag.Set(var, cmd.args[i].value);
// fprintf(stderr,"<%d:%d>=%d;",var.type,var.number,cmd.args[i].value);
					var.number++;
				}
// fprintf(stderr,"%d; ",stack.size());
			}
			int scn_pt = script - script_start;
			stack.push_back(StackItem(scn_number, scn_pt));
// fprintf(stderr,"\nglobal call %d:%d from %d:%d\n",new_scn,new_pt,scn_number,scn_pt);
			eprintf("global call to %d, %d\n",new_scn, new_pt);
			if (! ChangeScript(new_scn, new_pt)) return; // 読み込めない; abort.
			cmd.clear();
		} else if (cmd.cmd3 == 0x65) { // 文字列の返り値をセットする
			int arg1 = cmd.args[0].value;
			string s = cmd.Str(cmd.args[1]);
			int sp = stack_strbuffer.size();
			stack.push_back(StackItem(SCN_INFO_RETSTR+arg1, sp));
			stack_strbuffer.push_back(s);
			cmd.clear();
		} else if (cmd.cmd3 == 0x0d || cmd.cmd3 == 0x0a || cmd.cmd3 == 0x11 || cmd.cmd3 == 0x13) { // return (0a: local return) (0x13はよくわからない)
// fprintf(stderr,"global return : stack size %d\n",stack.size());
			if (stack.empty()) {
				cmd.clear();
				return; // スタックがおかしい：abort
			}
			map<int, string> retstr;
			while( (!stack.empty()) && stack.back().scn_number >= SCN_INFO_RETSTR) {
				int ret_num = stack.back().scn_number - SCN_INFO_RETSTR;
// fprintf(stderr,"\nRetStr;");
				string str = stack_strbuffer.back();
				stack_strbuffer.pop_back();
				retstr[ret_num] = str;
				stack.pop_back();
			}
			if (stack.empty()) {
				cmd.clear();
				return; // スタックがおかしい：abort
			}
			StackItem s = stack.back();
			stack.pop_back();
			bool localvar_init = false;
			while( (!stack.empty()) && stack.back().scn_number == SCN_INFO) {
				int mode = stack.back().scn_pt;
				stack.pop_back();
				if (mode == SCN_INFO_MENU) {
// fprintf(stderr,"\nInfo Menu;");
					// menu モード終了
					SetSkipMode(SkipMode(skip_mode & (~SKIP_IN_MENU) ));
				} else if (mode >= SCN_INFO_LOCALS && mode <= SCN_INFO_LOCALS+50) {
// fprintf(stderr,"\nInfo Local;");
					int i;
					// ローカル変数を元に戻す
					VarInfo var;
					var.type = 11;
					var.number = 0;
					for (i=0; i<40; i++) {
						var.number = i;
						flag.Set(var, 0);
					}
					var.type = TYPE_VARLOCSTR;
					for (i=0; i<3; i++) {
						var.number = i;
						flag.SetStr(var, "");
					}
					int args = mode - SCN_INFO_LOCALS;
// fprintf(stderr," args = %d; ",args);
					for (i=0; i<args; i++) {
						if (stack.empty() || stack.back().scn_number < SCN_INFO) {
							fprintf(stderr,"Fatal : Invalid stack found in preserved local variables!\n");
							break;
						}
						var.number = stack.back().scn_number;
// fprintf(stderr,"%d:%d; ",stack.back().scn_number,stack.back().scn_pt);
						if (var.number >= SCN_INFO_LOCALS && var.number < SCN_INFO_LOCALSTR) {
							var.type = 11;
							var.number -=  SCN_INFO_LOCALS;
							flag.Set(var, stack.back().scn_pt);
						} else if (var.number >= SCN_INFO_LOCALSTR && var.number < SCN_INFO_RETSTR) {
							var.type = TYPE_VARLOCSTR;
							var.number -= SCN_INFO_LOCALSTR;
							flag.SetStr(var, stack_strbuffer.back());
							stack_strbuffer.pop_back();
						}
						stack.pop_back();
					}
				}
// fprintf(stderr,"stack size %d string size %d\n",stack.size(),stack_strbuffer.size());
			}
			if (cmd.cmd3 == 0x11 || cmd.cmd3 == 0x13) {
// fprintf(stderr,"\nSet RetLocal;");
				// 返り値をセットする
				map<int,string>::iterator it;
				VarInfo var;
				var.type = TYPE_VARLOCSTR;
				for (it=retstr.begin(); it!=retstr.end(); it++) {
					var.number = it->first;
					flag.SetStr(var, it->second);
				}
				var.type = 11;
// fprintf(stderr,"return : cmd.cmd3 == 0x11; size %d\n",cmd.args.size());
				if (cmd.args.size() == 1) {
// fprintf(stderr,"return value %d\n",cmd.args[0].value);
					flag.SetSys(cmd.args[0].value);
				} else {
					int i;for (i=0; i<cmd.args.size(); i++) {
						var.number = i;
						flag.Set(var, cmd.args[i].value);
					}
				}
			}
// fprintf(stderr,"global return : return to %d:%d\n",s.scn_number,s.scn_pt);
// fprintf(stderr,"\nglobal return %d:%d from %d:%d\n",s.scn_number,s.scn_pt,scn_number, script - script_start);
			if (s.scn_number != -1) {
				if (! ChangeScript(s.scn_number, 0)) return; // 読み込めない; abort.
			}
			script = script_start + s.scn_pt;
			cmd.clear();
		}
	} else if (cmd.cmd1 == 2 && cmd.cmd2 == 1 && cmd.cmd3 == 12) { // DLL Call
		const char* regname = config.GetParaStr("#REGNAME");
		const char key_lb[] = "KEY\\LittleBusters";
		if (strcmp(regname, key_lb) == 0) {
			DllCall_LB(cmd, flag);
			cmd.clear();
		}
	} else if (cmd.cmd1 == 0 && cmd.cmd2 == 0x04) { // メニューモード
		if (cmd.cmd3 == 300 || cmd.cmd3 == 301 || cmd.cmd3 == 302) {
			// メニューからのreturn
			cmd.cmd2 = 1;
			cmd.cmd3 = 0x0d;
			SysExec(cmd);
		}
	} else if (cmd.cmd1 == 1 && cmd.cmd2 == 0x04) {
		if (cmd.cmd3 == 0 && cmd.cmd4 == 0) { // タイトル名設定
			const char* name = cmd.Str(cmd.args[0]);
			if (name == 0) name = "";
			window_title = name;
			const char* config_name = config.GetParaStr("#CAPTION");
			if (config_name == 0) config_name = "";
			string setname = kconv(string(config_name) + "  " + window_title);
			parent.Root().SetWindowCaption(setname.c_str());
			cmd.clear();
		} else if (cmd.cmd3 == 0x82 && cmd.cmd4 == 0) {
			/* cmd.cmd3 == 0x82 : マウスの press 状態クリアかも */
			event.presscount(MOUSE_LEFT);
			event.presscount(MOUSE_RIGHT);
			cmd.clear();
		} else if (cmd.cmd3 == 0x85 && cmd.cmd4 == 0) {
			int x,y,left,right;
			event.MousePos(x,y);
			if (event.presscount(MOUSE_LEFT)) left = 2;
			else if (event.pressed(MOUSE_LEFT)) left = 1;
			else left = 0;
			
			if (event.presscount(MOUSE_RIGHT)) right = 2;
			else if (event.pressed(MOUSE_RIGHT)) right = 1;
			else right = 0;
			
			// eprintf("mouse pos\n");
			flag.Set(cmd.args[0], x);
			flag.Set(cmd.args[1], y);
			flag.Set(cmd.args[2], left);
			flag.Set(cmd.args[3], right);
			cmd.clear();
		} else if (cmd.cmd3 == 0x15e || cmd.cmd3 == 0x161 || cmd.cmd3 == 0x162 || cmd.cmd3 == 0x14c || cmd.cmd3 == 0x7d1) {
/* 15e, 161, 162, 14c, 7d1 : なんらかのシステム情報を返す（skip modeなど？） */
/* 7d1: == 1 || 14c: == 1 || (15e==1&&161==1&&162==0) || (press_val == 2) : スキップ中？ タイトル画面のアニメーション終了 */
			flag.SetSys(0);
			cmd.clear();
		} else if (cmd.cmd3 == 0x4b0) { // 終了
			System::Main::Quit();
			script = 0; script_start = 0; script_end = 0;
			cmd.clear();
			cmd.cmd_type = CMD_WAITFRAMEUPDATE;
		} else if (cmd.cmd3 == 0x4b4 || cmd.cmd3 == 0x4b5) { // 選択肢巻き戻し
			LoadRollback(cmd);
		} else if (cmd.cmd3 == 0x58d) {
        		// 前にロード|セーブされた番号を返す。
			flag.SetSys(-1);
		} else if (cmd.cmd3 == 0x585) {
        		// 第一引数の記録された日付、タイトルなどが返される
        		// データがないなら sys に 0が、あるなら 1 が返る
			int y,m,d,wd,h,min,s,ms;
			string title;
fprintf(stderr,"StatSave %d:",cmd.args[0].value+1);
			if (StatSaveFile(cmd.args[0].value+1,y,m,d,wd,h,min,s,ms,title) == true) {
				flag.Set(cmd.args[1], y);
				flag.Set(cmd.args[2], m);
				flag.Set(cmd.args[3], d);
				flag.Set(cmd.args[4], wd);
				flag.Set(cmd.args[5], h);
				flag.Set(cmd.args[6], min);
				flag.Set(cmd.args[7], s);
				flag.Set(cmd.args[8], ms);
				if (cmd.args[9].type == TYPE_VARSTR) {
					flag.SetStr(cmd.args[9].number, kconv_rev(title));
				}
				flag.SetSys(1);
			} else {
				flag.SetSys(0);
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0xc23) { // save
			Save(cmd);
		} else if (cmd.cmd3 == 0xc25) { // load
			Load(cmd);
		} else if (cmd.cmd3 == 0x4b1 || cmd.cmd3 == 0x4b3) { // menu へ戻る (4b3: バッドエンド)
			int scn_start;
			if (config.GetParam("#SEEN_MENU", 1, &scn_start) == 0) {
				ChangeScript(scn_start, 0);
				save_scn = 0;
				save_point = 0;
				window_title = "";
				const char* window_title_config = config.GetParaStr("#CAPTION");
				if (window_title_config) window_title = window_title_config;
				parent.Root().SetWindowCaption(kconv(window_title).c_str());
				stack.clear();
				cmd_stack.clear();
				cmd_stack_str = cmd_stack_str_orig;
				flag.Load("");
				text_exec.Load("");
				grp_exec.Load("");
				SetSkipMode(SKIP_NO);
			}
		} else if (cmd.cmd3 == 0xcc) {
			eprintf("show mouse cursor\n");
			ShowCursor();
			cmd.clear();
		} else if (cmd.cmd3 == 0xcd) {
			eprintf("hide mouse cursor\n");
			HideCursor();
			cmd.clear();
		} else if (cmd.cmd3 == 0xcf) {
			mouse_type = cmd.args[0].value;
			eprintf("change mouse cursor : %d\n", mouse_type);
			if (mouse_surface) ShowCursor();
			cmd.clear();
		}
	}

}

#include<sys/types.h>
#include<sys/stat.h>

#ifdef ENABLE_PATCH
#if !defined(WINCE)
#include<errno.h>
#endif
#endif

#include<unistd.h>

// セーブファイルの名前をつくる
string Scn2k::MakeSaveFile(void) const {
	struct stat sstatus;

#ifdef ENABLE_PATCH
#if defined(PSP) || defined(WINCE)
	string dir = ".";
#else
	string dir = "~/.xkanon";
#endif

#if !defined(WINCE)
	if (dir.c_str()[0] == '~' && dir.c_str()[1] == '/') {
		char* home = getenv("HOME");
		if (home != 0) {
			string new_dir = string(home) + (dir.c_str()+1);
			dir = new_dir;
		}
	}
#endif
#endif

	// savepathにファイル名が入っていれば、それをセーブファイルとして使う
	if (stat(dir.c_str(), &sstatus) == -1) {
#ifdef ENABLE_PATCH
#if defined(WINCE)
		return "";
#else
		if (errno != ENOENT) {
			fprintf(stderr,"Cannot open save file; dir %s is not directory\n",dir.c_str());
			return "";
		}
		if (mkdir(dir.c_str(), S_IRWXU) != 0 && errno != EEXIST) {
			fprintf(stderr, "Cannot create directory %s ; Please create manually!!\n",dir.c_str());
		}
#endif
#endif
	} else {
		if ( (sstatus.st_mode & S_IFMT) == S_IFREG) {
			return dir;
		}
	}
	// ファイル名を作る
	const char* regname = config.GetParaStr("#REGNAME");

	char* fname = new char[strlen(regname)+1];
	/* レジストリ名をファイル名として有効なものにする */
	int i; for (i=0; regname[i]!=0; i++) {
		char c = regname[i];
		if (c == '\\' || c == '/' || c == ':' || c <= 0x20) c = '_';
		fname[i] = tolower(c);
	}
	fname[i] = 0;
	dir += "/save.";
	dir += fname;
	return dir;
}
// セーブファイルの名前をつくる
string Scn2kSaveTitle::operator() (int number) const {
	int y,m,d,wd,h,min,sec,msec;
	string title;
	if (! impl.StatSaveFile(number, y,m,d,wd,h,min,sec,msec,title)) {
		return "";
	} else {
		char buf[1024];
		sprintf(buf, "%2d/%2d %2d:%2d ",m,d,h,min);
		return string(buf) + title;
	}
};

void Scn2k::SaveSys(void) {
	char buf[1024];
	string save;
	string path = MakeSaveFile();
	
	sprintf(buf, "KEY=%s\n", config.GetParaStr("#REGNAME")); save += buf;
	string save_config;
	config.DiffOriginal(save_config);
	save += "CONFIG=";
	save += save_config;
	save += "\n";
	string save_flag; flag.SaveSys(save_flag);
	save += save_flag;
	string save_grp; grp_exec.SaveSys(save_grp);
	save += save_grp;
	map<int,set<int> >::iterator it;
	save += "[TextRead]\n";
	for (it=text_readflag.begin(); it != text_readflag.end(); it++) {
		set<int>& read_flag = it->second;
		set<int>::iterator jt;
		char buf[1024];
		sprintf(buf,"T<%05d>=",it->first);
		string save_readflag = buf;
		for (jt=read_flag.begin(); jt != read_flag.end(); jt++) {
			sprintf(buf, "%d,", *jt);
			save_readflag += buf;
		}
		save_readflag += "\n";
		save += save_readflag;
	}

	path += ".0";
	FILE* f = fopen(path.c_str(), "w");
	if (f == 0) {
		fprintf(stderr,"Cannot open save file %s\n",path.c_str());
		return;
	}
	fwrite(save.c_str(), save.length(), 1, f);
	fclose(f);
	return;
}

void Scn2k::LoadSys(void) {
	char buf[1024];
	string path = MakeSaveFile();
	path += ".0";
	FILE* f = fopen(path.c_str(), "r");
	if (f == 0) {
		fprintf(stderr, "Cannot open save file %s\n",path.c_str());
	} else {
		fseek(f,0,2);
		int sz = ftell(f);
		fseek(f,0,0);
		char* savedata = new char[sz+1];
		fread(savedata, sz, 1, f);
		savedata[sz] = 0;
		fclose(f);

		sprintf(buf, "KEY=%s\n", config.GetParaStr("#REGNAME"));
		if (strncmp(savedata, buf, strlen(buf)) != 0) {
			fprintf(stderr,"Invalid header in save file %s: it must be started with \"%s\"\n",buf);
		} else  {
			char* config_str = strstr(savedata, "\nCONFIG=");
			if (config_str) {
				config_str += strlen("\nCONFIG=");
				char* strend = strchr(config_str, '\n');
				if (strend) {
					int l = strend - config_str;
					char* config_copy = new char[l+1];
					strncpy(config_copy, config_str, l);
					config_copy[l] = 0;
					config.PatchOriginal(config_copy);
					delete[] config_copy;
				}
			}
			flag.LoadSys(savedata);
			grp_exec.LoadSys(savedata);
			char* save = strstr(savedata, "\n[TextRead]\n");
			if (save) {
				save += strlen("\n[TextRead]\n");
				do {
					if (save[0] == '[') break; // next section
					char* next_save = strchr(save, '\n');
					if (next_save) {
						*next_save++ = 0;
					}
					// T<XXXXX>=YYY,YYY,YYY,...
					if (strncmp(save,"T<",2) == 0) {
						int scn_num = atoi(save+2);
						set<int>& read_flag = text_readflag[scn_num];
						save += strlen("T<XXXXX>=");
						while(save && *save) {
							if (save[0] >= '0' && save[0] <= '9') {
								int num = atoi(save);
								read_flag.insert(num);
							}
							save = strchr(save, ',');
							if (save) save++;
						}
					}
					save = next_save;		
				} while(save);
			}

		}
		delete[] savedata;
	}

	/* 初期化 */
	int scn_start; config.GetParam("#SEEN_START", 1, &scn_start);
	ChangeScript(scn_start, 0);
	save_scn = 0;
	save_point = 0;
	window_title = "";
	const char* window_title_config = config.GetParaStr("#CAPTION");
	if (window_title_config) window_title = window_title_config;
	parent.Root().SetWindowCaption(kconv(window_title).c_str());
	stack.clear();
	cmd_stack.clear();
	cmd_stack_str = cmd_stack_str_orig;

	return;
}

bool Scn2k::StatSaveFile(int num, int& year, int& month, int& day, int& wday, int& hour,int& min, int& sec, int& msec, string& title) const {
	char buf[1024];
	string path = MakeSaveFile();
	if (num <= 0 || num > 99) return false;
	sprintf(buf,".%d",num);
	path += buf;

	struct stat sb;
	if (stat(path.c_str(), &sb) == -1) return false;
	struct tm* t = localtime((const time_t*)&sb.st_mtime);
	month = t->tm_mon + 1;
	day = t->tm_mday;
	hour = t->tm_hour;
	min = t->tm_min;
	/* タイトルの取得 */
	FILE* savefile = fopen(path.c_str(), "rb");
	if (savefile == 0) return false;
	char regname[1024];
	sprintf(regname, "KEY=%s\n", config.GetParaStr("#REGNAME"));
	fgets(buf,1000,savefile);
	if (strncmp(regname, buf, strlen(regname)) != 0) {
		fprintf(stderr,"invalid save file %s (registory name is not %s)\n",path.c_str(),regname);
		fclose(savefile);
		return false;
	}
	title="none";
	while(!feof(savefile)) {
		fgets(buf,1000,savefile);
		if (strncmp(buf,"Title=",6) == 0) {
			if (buf[strlen(buf)-2] == 0x0a) buf[strlen(buf)-2] = 0;
			if (strlen(buf) > 20) buf[20] = 0, buf[21] = 0;
			title = kconv(buf+6);
			break;
		}
	}
	fclose(savefile);
	return true;
}

void Scn2k::SaveRollback(void) {
fprintf(stderr,"Save rollback\n");
	new_rollback_save = "";
	string save_sys; SaveImpl(save_sys);
	string save_flag; flag.Save(save_flag);
	string save_text; text_exec.Save(save_text, true);
	string save_grp; grp_exec.Save(save_grp);
	new_rollback_save += save_sys;
	new_rollback_save += save_flag;
	new_rollback_save += save_text;
	new_rollback_save += save_grp;
}

void Scn2k::LoadRollback(Cmd& cmd) {
	if (rollback_save.empty()) return;
	new_rollback_save = "";
	string savedata = rollback_save.back();
	rollback_save.pop_back();
	LoadImpl(savedata.c_str());
	flag.Load(savedata.c_str());
	text_exec.Load(savedata.c_str());
	grp_exec.Load(savedata.c_str());

	/* 画面の回復など */
	SetSkipMode(SKIP_NO);
	vector<CmdSimplified>::iterator it;
	cmd.clear();
	for (it = cmd_stack.begin(); it != cmd_stack.end(); it++) {
		cmd.read(*it);
		cmd.cmd_type = CMD_OTHER;
		flag.Exec(cmd);
		text_exec.Exec(cmd);
		grp_exec.Exec(cmd);
	}
	cmd.clear();
	return;
}

void Scn2k::Save(Cmd& cmd) {
	if (cmd.cmd_type == CMD_SAVEREQ) {
		if (menu == 0) {
			SetSkipMode(SKIP_IN_MENU); // テキストスキップ等はここで中断
			menu = new Scn2kMenu(Scn2kMenu::MENU_SAVE, *this, flag, text_exec, system_version);
			menu->InitPanel(event, parent);
			menu->InitTitle(Scn2kSaveTitle(*this));
			if (mouse_surface) menu_mouseshown = true;
			else menu_mouseshown = false;
			ShowCursor();
			return;
		}
	}
	char buf[1024];
	string save;
	FILE* f = 0;
	if (save_scn == 0) {
		fprintf(stderr,"Cannot decide save point\n");
		return; // セーブ位置が保存されてない
	}
	string path = MakeSaveFile();
	int file_number = 1;
	if (cmd.args.size() == 1)
		file_number = cmd.args[0].value + 1;
	if (file_number <= 0) {
		fprintf(stderr, "Cannot open save file %s\n",path.c_str());
		return;
	}
	sprintf(buf, ".%d",file_number);
	path += buf;

	/* セーブファイル確認 */
	
	sprintf(buf, "KEY=%s\n", config.GetParaStr("#REGNAME")); save += buf;
	string save_sys; SaveImpl(save_sys);
	string save_flag; flag.Save(save_flag);
	string save_text; text_exec.Save(save_text, false);
	string save_grp; grp_exec.Save(save_grp);
	save += save_sys;
	save += save_flag;
	save += save_text;
	save += save_grp;
	vector<string>::iterator it;
	for (it=rollback_save.begin(); it != rollback_save.end(); it++) {
		save += "[Rollback Data]\n";
		save += *it;
		save += "[Rollback End]\n";
	}

	f = fopen(path.c_str(), "w");
	if (f == 0) {
		fprintf(stderr,"Cannot open save file %s\n",path.c_str());
		return;
	}
	fwrite(save.c_str(), save.length(), 1, f);
	fclose(f);
	cmd.clear();
	return;
}

void Scn2k::Load(Cmd& cmd) {
	if (cmd.cmd_type == CMD_LOADREQ) {
		if (menu == 0) {
			menu = new Scn2kMenu(Scn2kMenu::MENU_LOAD, *this, flag, text_exec, system_version);
			menu->InitPanel(event, parent);
			menu->InitTitle(Scn2kSaveTitle(*this));
			SetSkipMode(SKIP_IN_MENU); // テキストスキップ等はここで中断
			if (mouse_surface) menu_mouseshown = true;
			else menu_mouseshown = false;
			ShowCursor();
			return;
		}
	}
	char buf[1024];
	string path = MakeSaveFile();
	int file_number = 1;
	if (cmd.args.size() == 1)
		file_number = cmd.args[0].value + 1;
	sprintf(buf, ".%d",file_number);
	path += buf;
	FILE* f = 0;
	if (file_number > 0) f = fopen(path.c_str(), "r");
	if (f == 0) {
		fprintf(stderr, "Cannot open save file %s\n",path.c_str());
		return;
	}
	
	fseek(f,0,2);
	int sz = ftell(f);
	fseek(f,0,0);
	char* savedata = new char[sz+1];
	fread(savedata, sz, 1, f);
	savedata[sz] = 0;
	fclose(f);

	sprintf(buf, "KEY=%s\n", config.GetParaStr("#REGNAME"));
	if (strncmp(savedata, buf, strlen(buf)) != 0) {
		fprintf(stderr,"Invalid header in save file %s: it must be started with \"%s\"\n",buf);
		delete[] savedata;
		return;
	}
	LoadImpl(savedata);
	flag.Load(savedata);
	text_exec.Load(savedata);
	grp_exec.Load(savedata);
	rollback_save.clear();
	new_rollback_save = "";
	char* rollback_data = savedata;
	while( (rollback_data = strstr(rollback_data,"[Rollback Data]\n")) != 0) {
		rollback_data += strlen("[Rollback Data]\n");
		char* rollback_end = strstr(rollback_data, "[Rollback End]\n");
		if (rollback_end == 0) rollback_end = rollback_data + strlen(rollback_data);
		string s(rollback_data, rollback_end);
		rollback_save.push_back(s);
		rollback_data = rollback_end;
	}

	/* 画面の回復など */
	SetSkipMode(SKIP_NO);
	vector<CmdSimplified>::iterator it;
	for (it = cmd_stack.begin(); it != cmd_stack.end(); it++) {
		cmd.read(*it);
		cmd.cmd_type = CMD_OTHER;
		flag.Exec(cmd);
		text_exec.Exec(cmd);
		grp_exec.Exec(cmd);
	}
	cmd.clear();
	return;
}

void Scn2k::SaveImpl(string& save) {
	char buf[1024];

	/* save point */
	sprintf(buf, "\n[SCENARIO]\nScn=%d\nPoint=%d\n",save_scn, save_point); save += buf;
	sprintf(buf, "Title=%s\nMouseType=%d\nMouseShown=1\n",window_title.c_str(), mouse_type); save += buf;
	vector<StackItem>::iterator sit;
	for (sit=stack.begin(); sit!=stack.end(); sit++) {
		if (sit->scn_number == SCN_INFO && sit->scn_pt == SCN_INFO_MENU) break; // メニューに入る直前までのスタックを保存
		sprintf(buf, "Stack=%d,%d\n",sit->scn_number,sit->scn_pt);
		save += buf;
	}
	vector<string>::reverse_iterator ssit;
	for (ssit=stack_strbuffer.rbegin(); ssit != stack_strbuffer.rend(); ssit++) {
		sprintf(buf, "StackStr=%s\n",ssit->c_str());
		save += buf;
	}
	vector<CmdSimplified>::iterator cit;
	for (cit=cmd_stack.begin(); cit != cmd_stack.end(); cit++) {
		if (cit->type == CMD_SAVECMDGRP || cit->type == CMD_SAVECMDGRP_ONCE || cit->type == CMD_SAVECMDGRP_START) {
			save += "CmdG=";
		} else {
			save += "Cmd=";
		}
		string s; cit->Save(s);
		save += s;
		save += "\n";
	}
}

void Scn2k::LoadImpl(const char* save) {
	char buf[1024];
	save_scn = 0;
	save_point = 0;
	window_title = "";
	stack.clear();
	cmd_stack.clear();
	cmd_stack_str = cmd_stack_str_orig;

	save = strstr(save, "\n[SCENARIO]\n");
	if (save == 0) return;
	save += strlen("\n[SCENARIO]\n");
	while(save[0] != 0 && save[0] != '[') { // while next section start
		if (strncmp(save, "Scn=", 4) == 0) {
			sscanf(save, "Scn=%d", &save_scn);
		} else if (strncmp(save, "Point=", 6) == 0) {
			sscanf(save, "Point=%d", &save_point);
		} else if (strncmp(save, "Title=", 6) == 0) {
			save += 6;
			char* s = strchr(save, '\n');
			if (s == 0) window_title = save;
			else window_title.assign(save, s-save);
			const char* config_name = config.GetParaStr("#CAPTION");
			if (config_name == 0) config_name = "";
			string setname = kconv(string(config_name)+"  "+window_title);
			parent.Root().SetWindowCaption(setname.c_str());
		} else if (strncmp(save, "MouseType=", 10) == 0) {
			sscanf(save, "MouseType=%d", &mouse_type);
		} else if (strncmp(save, "MouseShown=", 11) == 0) {
			int v;
			sscanf(save, "MouseShown=%d", &v);
			if (v) ShowCursor();
			else HideCursor();
		} else if (strncmp(save, "Stack=", 6) == 0) {
			int scn, pt;
			sscanf(save, "Stack=%d,%d", &scn, &pt);
			stack.push_back( StackItem(scn, pt));
		} else if (strncmp(save, "StackStr=", 9) == 0) {
			save += 9;
			char* s = strchr(save, '\n');
			if (s == 0) stack_strbuffer.push_back("");
			else stack_strbuffer.push_back(string(save, s-save));
		} else if (strncmp(save, "Cmd=", 4) == 0) {
			CmdSimplified cmd;
			cmd.Load(save+4, cmd_stack_str);
			cmd_stack.push_back(cmd);
		} else if (strncmp(save, "CmdG=", 5) == 0) {
			CmdSimplified cmd;
			cmd.Load(save+5, cmd_stack_str);
			cmd.type = CMD_SAVECMDGRP;
			cmd_stack.push_back(cmd);
		}
		save = strchr(save, '\n');
		if (save != 0) save++;
	}
	ChangeScript(save_scn, 0);
	script = script_start + save_point;
	return;
}
void Scn2k::SetSkipMode(SkipMode mode) {
	if (skip_mode != mode) {
		skip_mode = mode;
		text_exec.SetSkipMode(mode);
		grp_exec.SetSkipMode(mode);
	}
}

/***********************************************************
**
**	DLL Call Implementation
**
**/
static double* lb_ef_param = 0;
void DLLCall_LB_EF00_0(Cmd& cmd, Flags& flags) { // エフェクトの設定
	if (lb_ef_param == 0) {
		lb_ef_param = new double[sizeof(double) * 0x60 * 8];
	}
	int i,j;
	int param_top, param_size;
	if (cmd.args[2].value == 1) {
		param_top = 0;
		param_size = 0x20;
	} else {
		param_top = cmd.args[3].value;
		param_size = cmd.args[4].value;
		if (param_top < 0) param_top = 0;
		if (param_top > 0x20) param_top = 0x20;
		if (param_size+param_top > 0x20) param_size = 0x20 - param_top;
	}
	for (i=0; i<8; i++) {
		double* param = lb_ef_param + i*0x60 + param_top*3;
		for (j=0; j<param_size; j++) {	
			*param++ = random() % 800 - 400;
			*param++ = random() % 600 - 300;
			*param++ = random() % 700 - 350;
		}
	}
	if (cmd.args[5].value != 1) return;
static int random_dirtable[] = {
		0, 2, 1, 3, 0, 2, 1, 3,
		1, 3, 2, 0, 1, 3, 2, 0,
		0, 0, 0, 0, 3, 1, 2, 0,
		3, 1, 3, 1, 0, 2, 3, 1
	};
	int* dir = &random_dirtable[(random()&3) * 8];
	for (i=0; i<8; i++) {
		double* param = lb_ef_param + i*0x60;
		double x = random()%600 - 300;
		double y = random()%480-240;
		if (x < 0) x -= 80;
		else x += 80;
		if (y < 0) y -= 80;
		else y += 80;
		switch(*dir++) {
		case 0:
			if (x < 0) x = -x;
			if (y < 0) y = -y;
			break;
		case 1:
			if (x > 0) x = -x;
			if (y < 0) y = -y;
			break;
		case 2:
			if (x < 0) x = -x;
			if (y > 0) y = -y;
			break;
		case 4:
			if (x > 0) x = -x;
			if (y > 0) y = -y;
			break;
		}
		param[9] = x*1.2;
		param[10] = y*1.2;
		param[11] *= 1.2;
		param[12] *= -0.08;
		param[13] *= -0.08;
		param[14] *= -0.08;
		param[15] = -param[9];
		param[16] = -param[10];
		param[17] = -param[11];
	}
	return;
}
void DLLCall_LB_EF00_1(Cmd& cmd, Flags& flags) { // 計算を行う
	if (lb_ef_param == 0) {
		fprintf(stderr,"Warning : DLLCall_LB_EF00_1 : Script error : effect calculation was called before setting\n");
		return;
	}
	int index = cmd.args[2].value;
	int v5_1154 = flags.Get(5, 1154+index);
	int j = ((v5_1154) & 0x1f) + index * 0x20;
	int k = ((v5_1154+1) & 0x1f) + index * 0x20;
	int l = ((v5_1154+2) & 0x1f) + index * 0x20;
	int m = ((v5_1154+3) & 0x1f) + index * 0x20;
	j *= 3;
	k *= 3;
	l *= 3;
	m *= 3;

	// 0 < x < 1
	// va - vd は 0-1 の範囲で対称性を持つ３次関数
	double x = double(flags.Get(5, 1162 + index)) * 0.001;
	double va = (x * x * x)/6;
	double vb = (-x*x*x + 3*x*x - 3*x + 1) / 6;
	double vc = (3*x*x*x - 6*x*x + 4) / 6;
	double vd = (-3*x*x*x+3*x*x+3*x+1) / 6;

	double r1 = va * lb_ef_param[m+3] + vd * lb_ef_param[l+3] + vc * lb_ef_param[k+3] + vb * lb_ef_param[j+3];
	double r2 = va * lb_ef_param[m+2] + vd * lb_ef_param[l+2] + vc * lb_ef_param[k+2] + vb * lb_ef_param[j+2];
	double r3 = va * lb_ef_param[m+1] + vd * lb_ef_param[l+1] + vc * lb_ef_param[k+1] + vb * lb_ef_param[j+1];
	if (r1 != 400) {
		r2 = r2 * 800 / (400-r1);
		r3 = r3 * 700 / (400-r1);
	}
	VarInfo var;
	var.type = 5;
	var.number = 1151;
	flags.Set(var, int(r2));
	var.number = 1152;
	flags.Set(var, int(r3));
	var.number = 1153;
	flags.Set(var, int(r1));
	return;
}


void DllCall_LB(Cmd& cmd, Flags& flags) {	// リトルバスターズ！の EF00.dll をエミュレート
	if (cmd.args[0].value == 1) {
		// "EF00.dll"
		if (cmd.args[1].value == 0) { // エフェクトの設定
			DLLCall_LB_EF00_0(cmd, flags);
		} else if (cmd.args[1].value == 1) { // 計算を行う
			DLLCall_LB_EF00_1(cmd, flags);
		}
	} else {
		fprintf(stderr,"Unsupported DLL call for DLL<%d>\n",cmd.args[0].value);
	}
	return;
}

/**********************************************************
**
**	MenuImpl
**
*/

#include"window/widget.h"
#include"window/menuitem.h"

void DSurfaceFill(Surface* src, const Rect& rect, int r, int g, int b, int a = 0xff);

struct Scn2kMenuImpl {
	Scn2kMenu& interface;
	MenuItem* menu;
	Event::Container* pevent;
	PicContainer* pparent;

	virtual void InitPanel(Event::Container& event, PicContainer& parent) = 0;
	virtual void InitTitle(const SaveTitle&) = 0;
	virtual void Cancel(void) = 0;
	virtual void Exec(Cmd& cmd) = 0;
	Scn2kMenuImpl(Scn2kMenu& _interface) : interface(_interface) {
		menu = 0;
		pevent = 0;
		pparent = 0;
	}
	virtual ~Scn2kMenuImpl() {
		if (menu) delete menu;
		menu = 0;
	}
};

struct LoadMenu : Scn2kMenuImpl {
	vector<string> title;
	vector<int> title_valid;
	RadioButton* btn_local;
	RadioButton* btn_page;
	RadioButton* btn_set;
	Scale* btn_scale;
	Dialog* awk_dialog;
	int btn_page_val, btn_set_val, btn_local_val, select_page, select_value;
	LoadMenu(Scn2kMenu& _interface);
	~LoadMenu();
	void InitPanel(Event::Container& event, PicContainer& parent);
	void InitTitle(const SaveTitle&);
	void Cancel(void);
	void Exec(Cmd& cmd);
	static void ChangeBtnPage(void* pointer, MenuItem* widget);
	static void ChangeBtnLocal(void* pointer, MenuItem* widget);
	static void ChangeBtnScale(void* pointer, Scale* widget);
	static void ChangeBtnSet(void* pointer, MenuItem* widget);
	static void ChangeDialog(void* pointer, Dialog* widget);
	bool in_setpage;
	void SetPage(int new_page);
	void SetValue(int new_value);
	void PressOk(void);
};
LoadMenu::LoadMenu(Scn2kMenu& _interface) : Scn2kMenuImpl(_interface) {
	btn_local = 0;
	btn_scale = 0;
	btn_set = 0;
	btn_page_val = 0;
	btn_set_val = -1;
	btn_local_val = -1;
	awk_dialog = 0;
	in_setpage = false;
	select_page = 0;
	select_value = -1;
}
LoadMenu::~LoadMenu() {
	if (awk_dialog) delete awk_dialog;
}
void LoadMenu::InitPanel(Event::Container& event, PicContainer& parent) {
	pevent = &event;
	pparent = &parent;

	if (menu) delete menu;
	menu = 0;
	menu = new MenuItem(&parent, Rect(80,30,560, 450), 1, 3, 0);
	Surface* surface = parent.Root().NewSurface(menu->Pic()->Width(), menu->Pic()->Height(), ALPHA_MASK);
	if (interface.type == Scn2kMenu::MENU_LOAD) {
		menu->SetLabelTop(new Label(menu->PicNode(), Rect(0,0), true, "Load", 26), Rect(0,0,10,0), Rect(0,0,0,20));
		DSurfaceFill(surface, Rect(*surface), 0, 0, 0x80, 0x80);
	} else {
		menu->SetLabelTop(new Label(menu->PicNode(), Rect(0,0), true, "Save", 26), Rect(0,0,10,0), Rect(0,0,0,20));
		DSurfaceFill(surface, Rect(*surface), 0, 0x80, 0, 0x80);
	}
	menu->Pic()->SetSurface(surface, 0, 0);
	menu->Pic()->SetSurfaceFreeFlag();

	btn_page = new RadioButton(event, menu->PicNode(), Rect(0, 0, 480, 40), 10, 1, &btn_page_val,
		Rect(0,0,0,0), 18, Color(0,0,0),Color(0xff,0,0),Color(0xff,0x80,0));
	btn_page->set_func = &ChangeBtnPage;
	btn_page->set_pointer = this;
	btn_page->SetLabelLeft(new Label(btn_page->PicNode(), Rect(0,0), true, "Page", 18), Rect(0, 0, 180, 0), Rect(0,0));
	btn_page->Add(" 1 ");
	btn_page->Add(" 2 ");
	btn_page->Add(" 3 ");
	btn_page->Add(" 4 ");
	btn_page->Add(" 5 ");
	btn_page->Add(" 6 ");
	btn_page->Add(" 7 ");
	btn_page->Add(" 8 ");
	btn_page->Add(" 9 ");
	btn_page->Add(" 10 ");
	btn_page->pack();
/*
	surface = parent.Root().NewSurface(btn_page->Pic()->Width(), btn_page->Pic()->Height(), ALPHA_MASK);
	DSurfaceFill(surface, Rect(*surface), 0xff, 0, 0, 0x80);
	btn_page->Pic()->SetSurface(surface, 0, 0);
	btn_page->Pic()->SetSurfaceFreeFlag();
*/
	menu->item[0] = btn_page;
	btn_set = new RadioButton(event, menu->PicNode(), Rect(0, 0, 480, 40), 2, 1, &btn_set_val,
		Rect(0,0,0,0), 18, Color(0,0,0),Color(0xff,0,0),Color(0xff,0x80,0));
	btn_set->set_func = &ChangeBtnSet;
	btn_set->set_pointer = this;
	btn_set->SetLabelLeft(new Label(btn_set->PicNode(), Rect(0,0)), Rect(0,0,200,0), Rect(0,0));
	if (interface.type == Scn2kMenu::MENU_LOAD) {
		btn_set->Add(" Load ");
	} else {
		btn_set->Add(" Save ");
	}
	btn_set->Add(" Cancel ");
	btn_set->pack();
/*
	surface = parent.Root().NewSurface(btn_set->Pic()->Width(), btn_set->Pic()->Height(), ALPHA_MASK);
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0xff, 0x80);
	btn_set->Pic()->SetSurface(surface, 0, 0);
	btn_set->Pic()->SetSurfaceFreeFlag();
*/
	menu->item[2] = btn_set;
	// void btn_set_press(void* pointer, MenuItem* widget);
	// btn_set->set_func = btn_set_press;
	// btn_set->set_pointer = this;
	btn_local = new RadioButton(*pevent, menu->PicNode(), Rect(0, 0, 480, 300), 1, 100, &btn_local_val,
		Rect(0,0,300,30), 18, Color(0,0,0),Color(0xff,0,0),Color(0xff,0x80,0));
	btn_local->set_func = &ChangeBtnLocal;
	btn_local->set_pointer = this;
/*
	surface = pparent->Root().NewSurface(btn_local->Pic()->Width(), btn_local->Pic()->Height(), ALPHA_MASK);
	DSurfaceFill(surface, Rect(*surface), 0, 0xff, 0, 0x80);
	btn_local->Pic()->SetSurface(surface, 0, 0);
	btn_local->Pic()->SetSurfaceFreeFlag();
*/
	menu->item[1] = btn_local;
	int i;
	for (i=0; i<12; i++)
		btn_local->Add("",false);
	btn_local->pack();
	btn_local->show_all();
	menu->pack();

	PicBase* local_pic = btn_local->Pic();
	int local_x2 = local_pic->PosX() + local_pic->Width();
	int local_y2 = local_pic->PosY() + local_pic->Height();
	btn_scale = new Scale(*pevent, menu->PicNode(), Rect(local_x2-16, local_pic->PosY(), local_x2, local_y2), Color(0xff, 0x80, 0), true);
	btn_scale->SetRange(0, 900);
	btn_scale->InitCursor(1024/10);
	btn_scale->SetValue(0);
	btn_scale->change_func = &ChangeBtnScale;
	btn_scale->change_pointer = this;

	menu->PicNode()->show_all();
}

void LoadMenu::InitTitle(const SaveTitle& title_op) {
	title.clear();
	int i;
	for (i=1; i<=100; i++) {
		char buf[100];
		sprintf(buf,"%2d:",i);
		string t = title_op(i);
		string s = string(buf) + t;
		if (t.length() == 0) {
			string s = string(buf) + "--------";
			title_valid.push_back(0);
		} else {
			title_valid.push_back(1);
		}
		title.push_back(s);
	}
	if (btn_local==0) return;
	for (i=0; i<10; i++) {
		TextButton* button = dynamic_cast<TextButton*>(btn_local->item[i]);
		if (button) button->SetText(title[i].c_str());
	}
}

void LoadMenu::SetPage(int new_page) {
	if (new_page < 0) new_page = 0;
	if (new_page > 900) new_page = 900;
	if (select_page == new_page) return;
	if (in_setpage) return;
	in_setpage = true;
	
	int prev_page = select_page / 10;
	int cur_page = new_page / 10;
	int prev_point = select_page%10;
	int new_point = new_page%10;
	select_page = new_page;
	if (prev_page != cur_page) {
		int i;
		for (i=0; i<12; i++) {
			TextButton* button = dynamic_cast<TextButton*>(btn_local->item[i]);
			if (button) {
				if (cur_page+i < title.size()) button->SetText(title[cur_page+i].c_str());
				else button->SetText("----");
			}
		}
		// ボタンの内容を変更する
		if (select_value < cur_page || select_value > cur_page+12)
			btn_local->SetValue(-1);
		else
			btn_local->SetValue(select_value - cur_page);
	}
	if (prev_point != new_point) {
		int i;
		for (i=0; i<12; i++) {
			int old_x = btn_local->item[i]->Pic()->PosX();
			btn_local->item[i]->Pic()->Move(old_x, i*30-new_point*3);
		}
	}
	if (btn_page) {
		if (select_page%100 == 0) btn_page->SetValue(select_page/100);
		else btn_page->SetValue(-1);
	}
	if (btn_scale) {
		btn_scale->SetValue(select_page);
	}
	in_setpage = false;
	return;
}
void LoadMenu::SetValue(int new_value) {
	if (in_setpage) return;
	in_setpage = true;

	if (new_value < 0 || new_value > title.size() ||
	    (interface.type == Scn2kMenu::MENU_LOAD && title_valid[new_value] == 0) ) { // 無効な選択肢
		if (select_value < select_page/10 || select_value > select_page/10+12)
			btn_local->SetValue(-1);
		else
			btn_local->SetValue(select_value-select_page/10);
	} else { // 選択肢を変更する
		if (select_value == new_value) {
			PressOk(); // ダブルクリック
		} else {
			select_value = new_value;
			if (interface.type == Scn2kMenu::MENU_SAVE && title_valid[select_value] == 0) {
				PressOk(); // 新しいセーブデータなら無条件に選択
			}
		}
	}

	in_setpage = false;
	return;
}
void LoadMenu::PressOk(void) {
	if (select_value == -1) {
		btn_set->SetValue(-1); // なにもしない
		return;
	}
	menu->deactivate();
	if (interface.type == Scn2kMenu::MENU_LOAD) {
		interface.cmd.cmd_type = CMD_LOAD;
		interface.cmd.args.push_back(VarInfo(select_value));
		awk_dialog = new Dialog(*pevent, pparent, "ファイルをロードしますか？", true);
		awk_dialog->set_pointer = this;
		awk_dialog->set_func = ChangeDialog;
	} else {// MENU_SAVE
		interface.cmd.cmd_type = CMD_SAVE;
		interface.cmd.args.push_back(VarInfo(select_value));
		if (title_valid[select_value] == 0) { // 新しいセーブデータ
			interface.status = Scn2kMenu::MenuStatus(Scn2kMenu::MENU_CMD | Scn2kMenu::MENU_DELETE);
		} else { // セーブデータを上書き：確認
			awk_dialog = new Dialog(*pevent, pparent, "データを上書きしますか？", true);
			awk_dialog->set_pointer = this;
			awk_dialog->set_func = ChangeDialog;
		}
	}
}
void LoadMenu::Cancel(void) {
	if (awk_dialog) { // ダイアログのキャンセル
		awk_dialog->status = Dialog::CANCEL;
		ChangeDialog(this, awk_dialog);
	} else { // 一般キャンセル
		btn_set->SetValue(1);
	}
}
void LoadMenu::Exec(Cmd& cmd) {
}
void LoadMenu::ChangeBtnPage(void* pointer, MenuItem* widget) {
	LoadMenu* instance = (LoadMenu*)pointer;
	if (instance->btn_page_val == -1) return;
	instance->SetPage(instance->btn_page_val*100);
}
void LoadMenu::ChangeBtnScale(void* pointer, Scale* from) {
	LoadMenu* instance = (LoadMenu*)pointer;
	int value = from->GetValue();
	instance->SetPage(value);
}
void LoadMenu::ChangeBtnSet(void* pointer, MenuItem* widget) {
	LoadMenu* instance = (LoadMenu*)pointer;
	if (instance->btn_set_val == 1) { // cancel
		instance->interface.status = Scn2kMenu::MENU_DELETE;
		return;
	} else if (instance->btn_set_val == 0) { // OK
		instance->PressOk();
	}
}
void LoadMenu::ChangeDialog(void* pointer, Dialog* widget) {
	LoadMenu* instance = (LoadMenu*)pointer;
	if (widget->status == Dialog::CANCEL) {
		// ダイアログ消去、OK ボタン復帰
		delete instance->awk_dialog;
		instance->awk_dialog = 0;
		instance->menu->activate();
		instance->btn_set->SetValue(-1);
		return;
	} else if (widget->status == Dialog::OK) {
		instance->interface.status = Scn2kMenu::MenuStatus(Scn2kMenu::MENU_CMD | Scn2kMenu::MENU_DELETE);
		return;
	}
}
void LoadMenu::ChangeBtnLocal(void* pointer, MenuItem* widget) {
	LoadMenu* instance = (LoadMenu*)pointer;
	if (instance->btn_local_val == -1) return;
	instance->SetValue( (instance->select_page/10) + instance->btn_local_val);
}

struct BacklogMenu : Scn2kMenuImpl {
	Scn2k& scn_impl;
	Text& text_exec;
	bool backlog_update;
	int backlog_cnt;
	BacklogMenu(Scn2kMenu& _interface, Scn2k& scn_impl, Text& text_exec);
	~BacklogMenu();
	void InitPanel(Event::Container& event, PicContainer& parent);
	void InitTitle(const SaveTitle&);
	void Cancel(void);
	void Exec(Cmd& cmd);
};
BacklogMenu::BacklogMenu(Scn2kMenu& _interface, Scn2k& _scn, Text& parent_text_exec) : Scn2kMenuImpl(_interface), scn_impl(_scn), text_exec(parent_text_exec) {
	backlog_cnt = -1;
	backlog_update = false;
}
BacklogMenu::~BacklogMenu() {
}
void BacklogMenu::InitPanel(Event::Container& event, PicContainer& parent) {
	pevent = &event;
}

void BacklogMenu::InitTitle(const SaveTitle& title_op) {
}
void BacklogMenu::Cancel(void) {
	interface.status = Scn2kMenu::MenuStatus(Scn2kMenu::MENU_DELETE);
}
void BacklogMenu::Exec(Cmd& cmd) {
	int command_direction = 0; // forward
	if (cmd.cmd_type == CMD_NOP) text_exec.Wait(0xffffffffUL, cmd);
	if (cmd.cmd_type == CMD_BACKLOGREQ || pevent->presscount(MOUSE_UP)) {
		if (cmd.cmd_type == CMD_BACKLOGREQ) cmd.clear();
		backlog_cnt++;
		backlog_update = false;
		command_direction = 1;
	}
	if (cmd.cmd_type == CMD_BACKLOGREQ_FWD || pevent->presscount(MOUSE_DOWN)) {
		if (cmd.cmd_type == CMD_BACKLOGREQ_FWD) cmd.clear();
		backlog_cnt--;
		backlog_update = false;
		if (backlog_cnt == -2 || (
		   (backlog_cnt == -1 && text_exec.backlog_item.scn == -1 && text_exec.backlog_item.pos == -1)) ){
			Cancel();
			return;
		}
		command_direction = -1;
	}
	if (cmd.cmd_type != CMD_NOP) return;
	if (backlog_update) return;
	// backlog を最新の状態に更新
	cmd.clear();
	BacklogItem item;

retry:
	if (backlog_cnt < -1) backlog_cnt = -1;
	if (backlog_cnt >= int(text_exec.backlog.size())) backlog_cnt = text_exec.backlog.size() - 1;

	if (backlog_cnt == -1) {
		if (text_exec.backlog_item.scn == -1 && text_exec.backlog_item.pos == -1) {
			if (text_exec.backlog.size() == 0 || command_direction < 0) {
				Cancel();
				return;
			}
			item = text_exec.backlog.back();
			backlog_cnt = 0;
		} else {
			// item = text_exec.backlog.back();
			item = text_exec.backlog_item;
		}
	} else {
		item = text_exec.backlog[text_exec.backlog.size()-1-backlog_cnt];
	}
	if (item.scn ==  BacklogItem::SaveSelect) { // select marker ; skip this item
		if (command_direction == 0) command_direction = 1;
		backlog_cnt += command_direction;
		goto retry;
	}
	if (item.scn == 0 && item.pos == -1) ; // not read cmd
	else {
		scn_impl.ReadCmdAt(cmd, item.scn, item.pos);
	}
	text_exec.DrawBacklog(item, cmd);
	cmd.clear();
	backlog_update = true;
}

/*******************************************************************************
**
**
*/

Scn2kMenu::Scn2kMenu(MenuType _type, Scn2k& scn_impl, const Flags& flags, Text& text_exec, int system_version) :
	cmd(flags, system_version), type(_type) {
	pimpl = 0;
	status = MENU_CONTINUE;
	switch(type) {
	case MENU_LOAD: pimpl = new LoadMenu(*this); break;
	case MENU_SAVE: pimpl = new LoadMenu(*this); break;
	case MENU_BACKLOG: pimpl = new BacklogMenu(*this, scn_impl, text_exec); break;
	}
	return;
}
Scn2kMenu::~Scn2kMenu() {
	if (pimpl) delete pimpl;
	pimpl = 0;
}
void Scn2kMenu::InitPanel(Event::Container& event, PicContainer& parent) {
	if (pimpl) pimpl->InitPanel(event, parent);
}
void Scn2kMenu::InitTitle(const SaveTitle& t) {
	if (pimpl) pimpl->InitTitle(t);
}
void Scn2kMenu::Cancel(void) {
	if (pimpl) pimpl->Cancel();
}
void Scn2kMenu::Exec(Cmd& ret_cmd) {
	if (pimpl == 0) return;
	pimpl->Exec(ret_cmd);
	if (pimpl->pevent->presscount(MOUSE_RIGHT)) {
		Cancel();
	}
	if (status & MENU_CMD && cmd.cmd_type != CMD_NOP) {
		status = Scn2kMenu::MenuStatus(status & (~Scn2kMenu::MENU_CMD) );
		CmdSimplified tmp_cmd;
		char cmd_str[32768];
		char* tmp_cmd_str = cmd_str;
		cmd.write(tmp_cmd, tmp_cmd_str);
		ret_cmd.read(tmp_cmd);
	}
}
void Scn2kMenu::activate(void) {
	if (pimpl && pimpl->menu) pimpl->menu->activate();
}
void Scn2kMenu::deactivate(void) {
	if (pimpl && pimpl->menu) pimpl->menu->deactivate();
}

