#ifndef __SCN2k_IMPL_H__
#define __SCN2k_IMPL_H__

#include"scn2k.h"
#include"window/widget.h"
#include"window/event.h"
#include<vector>
#include<string>
#include<map>
#include<set>

struct StackItem {
	int scn_number;
	int scn_pt;
	StackItem(int number, int pt) : scn_number(number), scn_pt(pt) {}
};

struct SaveTitle {
	virtual std::string operator()(int number) const = 0;
};

struct Scn2kMenu {
	Cmd cmd;
	class Scn2kMenuImpl* pimpl;
	enum MenuType { MENU_LOAD, MENU_SAVE, MENU_CONFIG, MENU_BACKLOG, MENU_BACKLOG2};
	MenuType type;
	enum MenuStatus { MENU_CONTINUE=0, MENU_CMD=1, MENU_DELETE=2};
	MenuStatus status;
	void InitPanel(Event::Container& event, PicContainer& parent);
	void InitTitle(const SaveTitle&);
	void Cancel(void);
	void Exec(Cmd& cmd);
	void activate(void);
	void deactivate(void);
	Scn2kMenu(MenuType type, class Scn2k& scn_impl, const Flags& flags, Text& text, int system_version);
	~Scn2kMenu();
};

struct Scn2kSaveTitle : SaveTitle {
	const class Scn2k& impl;
	std::string operator() (int number) const;
	Scn2kSaveTitle(const Scn2k& _impl) : impl(_impl) {}
};

class Scn2k : Event::Time {
	Event::Container& event;
	PicContainer& parent;
	AyuSysConfig& config;
	WidMouseCursor* mcursor;
	Flags flag;
	Text text_exec;
	Grp grp_exec;
	int system_version;
	SkipMode skip_mode;
	int scn_number;
	int scn_point;
	enum {SCN_INFO=999999, SCN_INFO_MENU=10000001, SCN_INFO_LOCALS = 10000100, SCN_INFO_LOCALSTR = 10000200, SCN_INFO_RETSTR = 10000300}; // stack に積まれる特殊な番号

	int save_scn, save_point;

	char* script_start;
	char* script_end;
	const char* script;

	int backlog_script_scn;
	char* backlog_script_start;
	char* backlog_script_end;

	std::string window_title;
	std::vector<StackItem> stack;
	std::vector<std::string> stack_strbuffer;
	std::vector<CmdSimplified> cmd_stack;
	std::vector<std::string> rollback_save;
	std::string new_rollback_save;
	std::map<int, std::set<int> > text_readflag;
	char* cmd_stack_str;
	char cmd_stack_str_orig[32768];

	Cmdtype dialog_type;
	class WidDialog* dialog;
	Scn2kMenu* menu;
	bool menu_mouseshown;

	Surface* mouse_surface;
	int mouse_type;
	int mouse_pressed;
	void ShowCursor();
	void HideCursor();

	void SetSkipMode(SkipMode mode);

	virtual void Elapsed(unsigned int current_time);

	// セーブ関連
	std::string MakeSaveFile(void) const;
	bool StatSaveFile(int num, int& year, int& month, int& day, int& wday, int& hour,int& min, int& sec, int& msec, std::string& title) const;
	void SaveImpl(std::string& s);
	void LoadImpl(const char* s);
	void Save(Cmd& cmd);
	void Load(Cmd& cmd);
	void SaveRollback(void);
	void LoadRollback(Cmd& cmd);
	void SaveSys(void);
	void LoadSys(void);
public:
	Scn2k(Event::Container& _event, PicContainer& _parent, class MuSys& mu, AyuSysConfig& config);
	~Scn2k();
	static char* OpenScript(int scn_number, char*& end, int* call_vec, int& system_version);
	bool ChangeScript(int scn_number, int call_no);
	bool ReadCmdAt(Cmd& cmd, int scn, int pt);
	void show_textwindow(int type);
	void hide_textwindow(void);
	void SysExec(Cmd& cmd);
	bool SysWait(Cmd& cmd);

	friend struct Scn2kSaveTitle;
};
#endif
