#ifndef __SCN2k_H__
#define __SCN2k_H__

#include<vector>
#include<string>
#include<map>
#include<set>

/*
namespace Widget {
	class Text;
	class Button;
	class TextButton;
	class Label;
	class Dialog;
	class AnmTime;
}
*/
	class WidText;
	class WidButton;
	class WidTextButton;
	class WidLabel;
	class WidDialog;
	class WidAnmTime;
namespace Event {
	class Container;
}
class PicBase;
class PicContainer;
class AyuSysConfig;
class Surface;

void dprintf(const char* fmt, ...);
void eprintf(const char* fmt, ...);

struct VarInfo {
#define TYPE_NONSYSVARMAX 5
#define TYPE_VARMAX 9
#define TYPE_VARLOCSTR 10
#define TYPE_VARSYSSTR 12
#define TYPE_VARSTR 18
#define TYPE_STR 58
#define TYPE_VAL 68
#define TYPE_SYS 0xc8
#define TYPE_END 0x7f

#define TYPE_SYS_SYS 0
#define TYPE_SYS_SKIPMODE 1
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
**      type 10,11: ローカル整数??、各2000個
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
	int var[TYPE_VARMAX+1][2000];
	std::string str[2000];
	std::string sys_str[2000];
	std::string loc_str[3];
public:
	Flags(void);
	int operator () () const;
	int operator () (VarInfo info) const;
	void Str(int type, unsigned int number, char* buf, int sz) const;
	std::string Str(int type, unsigned int number) const;
	std::set<int> cgm_data;

	bool IsInt(int type) const;
	int MaxIndex(int type) const;

	void Set(VarInfo info, int value);
	int Get(int type, int number) const;
	void SetSys(int value);
	void SetStr(VarInfo info, std::string val);

	bool Exec(class Cmd& cmd);

	void Save(std::string& str);
	void Load(const char* str);

	void SaveSys(std::string& str);
	void LoadSys(const char* str);
};

/* commands */
#define STRHEAP_SIZE 10000
enum Cmdtype {
	CMD_NOP, CMD_FLAGS, CMD_JMP, CMD_TEXT, CMD_OTHER, CMD_SYSVAR,
	CMD_TEXTEND,
	CMD_SAVECMDGRP, CMD_SAVECMDGRP_START, CMD_SAVECMDGRP_ONCE, CMD_SAVECMD_ONCE, CMD_WAITFRAMEUPDATE,CMD_SAVEPOINT, CMD_ROLLBACKPOINT,
	CMD_SAVEREQ, CMD_SAVE,
	CMD_LOADREQ, CMD_LOAD,
	CMD_MENUREQ,
	CMD_BACKLOGREQ, CMD_BACKLOGREQ_FWD,
	CMD_END};
struct CmdSimplified { // Cmd 保存用
	int type, cmd1, cmd2, cmd3, cmd4, argc;
	char* args;
	void Save(std::string& save);
	void Load(const char* save, char*& args_buffer);
	void copy(const CmdSimplified& from, char*& args_buffer);
};
class Cmd {
public:
	Cmdtype cmd_type;
	int cmd1, cmd2, cmd3, cmd4;
	int argc;
	int pos, scn;
	const char* rawdata;
	char cmdstr[1024];
	std::vector<VarInfo> args;

private:
	const Flags& flags;
	bool errorflag;
	int system_version;

	int GetArgs(const char*& d);
	int GetArgsSpecial(int normal_args,const char*& d);
	void GetSelection(const char*& d);
	int GetSwitch(const char*& d);
	int GetSimpleSwitch(const char*& d);
	int GetExpression(const char*& d, struct VarInfo* info = 0);
	int GetExpressionCond(const char*& d);
	int GetLeftToken(const char*& d, struct VarInfo& info);
	int GetString(const char*& d);
	int CopyString(const char* d);
	int StrVar(int type, int number);
	static char strtype[256];
	static int StrType(const char* d) { return strtype[*(unsigned const char*)d];}
public:
	const char* Str(const VarInfo& info) const {
		if (info.type != TYPE_STR && info.type != TYPE_VARSTR && info.type != TYPE_VARLOCSTR && info.type != TYPE_VARSYSSTR) return "";
		int pt = info.value;
		if (pt < 0 || pt >= STRHEAP_SIZE) return "";
		return strheap + pt;
	}
	int AddStr(char* s) {
		// 1-0a-0064 はこういうものが必要らしい
		int start = strend;
		while (*s) strheap[strend++] = *s++;
		strheap[strend++] = 0;
		return start;
	}
private:
	char strheap[STRHEAP_SIZE];
	int strend;
	void SetError(void) { errorflag = true;}
	void ResetString(void) {
		strend = 0;
	}
public:
	void GetCmd(Flags& f, const char*& d);
	void SetSysvar(int n, int v);
	void SetSysvar(int v) { SetSysvar(TYPE_SYS_SYS, v); }
	void SetFlagvar(VarInfo info, int v);
	void SetStrvar(VarInfo info, const std::string& s);
	bool IsError() { return errorflag;}
	void clear(void);
	Cmd(const Flags& f, int _sys_ver) : flags(f), system_version(_sys_ver) { cmd_type = CMD_NOP; argc = 0; errorflag = false; cmdstr[0] = 0; strend = 0; pos = -1;}
	void read(const CmdSimplified& cmd);
	void write(CmdSimplified& cmd, char*& args_buffer) const;
};

enum SkipMode {SKIP_NO=0, SKIP_TEXT=1, SKIP_GRP_FAST=16, SKIP_GRP_NOEFFEC=32, SKIP_GRP_NODRAW=64, SKIPEND_TEXT=256, SKIPEND_KEY=512,
	SKIP_IN_MENU=1024};

#include"font/text.h"

struct BacklogItem {
	enum {SaveSelect = -2};
	int scn, pos;
	int koe;
	std::string face;
	struct TextStream text;
	BacklogItem(void);
	void Clear(void);
	void AddTextPos(Cmd&);
	void DeleteTextPos();
	void SetSavepos(int pos);
	BacklogItem& operator =(const BacklogItem&);
};
class Text {
	class TextImpl* pimpl;
public:
	std::vector<BacklogItem> backlog;
	BacklogItem backlog_item;
	Text(Event::Container& _event, PicContainer& _parent, AyuSysConfig& config);
	~Text();
	void InitWindow(void);
	void Exec(Cmd& cmd);
	bool Wait(unsigned int current_time, Cmd& cmd);
	void SetSkipMode(SkipMode mode);
	void hide(void);
	void show(void);
	void show(int num);
	void Save(std::string& str, bool rollback_save);
	void Load(const char* str);
	void DrawBacklog(BacklogItem& item, Cmd& cmd);
};

#include"../window/rect.h"

class Grp {
	class GrpImpl* pimpl;
public:
	Grp(Event::Container& _event, PicContainer& _parent, const Flags& f, std::set<int>& _cgm_data, class MuSys& mu, AyuSysConfig& config);
	~Grp();
	bool Wait(unsigned int current_time, Cmd& cmd);
	void Exec(Cmd& cmd);
	void SetSkipMode(SkipMode mode);
	void InitSel(AyuSysConfig& config);
	void Save(std::string& str);
	void Load(const char* str);
	void SaveSys(std::string& str);
	void LoadSys(const char* str);
};

void dprintf(const char* fmt, ...); // debug 用
void eprintf(const char* fmt, ...); // コマンド実行(XXXexec)追跡用
#endif
