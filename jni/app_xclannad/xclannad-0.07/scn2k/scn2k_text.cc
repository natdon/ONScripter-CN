/*
TODO:
	日付のラベルが画面切り替え時に欠けるのを修正
	画像効果 : 人間の入れ換わりなど
	kcursor の操作を WidText クラスに任せる
	WidText クラスには新たに以下の操作を加える
		・ウェイト終了後、クリアなしに新たなテキストを追加、新たにstart-waitする
		・文字の描画 (Start) と Wait(カーソル表示待ち)の分離。
			Start すると文字を描画開始する。クリックで全描画。
			Flush するとバッファ内の文字をすべて描画する
			Wait すると全描画後、クリックされるまでカーソルを表示するまで待つ
		TextImpl 側の状態としては Wait のみを持つ (PREPAREに戻るのを待つ)
		ただし、Skip の権利はどっちがもつ？（現状は？）

	GrpObj: NextObj と GrpObj を分離。CreateObj は現状通り、Visible=1 時に行う。
		それぞれ num=0 (screen) の枝leaf として実装。delete時は親のdeleteのみを
		行い、子はGrpObjの実体だけを削除する
		Visible 後のhide は実際に hide とする
		ExecReservedCmd() はなくせるはず。Delete() もなくなる。
	カノギ：ReBlit() がうまくいかないせいで名前ウィンドウが消えた時の背景がなくなる

	くら：回想表示
	SEL画像効果
DONE:
	ともよのテキストウィンドウ実装、ボタン実装
	shake の画像効果
	オブジェクト内のテキスト色の実装
	画像効果の改善
*/

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

#include"window/event.h"
#include"window/picture.h"
#include"window/widget.h"
#include"system/file.h"
#include"system/system_config.h"
#include"scn2k.h"

#include<string>
using namespace std;

// kanji conv : デバッグ表示用
void kconv(const unsigned char* src, unsigned char* dest);
void kconv_rev(const unsigned char* src, unsigned char* dest);
string kconv(const string& s);
string kconv_rev(const string& s);
// render.cc
void DSurfaceFillA(Surface* src, const Rect& rect, int r, int g, int b, int a); // テキストウィンドウ背景の設定
void DSurfaceMove(Surface* src_o, const Rect& srcrect, Surface* dst_o, const Rect& dstrect); // コピー

/**************************************************************::
**
**	TextImpl(interface)
*/
struct TimerAtom {
	int from;
	int to;
	unsigned int start_time;
	unsigned int total_time;
};

struct TextWindow {
/* @@@ : SetWindowColor での surface 再設定に注意 */
	WidText* wid;
	bool name_visible;
	WidLabel* name;
	PicContainer* name_container;
	PicBase* face;
	PicBase* face_pics[8];
	TextWindow(PicContainer& parent, Event::Container& event, int window_no, const AyuSysConfig& config, void* callback);
	~TextWindow() {
		if (name_container) delete name_container;
		int i; for (i=0; i<8; i++) {
			if (face_pics[i]) delete face_pics[i];
		}
		if (wid) delete wid;
	}
	Rect WakuSize(PicContainer& pic, int waku_no, const AyuSysConfig& config);
	void MakeWaku(PicContainer& pic, Event::Container& event, int waku_no,int window_no, bool* use_btn, const AyuSysConfig& config, void* callback);
	void show(void) {
		wid->show();
		if (name_container && name_visible) name_container->show();
		if (face) face->show();
	}
	void hide(void) {
		wid->hide();
		if (name_container) name_container->hide();
		if (face) face->hide();
	}
	void ShowFace(const char* path) {
		if (!face) return;
		face->SetSurface( path, 0,0);
	}
	void ResetFace(void) {
		if (!face) return;
		face->SetSurface( (Surface*)0, 0,0);
	}
	void StartText(const TextStream& _stream) {
		wid->Clear();
		wid->stream = _stream;
		if (name_container) {
			char namestr[1024];
			namestr[0] = 0;
			wid->stream.RemoveName(namestr, 1024);
			if (namestr[0] == 0) {
				name_container->hide();
			} else {
				if (name) {
					name_container->show_all();
					name->SetText(namestr);
				}
			}
		}
		wid->Start();
	}
	void SetName(const char* n) {
		if (name_container && name) {
			if (n[0]) {
				name_container->show();
				name->SetText(n);
				name_visible = true;
			} else {
				name_container->hide();
				name_visible = false;
			}
		}
	}
};

class TextImpl {
private:
public:
	TextWindow* text;
	typedef enum {NORMAL=0, WAIT_TEXT=1, WAIT=2, WAIT_CLICK=3, WAIT_ABORT=4, WAIT_CLICK_MOUSEPOS = 5, WAIT_CLICK_MOUSEPOSEND_L = 6, WAIT_CLICK_MOUSEPOSEND_R = 7,
		WAIT_SELECT_INBOX = 10, WAIT_SELECT_OUTBOX=11, WAIT_SELECT_VALUE = 12,
		WAIT_EXTRN_MASK = 64, SAVEMASK = 128, LOADMASK = 256, SKIPMASK = 512,
		CLEARSCR_MASK = 1024, STATSAVE_MASK = 2048, CLEARSCR_WAIT_MASK=(1<<12),
		SKIPEND_MASK = (1<<13), BACKLOG_MASK=(1<<14), BACKLOG_MASK_FWD=(1<<15), BACKLOG_MASK_KOE=(1<<16), BACKLOG_WAIT_MASK=(1<<17),
		ALLMASK = (CLEARSCR_MASK | WAIT_EXTRN_MASK | SAVEMASK | LOADMASK | SKIPMASK | BACKLOG_MASK | BACKLOG_MASK_FWD | BACKLOG_MASK_KOE | BACKLOG_WAIT_MASK | STATSAVE_MASK | CLEARSCR_WAIT_MASK | SKIPEND_MASK)
	} Status;
	Status status, status_saved, status_mask;
private:
	std::string ruby_text;
	bool ruby_text_flag;
	unsigned int wait_time;
	unsigned int old_time;
	unsigned int base_time;
	int text_window_number;
	bool text_parsing;
	TextStream text_stream;
	SkipMode skip_mode;
	int save_selectcount;

	std::map<int, TimerAtom> timer_var;
	std::vector<WidTextButton*> selects;
	std::vector<int> sel_backlog_pos;
	string replace_name[26];
	string replace_name2[26];
	PicContainer* sel_widget;
	PicWidget* backlog_widget;

	vector<BacklogItem>& backlog;
	BacklogItem& backlog_item;
	BacklogItem cur_backlog_item;
	BacklogItem drawn_backlog_item;

public:
	PicContainer& parent;
	Event::Container& event;
	AyuSysConfig& config;
private:
	TextWindow* widgets[32];
	WidTimeCursor* kcursor;
	Surface* sel_bg1;
	Surface* sel_bg2;
	Rect sel_bg_rect;

	void SetCursor(int num);
	VarInfo wait_savedvar[2];

public:
	void AddText(const char* str);

	static void PressFuncSkip(void* pointer, WidButton* from);
	static void PressFuncLoad(void* pointer, WidButton* from);
	static void PressFuncSave(void* pointer, WidButton* from);
	static void PressFuncBacklog(void* pointer, WidButton* from);
	static void PressFuncBacklogFwd(void* pointer, WidButton* from);
private:
	static void PressFuncButton(void* pointer, WidButton* from);
	static bool PressFunc(int x, int y, void* pointer);

public:
	TextImpl(Event::Container& _event, PicContainer& _parent, AyuSysConfig& config, vector<BacklogItem>& parent_backlog, BacklogItem& parent_backlog_item);
	~TextImpl();
	void InitWindow(void);
	void SetWindowColor(int r, int g, int b, int a, bool is_transparent);
	void SetTextSpeed(int new_speed);
	void SetTextWait(int new_wait);
	void CreateSelect(Cmd& cmd);
	void Exec(Cmd& cmd);
	bool Wait(unsigned int current_time, Cmd& cmd);
	void hide(void);
	void show(void) { show(text_window_number); }
	void show(int num);
	void DrawBacklog(BacklogItem& item, Cmd& cmd);
	void Save(std::string& str, bool select_save);
	void Load(const char* str);
	void SetSkipMode(SkipMode _mode);
	void CreateSelBG(void);
};

/**************************************************************::
**
**	TextImpl(implementation)
*/
TextImpl::TextImpl(Event::Container& _event, PicContainer& _parent, AyuSysConfig& _config, vector<BacklogItem>& parent_backlog, BacklogItem& parent_backlog_item) :
	text(0),status(TextImpl::NORMAL), status_saved(TextImpl::NORMAL), status_mask(TextImpl::NORMAL), ruby_text_flag(false),
	old_time(0), base_time(0), text_window_number(0), text_parsing(false), skip_mode(SKIP_NO), save_selectcount(0), sel_widget(0),
	backlog_widget(0), backlog(parent_backlog), backlog_item(parent_backlog_item), parent(_parent), event(_event), config(_config),
	kcursor(0), sel_bg1(0), sel_bg2(0), sel_bg_rect(0,0,0,0) {
	int i;
	for (i=0; i<32; i++) {
		widgets[i] = 0;
	}
	text_stream.kanji_type = TextStream::sjis;
	event.RegisterGlobalPressFunc(&PressFunc, (void*)this);
}

TextImpl::~TextImpl() {
	if (sel_widget) delete sel_widget;
	int i;
	for (i=0; i<32; i++) {
		if (widgets[i]) delete widgets[i];
	}
	if (backlog_widget) delete backlog_widget;
	if (sel_bg1) parent.Root().DeleteSurface(sel_bg1);
	if (sel_bg2) parent.Root().DeleteSurface(sel_bg2);
	event.DeleteGlobalPressFunc(&PressFunc, (void*)this);
}

bool TextImpl::PressFunc(int x, int y, void* pointer) {
	TextImpl* t = (TextImpl*)pointer;
	if (t->status == WAIT_CLICK) {
		t->status = WAIT_ABORT;
	} else if (t->status == WAIT_CLICK_MOUSEPOS) {
		t->status = WAIT_CLICK_MOUSEPOSEND_L;
	} else if (t->status_mask & CLEARSCR_WAIT_MASK) {
		t->status_mask = Status(t->status_mask & (~CLEARSCR_WAIT_MASK));
		if (t->text) t->text->show();
		if (t->kcursor) {
			if (t->status == WAIT_TEXT) t->kcursor->show();
		}
		if (t->sel_widget) t->sel_widget->show();
		if (t->backlog_widget) t->backlog_widget->show();
	} else if (t->status_mask & BACKLOG_WAIT_MASK) {
		t->status_mask = Status(t->status_mask | BACKLOG_MASK_KOE);
	} else if ( (t->skip_mode & SKIP_TEXT) && (!(t->skip_mode & SKIP_IN_MENU)) ) {
		if (t->status == WAIT_SELECT_INBOX) ;
		else if (t->status == WAIT_SELECT_OUTBOX) ;
		else if (t->status == WAIT_SELECT_VALUE) ;
		else t->status_mask = Status(t->status_mask | SKIPEND_MASK);
	}
	return true; // event not deleted
}
void TextImpl::PressFuncButton(void* pointer, WidButton* from) {
	TextImpl* t = (TextImpl*)pointer;
	if (t->status != WAIT_SELECT_INBOX && t->status != WAIT_SELECT_OUTBOX) return;
	vector<WidTextButton*>::iterator it;
	int sel = 0;
	for (it=t->selects.begin(); it != t->selects.end(); it++, sel++) {
		if (from == *it) break;
	}
	if (it == t->selects.end()) {
		fprintf(stderr,"TextImpl::PressFuncButton: Cannot find select widget\n");
		return;
	}
	t->status = Status(WAIT_SELECT_VALUE + sel);
	return;
}

void TextImpl::SetSkipMode(SkipMode _mode) {
	if ( (skip_mode & SKIP_IN_MENU) && (_mode & SKIP_IN_MENU) == 0) {
		if (status_mask & BACKLOG_WAIT_MASK) { // backlog mode から復帰
			status_mask = Status(status_mask & (~(BACKLOG_MASK|BACKLOG_MASK_FWD|BACKLOG_MASK_KOE|BACKLOG_WAIT_MASK)));
			text->wid->Clear();
			if (status == WAIT_TEXT && text != 0) {
				text->StartText(text_stream);
				text->ShowFace(backlog_item.face.c_str());
				text->wid->Flush();
				if (kcursor) kcursor->show();
			}
			drawn_backlog_item.Clear();
		}
		if (text) text->wid->activate();
		if (sel_widget) {
			sel_widget->show();
			if (kcursor) kcursor->hide();
		}
		if (backlog_widget) backlog_widget->show();
		if (status_mask & STATSAVE_MASK) {
			status_mask = Status(status_mask & (~STATSAVE_MASK));
			status = status_saved;
		}
	} else if ( (skip_mode & SKIP_IN_MENU) == 0 && (_mode & SKIP_IN_MENU) ) {
		if (text) text->wid->deactivate();
		if (sel_widget) sel_widget->hide();
		if (backlog_widget) backlog_widget->hide();
	}
	skip_mode = _mode;
}

/* hash_map が欲しい……*/
#include<map>
#include<list>
struct SaveFaceHash { // バックログセーブ時の顔画像管理を行う
	map<string, int> facetonum;
	typedef pair<string,int> Node;
	typedef list<Node> List;
	List container;
	int id_max;
	static int size_max;
	SaveFaceHash() : id_max(0) {
	}
	void NewNode(string face, int face_id) {
		facetonum[face] = face_id;
		container.push_front(Node(face, face_id));
		if (container.size() > size_max) {
			Node remove = container.back();
			container.pop_back();
			facetonum.erase(remove.first);
		}
	}
	int Add(string face) {
		int id; int ret = -1;
		int i; List::iterator it;
		if (face.empty()) return -1;
		if (facetonum.find(face) == facetonum.end()) {
			id = ++id_max;
			NewNode(face, id);
			ret = -1;
		} else {
			id = facetonum[face];
			for (i=0, it=container.begin(); it != container.end(); i++, it++) {
				if (it->second == id) {
					ret = i;
					Node n = *it;
					container.erase(it);
					container.push_front(n);
					break;
				}
			}
		}
		return ret;
	}
	string Get(int num) {
		if (num < 0) return "";
		List::iterator it = container.begin();
		for (; it != container.end(); it++) {
			if (num == 0) return it->first;
			num--;
		}
		return "";
	}
};
int SaveFaceHash::size_max = 20;

void TextImpl::Save(string& str, bool rollback_save) {
	char buf[1024];
	str = "\n";
	str += "[TextImpl Window]\n";
	sprintf(buf, "TextImplWindow=%d\n",text_window_number);
	str += buf;
	if (rollback_save) {
		++save_selectcount;
		BacklogItem save_item;
		save_item.SetSavepos(save_selectcount);
		backlog.push_back(save_item);
	}
	sprintf(buf, "SaveSelectCount=%d\n",save_selectcount);

	str += buf;
	int i;
	for (i=0; i<26; i++) {
		if (replace_name2[i].empty()) continue;
		sprintf(buf, "RName.%c=%s\n",i+'A',replace_name2[i].c_str());
		str += buf;
	}
	int cnt = 0;
	vector<BacklogItem>::iterator it;
	it = backlog.begin();
	if (!rollback_save) {
		SaveFaceHash face_log;
		do {
		int cur_scn = -1; int cur_pos = -1;
		sprintf(buf, "Backlog.%d=",++cnt);
		str += buf;
		for (; it != backlog.end(); it++) {
			buf[0] = 0; int buflen = 0;
			if (it->scn == -1) continue;
			if (it->pos == -1 && it->scn != 0) continue;

			buf[buflen++] = ';';
			if (it->scn == 0 && it->pos == -1) {
				buflen += snprintf(buf+buflen, 1000-buflen, "\"%s\".", it->text.Save().c_str());
			} else {
				if (cur_scn != -1 && cur_scn != it->scn) break; // scn change
				if (cur_pos != -1 && cur_pos/5000 != it->pos/5000) break; // pos exceeded
				if (!it->text.container.empty()) {
					buflen += snprintf(buf+buflen, 1000-buflen, "\"%s\"", it->text.Save().c_str());
				}
				if (cur_scn == -1) { // scene change
					buflen += snprintf(buf+buflen, 1000-buflen, ":%d:%d",it->scn,it->pos);
					cur_scn = it->scn;
				} else {
					buflen += snprintf(buf+buflen, 1000-buflen, "%d",it->pos);
				}
				cur_pos = it->pos;
			}
			if (it->koe != -1)
				buflen += snprintf(buf+buflen, 1000-buflen, ",%d",it->koe);
			if (!it->face.empty()) {
				if (it->koe == -1) buf[buflen++] = ',';
				int face_num = face_log.Add(it->face);
				if (face_num >= 0 && face_num < 20)
					buflen += snprintf(buf+buflen, 1000-buflen, ",%c", 'A'+face_num);
				else
					buflen += snprintf(buf+buflen, 1000-buflen, ",\"%s\"", it->face.c_str());
			}
			buf[buflen++] = '\0';
			if (buflen >= 1000) { // 万が一、バックログ１アイテムの大きさが 1000byte を越えるとき
				fprintf(stderr,"Fatal : Cannot save backlog crrectly; Please send bug report to the author.\n");
			} else str += buf;
		}
		str += "\n";
	} while(it != backlog.end());
	}
	return;
}
void TextImpl::Load(const char* str) {
	if (text) text->wid->Clear();
	hide();
	text_window_number = 0;
	save_selectcount = 0;
	if (sel_widget) {
		selects.clear();
		sel_backlog_pos.clear();
		delete sel_widget;
		sel_widget = 0;
	}
	if (backlog_widget) {
		delete backlog_widget;
		backlog_widget = 0;
	}
	status = NORMAL;
	status_mask = NORMAL;
	status_saved = NORMAL;
	text_parsing = false;
	text_stream.Clear();
	// backlog.clear();
	vector<BacklogItem> new_backlog;
	backlog_item.Clear();
	cur_backlog_item.Clear();
	drawn_backlog_item.Clear();

	str = strstr(str, "\n[TextImpl Window]\n");

	if (str) {
		SaveFaceHash face_log;
		str += strlen("\n[TextImpl Window]\n");
		const char* strend = str;
		do {
			str = strend;

			strend = strchr(str, '\n');
			if (strend == 0) strend = str + strlen(str);
			else strend++;

			if (str[0] == '[') break; // next section
			if (strncmp(str, "TextImplWindow=",15) == 0) {
				str += 15;
				sscanf(str, "%d", &text_window_number);
			} else if (strncmp(str, "SaveSelectCount=",16) == 0) {
				str += 16;
				sscanf(str, "%d", &save_selectcount);
			} else if (strncmp(str, "RName.", 6) == 0) {
				int n = str[6]-'A';
				if (n >= 0 && n < 26 && str[7] == '=') {
					const char* s = strchr(str, '\n');
					int len = -1;
					if (s) len = s-(str+8);
					if (len > 0) {
						replace_name2[n].assign(str+8, len);
					}
				}
			} else if (strncmp(str, "Backlog.", 8) == 0) {
				int cur_scn = -1;
				int n = -1;
				sscanf(str+8, "%d", &n); /* not used */
				const char* next_str = strchr(str, ';');
				while(next_str != 0 && next_str < strend) {
					str = next_str + 1;
					next_str = strchr(str, ';');
					if (next_str == 0) next_str = strend;

					BacklogItem item;
					if (str[0] == '"') {
						const char* send = strchr(str+1, '"');
						if (send == 0 || send > next_str) continue;
						string tmp_str; tmp_str.assign(str+1, send-str-1);
						item.DeleteTextPos();
						item.text.Load(tmp_str);
						str = send + 1;
					}
					if (str[0] == '.') {
						item.DeleteTextPos();
						str++;
					} else if (str[0] == ':') {
						sscanf(str, ":%d:%d", &item.scn, &item.pos);
						cur_scn = item.scn;
						
					} else {
						item.scn = cur_scn;
						sscanf(str, "%d", &item.pos);
					}
					str = strchr(str, ',');
					if (str == 0 || str > next_str) goto backlog_store;
					str++;
					if (str[0] == ';' || str[0] == ',')
						item.koe = -1;
					else
						sscanf(str, "%d", &item.koe);
					str = strchr(str, ',');
					if (str == 0 || str > next_str) goto backlog_store;
					str++;
					if (*str == '"') {
						const char* send = strchr(str+1, '"');
						if (send) {
							item.face.assign(str+1, send-str-1);
						}
					} else if (*str >= 'A' && *str <= 'Z') {
						item.face = face_log.Get(*str - 'A');
					}
					face_log.Add(item.face);
				backlog_store:
					new_backlog.push_back(item);
				}
			}
		} while (*strend != 0);
	}
	if (new_backlog.empty() && (!backlog.empty())) { // empty なら save_selectcount まで backlog を巻き戻す
		vector<BacklogItem>::iterator it = backlog.end();
		do {
			it--;
			if (it->scn == BacklogItem::SaveSelect && it->pos == save_selectcount) {
				// Save 位置を見つけたらそれ以降を erase
				backlog.erase(it, backlog.end());
				break;
			}
		} while(it != backlog.begin());
		--save_selectcount;
	} else {
		backlog.swap(new_backlog);
	}
	// backlog.clear();
	return;
}

void TextImpl::hide(void) {
	if (text) text->hide();
	if (kcursor) kcursor->hide();
	text = 0;
}
void TextImpl::show(int num) {
	if (num != text_window_number) {
		hide();
		if (num >= 0 && num < 32 && widgets[num] != 0) {
			text_window_number = num;
		}
	}
	text = widgets[text_window_number];
	text->show();
	if (kcursor) {
		int kx, ky, d;
		char key[1024];
		sprintf(key, "#WINDOW.%03d.KEYCUR_MOD", text_window_number);
		config.GetParam(key, 3, &d, &kx, &ky);
		// 正しくない気がする
		kx += text->wid->Pic()->PosX();
		ky += text->wid->Pic()->PosY();
		// 微妙に下にする
		ky += 8;
		kcursor->Pic()->Move(kx, ky);
	}
}
void TextImpl::DrawBacklog(BacklogItem& item, Cmd& cmd) {
	show();
	text->wid->deactivate();
	status_mask = Status(status_mask | BACKLOG_WAIT_MASK);
	drawn_backlog_item = item;
	if (item.text.container.empty()) {
		// cmd から text 内容を再構成
		TextStream saved_text = text_stream;
		text_stream.Clear();
		AddText(cmd.Str(cmd.args[0]));
		item.text = text_stream;
		text_stream = saved_text;
	}
 	item.text.InsertColor(0, item.text.container.size(), 0xff,0xff,0);
	text->StartText(item.text);
	text->wid->Flush();
	if (item.face.empty()) text->ResetFace();
	else text->ShowFace(item.face.c_str());
	if (kcursor) kcursor->hide();
}

void TextImpl::CreateSelBG(void) {
	if (sel_bg1 != 0 || sel_bg2 != 0) return;

	const char* btnfile1 = config.GetParaStr("#SELBTN.000.NAME");
	const char* btnfile2 = config.GetParaStr("#SELBTN.000.BACK");
	char path[1024];
	strcpy(path, btnfile1);
	sel_bg1 = parent.Root().NewSurface(path);
	if (sel_bg1 == 0) {
		sprintf(path,"%s.g00",btnfile1);
		sel_bg1 = parent.Root().NewSurface(path);
	}
	strcpy(path, btnfile2);
	sel_bg2 = parent.Root().NewSurface(path);
	if (sel_bg2 == 0) {
		sprintf(path,"%s.g00",btnfile2);
		sel_bg2 = parent.Root().NewSurface(path);
	}
	sel_bg_rect = Rect(0,0,0,0);
	if (sel_bg1) sel_bg_rect.join(Rect(*sel_bg1));
	if (sel_bg2) sel_bg_rect.join(Rect(*sel_bg2));
	return;
}

void TextImpl::CreateSelect(Cmd& cmd) {
	char key[1024];
	sprintf(key, "#WINDOW.%03d.SELCOM_USE",text_window_number);
	int sel_type = 0;
	config.GetParam(key, 1, &sel_type);

	int sel_size = cmd.args.size() / 2;
	int i;
	// cur_backlog_item に次にbacklogに入るべき内容を作成
	// CreateSelect() 後、SAVEPOINT なので現在のbacklogの内容(前のメッセージ)が
	// backlog に代入される。その後、backlog_item に cur_backlog_item の内容がセットされる(Wait()内)
	char backlog_sel_text[11] = {0x81,0x69,0x91,0x49,0x91,0xf0,0x8e,0x88,0x81,0x6a,0x00};
	cur_backlog_item.Clear();
	cur_backlog_item.AddTextPos(cmd);
	cur_backlog_item.text.Add(backlog_sel_text);
	cur_backlog_item.text.AddReturn();
	sel_backlog_pos.clear();
	for (i=0; i<sel_size; i++) {
		sel_backlog_pos.push_back(cur_backlog_item.text.container.size());
		cur_backlog_item.text.Add(cmd.Str(cmd.args[i*2]));
		cur_backlog_item.text.AddReturn();
	}
	sel_backlog_pos.push_back(cur_backlog_item.text.container.size());

	if (sel_type == 0) { // Princess Bride: 選択ウィンドウを別表示
External_select:
		CreateSelBG();
		hide(); // なので、テキストウィンドウは消去
		int baseposx, baseposy, repposx, repposy;
		int mojisize, col1, col2;
		config.GetParam("#SELBTN.000.BASEPOS", 2, &baseposx, &baseposy);
		config.GetParam("#SELBTN.000.REPPOS", 2, &repposx, &repposy);
		config.GetParam("#SELBTN.000.MOJISIZE", 1, &mojisize);
		config.GetParam("#SELBTN.000.MOJIDEFAULTCOL", 1, &col1);
		config.GetParam("#SELBTN.000.MOJISELECTCOL", 1, &col2);
		if (col1 == col2) col2 = 1; // CLANNAD でとりあえず。
		int r, g, b;
		sprintf(key, "#COLOR_TABLE.%03d", col1);
		config.GetParam(key, 3, &r, &g, &b);
		Color fore(r,g,b);
		sprintf(key, "#COLOR_TABLE.%03d", col2);
		config.GetParam(key, 3, &r, &g, &b);
		Color seled(r,g,b);

		/* ウィジット作成 */
		/* ウィンドウ背景の大きさを求める */
		if (baseposx == 0 && sel_bg_rect.width() != 0)
			baseposx = (parent.Width()-sel_bg_rect.width()) / 2; // ボタン位置をセンタリング

		sel_widget = parent.create_node( Rect(0, 0, parent.Width(), parent.Height()),0);

		for (i=0; i<sel_size; i++) {
			PicBase* p;
			// 背景作成
			if (sel_bg2) {
				p = sel_widget->create_node(Rect(baseposx, baseposy, baseposx+sel_bg_rect.width(), baseposy+sel_bg_rect.height()),0);
				p->SetSurface(sel_bg2, 0, 0);
			}
			if (sel_bg1) {
				p = sel_widget->create_node(Rect(baseposx, baseposy, baseposx+sel_bg_rect.width(), baseposy+sel_bg_rect.height()),0);
				p->SetSurface(sel_bg1, 0, 0);
			}
			/* ボタン作成 */
			const char* str = cmd.Str(cmd.args[i*2]);
			int value = cmd.args[i*2+1].value;
			while(selects.size() <= value) selects.push_back(0); // vector の大きさを広げる

			kconv( (const unsigned char*)str, (unsigned char*)key);
			selects[value] = new WidTextButton(event, sel_widget, key, mojisize, WidTextButton::CENTER,
				Rect(baseposx, baseposy, baseposx+sel_bg_rect.width(), baseposy+sel_bg_rect.height()), 1, fore, seled, Color(0,0,0,0));
			selects[value]->press_func = &PressFuncButton;
			selects[value]->press_pointer = (void*)this;

			baseposx += repposx;
			baseposy += repposy;
		}
		sel_widget->show_all();
		status = WAIT_SELECT_OUTBOX;
	} else { // CLANNAD: テキストウィンドウ内に選択肢表示
		int mojisize;
		config.GetParam("#SELBTN.000.MOJISIZE", 1, &mojisize);
		Color fore(0xff,0xff,0xff);
		Color seled(0xff,0xff,0xff);

		show();
		if (text == 0) goto External_select; // テキスト・ウィンドウを表示できなければ外部選択肢にする
		text->wid->Clear();
		if (kcursor) kcursor->hide();
		/* ウィジット作成  : テキスト表示範囲と同じ*/
		int posx = text->wid->pictext->PosX();
		int posy = text->wid->pictext->PosY();
		int sel_w = text->wid->pictext->Width();
		int sel_h = text->wid->pictext->Height();
		sel_widget = text->wid->PicNode()->create_node(Rect(posx, posy, posx+sel_w, posy+sel_h), 0);

		int sel_y = 0;
		for (i=0; i<sel_size; i++) {
			PicBase* p;
			/* ボタン作成 */
			const char* str = cmd.Str(cmd.args[i*2]);
			int value = cmd.args[i*2+1].value;
			while(selects.size() <= value) selects.push_back(0); // vector の大きさを広げる

			kconv( (const unsigned char*)str, (unsigned char*)key);
			selects[value] = new WidTextButton(event, sel_widget, key, mojisize, WidTextButton::Attribute(WidTextButton::REVERSE | WidTextButton::NOPADDING),
				Rect(0, sel_y, sel_w, sel_y), 1, fore, seled, Color(0,0,0,0));
			selects[value]->press_func = &PressFuncButton;
			selects[value]->press_pointer = (void*)this;

			sel_y += selects[value]->Pic()->Height() + 1;
		}
		sel_widget->show_all();
		status = WAIT_SELECT_INBOX;
	}
}

void TextImpl::AddText(const char* str_o) {
	char str[10001];
	if (text == 0) return;
	/* まず、replace string を変換 */
	int i;
	int cnt = 0;
	/* ＊ = 81 96 A-Z = 0x82 [0x60-0x79] */
	/* ％ = 81 93 A-Z = 0x82 [0x60-0x79] */
	for (i=0; cnt<10000 && str_o[i] != 0; i++) {
		if (str_o[i] < 0) {
			if ( (unsigned char)str_o[i] == 0x81 && (unsigned char)str_o[i+1] == 0x96 && (unsigned char)str_o[i+2] == 0x82) {
				int c = str_o[i+3];
				if (c >= 0x60 && c <= 0x79 && replace_name[c-0x60].length() != 0) { // 名前変換
					i += 3;
					strcpy(str+cnt, replace_name[c-0x60].c_str());
					cnt += replace_name[c-0x60].length();
					continue;
				}
			} else if ( (unsigned char)str_o[i] == 0x81 && (unsigned char)str_o[i+1] == 0x93 && (unsigned char)str_o[i+2] == 0x82) {
				int c = str_o[i+3];
				if (c >= 0x60 && c <= 0x79 && replace_name2[c-0x60].length() != 0) { // 名前変換２
					i += 3;
					strcpy(str+cnt, replace_name2[c-0x60].c_str());
					cnt += replace_name2[c-0x60].length();
					continue;
				}
			}
			str[cnt++] = str_o[i++];
		}
		str[cnt++] = str_o[i];
	}
	str[cnt] = 0;
	str[10000] = 0;
	char* str_top = str;

	for (char* s = str_top; *s != 0; s++) {
		// if (*(unsigned char*)s == 0xa1 && *(unsigned char*)(s+1) == 0xda) { /* euc */
		if (*(unsigned char*)s == 0x81 && *(unsigned char*)(s+1) == 0x79) { /* sjis */
			// 名前
			*s = 0;
			if (s != str_top) text_stream.Add(str_top);
			s += 2;
			char* name_top = s;
			for (; *s != 0; s++) {
				// if (*(unsigned char*)s == 0xa1 && *(unsigned char*)(s+1) == 0xdb) { /* euc */
				if (*(unsigned char*)s == 0x81 && *(unsigned char*)(s+1) == 0x7a) { /* sjis */
					*s = 0;
					s += 2;
					text_stream.AddName(name_top);
					break;
				}
				if (*s < 0 && s[1] != 0) s++; // 全角文字なら２字飛ばす
			}
			str_top = s;
		}
		if (*s == 0x0a) {
			*s = 0;
			text_stream.Add(str_top);
			text_stream.AddReturn();
			str_top = s;
		} else if (*s < 0 && s[1] != 0) s++;
	}
	text_stream.Add(str_top);
	return;
}

void TextImpl::Exec(Cmd& cmd) {
	if (cmd.cmd_type == CMD_TEXT) {
		if (text == 0) {
			show();
		}
		if (cmd.args.size() != 1) return;
		if (ruby_text_flag) {
			ruby_text = cmd.Str(cmd.args[0]);
			ruby_text_flag = 0;
			cmd.clear();
			return;
		}
		cur_backlog_item.AddTextPos(cmd);
		AddText(cmd.Str(cmd.args[0]));
		char debug[1024];
		kconv( (unsigned char*)cmd.Str(cmd.args[0]), (unsigned char*)debug);
		eprintf("text: %s\n",debug);
		if (text_parsing)
			cmd.clear();
		else
			cmd.cmd_type = CMD_SAVEPOINT;
		text_parsing = true; /* テキスト待ち直後のテキスト位置＝セーブ位置 */
		return;
	}
	if (cmd.cmd_type != CMD_OTHER) return;
	/* テキストウィンドウを消去するコマンド類をチェックする */
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x21) {
		if (cmd.cmd3 == 0x49 || cmd.cmd3 == 0x4b || cmd.cmd3 == 0x4c) {
			if (text) text->ResetFace();
			cur_backlog_item.face = "";
			hide();
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x17 && cmd.cmd3 == 0 && cmd.cmd4 == 1) { 
		// PlayKoe ; 声出力コマンドをチェックする */
		cur_backlog_item.koe = cmd.args[0].value;
	}
	if (cmd.cmd1 == 0 && cmd.cmd2 == 3 && cmd.cmd3 == 0x97) { // いいのかなー
		
		if (text) {
			text->ResetFace();
			text->wid->Clear();
		}
		cur_backlog_item.face = "";
		text_stream.Clear();
		hide();
	}
	if (cmd.cmd1 == 0 && cmd.cmd2 == 3) {
		if (cmd.cmd3 == 0x11) { // テキスト表示、クリック待ち
			if (text) {
				eprintf("start\n");
				text->StartText(text_stream);
				if (skip_mode & SKIP_TEXT) text->wid->Flush();
				else if (kcursor) kcursor->show();
				status = WAIT_TEXT;
				text_parsing = false;
			}
			backlog_item = cur_backlog_item;
			if (cur_backlog_item.scn == 0 && cur_backlog_item.pos == -1) backlog_item.text = text_stream;
			cur_backlog_item.Clear();

			cmd.clear();
			cmd.cmd_type = CMD_WAITFRAMEUPDATE; // 画像描画に戻る(skip時にテキストが描画されやすくするため)
				// これだと１フレーム１テキストしか表示されなくなるので注意
		} else if (cmd.cmd3 == 3 || cmd.cmd3 == 0xc9) { // リターン挿入
			text_stream.AddReturn();
			cur_backlog_item.DeleteTextPos();
			cmd.clear();
		} else if (cmd.cmd3 == 0x3e8 || cmd.cmd3 == 0x3e9) { // 顔グラフィック変更
			if (text == 0) {
				show();
			}
			if (cmd.cmd3 == 0x3e8) {
				string s = cmd.Str(cmd.args[0]);
				s += ".g00";
				if (text) text->ShowFace(s.c_str());
				cur_backlog_item.face = s;
				cmd.cmd_type = CMD_SAVECMD_ONCE;
			} else if (cmd.cmd3 == 0x3e9) { // 顔グラフィック消去
				if (text) text->ResetFace();
				cur_backlog_item.face = "";
				cmd.cmd_type = CMD_SAVECMD_ONCE;
			}
		} else if (cmd.cmd3 == 0x78) { // ルビ関連
			if (text == 0) {
				show();
			}
			if (cmd.cmd4 == 1) {
				ruby_text_flag = true;
				eprintf("SetRubyTextImpl.");
				cmd.clear();
			} else if (cmd.cmd4 == 0) {
				if (ruby_text.length() == 0) { // ルビを振るテキストがない
					eprintf("Cannot find ruby text.\n");
					return;
				}
				if (cmd.args.size() != 1) return;
				char debug1[1024], debug2[1024];
				kconv( (unsigned char*)ruby_text.c_str(), (unsigned char*)debug1);
				kconv( (unsigned char*)cmd.Str(cmd.args[0]), (unsigned char*)debug2);
				eprintf("SetRuby. %s, %s",debug1, debug2);
				text_stream.AddRuby(ruby_text.c_str(), cmd.Str(cmd.args[0]));
				cur_backlog_item.DeleteTextPos();
				cmd.clear();
			}
		} else if (cmd.cmd3 == 0x66) { // テキストウィンドウの形
			if (cmd.cmd4 == 0) {
				eprintf("set text window <- %d\n",cmd.args[0].value);
				if (text) show(cmd.args[0].value);
				else text_window_number = cmd.args[0].value;
			} else if (cmd.cmd4 == 1) { // default value
				eprintf("set text window <- default\n");
				if (text) show(0);
				else text_window_number = 0;
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x67) { // テキストウィンドウ表示？
			show();
// 表示の際はテキストをクリアしない？
//			if (text) text->wid->Clear();
//			text_stream.Clear();
			cmd.clear();
		} else if (cmd.cmd3 == 0x68) { // テキスト表示？
			// 全テキスト表示
			if (text) {
				text->StartText(text_stream);
				text->wid->Flush();
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x98) { // テキストウィンドウクリア？
			show();
			if (text) text->wid->Clear();
			text_stream.Clear();
			cmd.clear();
		}
	} else if (cmd.cmd1 == 0 && cmd.cmd2 == 2 && (cmd.cmd3 == 1 || cmd.cmd3 == 3) && cmd.cmd4 == 0) {
		// 選択肢
		CreateSelect(cmd);
		cmd.cmd_type = CMD_ROLLBACKPOINT; /* 選択肢はセーブ位置 / シナリオ巻き戻し位置 */
		// cmd.clear();
	} else if (cmd.cmd1 == 0 && cmd.cmd2 == 4) {
		if (cmd.cmd3 == 0x44c) { // テキストスキップ開始
			status_mask = Status(SKIPMASK | status_mask);
			cmd.clear();
		} else if (cmd.cmd3 == 0x3e8) { // ウィンドウ消去
			status_mask = Status(CLEARSCR_MASK | status_mask);
			cmd.clear();
		}
	} else if (cmd.cmd1 == 1 && cmd.cmd2 == 0x04) {
		/* ウェイト関連命令 */
		if (cmd.cmd3 == 0x64 || cmd.cmd3 == 0x6f || cmd.cmd3 == 0x79) {
			eprintf("wait %dmsec\n",cmd.args[0].value);
			if (cmd.cmd3 == 0x64 && text) {
				/* 0x64 だと文字描画中の待ちに使うことがある */
				text->StartText(text_stream);
				text->wid->Flush();
			}
			if (cmd.cmd3 == 0x6f || cmd.cmd3 == 0x79) wait_time = base_time + cmd.args[0].value;
			else wait_time = old_time + cmd.args[0].value;
			status = WAIT;
			cmd.cmd_type = CMD_WAITFRAMEUPDATE; // 画像描画に戻る(skip時にテキストが描画されやすくするため)
		} else if (cmd.cmd3 == 0x65 || cmd.cmd3 == 0x70) {
			eprintf("wait %dmsec(click stop)\n",cmd.args[0].value);
			if (cmd.cmd3 == 0x70) wait_time = base_time + cmd.args[0].value;
			else wait_time = old_time + cmd.args[0].value;
			status = WAIT_CLICK;
			cmd.cmd_type = CMD_WAITFRAMEUPDATE; // 画像描画に戻る(skip時にテキストが描画されやすくするため)
		} else if (cmd.cmd3 == 0x83) {
			/* マウスがクリックされるまで待つ */
			eprintf("wait and get mouse pos at click\n");
			wait_time = old_time + 1000 * 1000;
			status = WAIT_CLICK_MOUSEPOS;
			wait_savedvar[0] = cmd.args[0];
			wait_savedvar[1] = cmd.args[1];
			cmd.clear();
		} else if (cmd.cmd3 == 0x1fe) {
			eprintf("get timer value[%d]\n",cmd.args[0].value);
			if (timer_var.find(cmd.args[0].value) == timer_var.end()) {
				cmd.SetSysvar(0);
			} else {
				TimerAtom& atom = timer_var[cmd.args[0].value];
				if (atom.total_time <= 0) atom.total_time = 1;
				int cur_tm = old_time - atom.start_time;
				if (cur_tm < 0) cur_tm = atom.total_time; // エラーなら最終時間に合わせる
				if (cur_tm > atom.total_time) cur_tm = atom.total_time;
				// use 'long long'(64bit) or 'double'(80bit) type, since total_time, to and from is 32 bit.
				int v = atom.from + (long long)(atom.to-atom.from)*cur_tm/int(atom.total_time);
				cmd.SetSysvar(v);
			}
/* From rldev-1.40, reallive.kfn
0x72  fun Timer           (store) <1:Sys:00114, 1> ('counter') ()
0x73  fun CmpTimer        (store) <1:Sys:00115, 1> ('time')    ('time', 'counter')
0x74  fun SetTimer                <1:Sys:00116, 1> ('time')    ('time', 'counter')

0x78  fun ResetExTimer            <1:Sys:00120, 1> ('counter') ()
0x79  fun timeEx                  <1:Sys:00121, 1> ('time')    ('time', 'counter')
0x7a  fun timeExC         (store) <1:Sys:00122, 1> ('time')    ('time', 'counter')
0x7b  fun timeExC2        (store) <1:Sys:00123, 1> ('time')    ('time', 'counter') // UNDOCUMENTED
0x7c  fun ExTimer         (store) <1:Sys:00124, 1> ('counter') ()
0x7d  fun CmpExTimer      (store) <1:Sys:00125, 1> ('time')    ('time', 'counter')
0x7e  fun SetExTimer              <1:Sys:00126, 1> ('time')    ('time', 'counter')
*/

		} else if (cmd.cmd3 == 0x6e || cmd.cmd3 == 0x78) { // set basetime
			if (cmd.cmd4 == 1) {
				eprintf("set basetime\n");
				base_time = old_time;
				cmd.clear();
			} else if (cmd.cmd4 == 0) { // n-th base time
				int index = cmd.args[0].value;
				eprintf("set basetime (%d)\n",index);
				TimerAtom& atom = timer_var[index];
				atom.from = 0;
				atom.to = 0;
				atom.total_time = 0;
				atom.start_time = old_time;
				cmd.clear();
			}
		} else if (cmd.cmd3 == 0x72 || cmd.cmd3 == 0x7c) { // get time
			if (cmd.cmd4 == 1) { // get time
				eprintf("get time\n");
				cmd.SetSysvar(old_time - base_time);
			} else if (cmd.cmd4 == 0) { // n-th get time
				int index = cmd.args[0].value;
				eprintf("get time %dth\n",index);
				if (timer_var.find(index) == timer_var.end()) cmd.SetSysvar(0);
				else cmd.SetSysvar(old_time - timer_var[index].start_time);
			}
		} else if (cmd.cmd3 == 0x26c || cmd.cmd3 == 0x270) { // set basetime(multi)
			int j = 0;
			eprintf("set basetime\n");
			int i; for (i=0; i<cmd.argc; i++) {
				int cnt = cmd.args[j++].value; // ３なので無視
				int num = cmd.args[j++].value;
				TimerAtom& atom = timer_var[num];
				atom.from = cmd.args[j++].value;
				atom.to = cmd.args[j++].value;
				atom.total_time = cmd.args[j++].value;
				atom.start_time = old_time;
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x276) { // get time (multi)
			int j = 0;
			eprintf("get timer value\n");
			vector<VarInfo> args = cmd.args;
			vector<VarInfo>::iterator it = args.begin();
			int argc = cmd.argc;
			int active_timers = 0;
			int i; for (i=0; i<argc; i++) {
				int cnt = (it++)->value;
				int num = (it++)->value;
				
				if (timer_var.find(num) == timer_var.end()) {
					cmd.SetFlagvar(*it++, 0);
				} else {
					TimerAtom& atom = timer_var[num];
					if (atom.total_time <= 0) atom.total_time = 1;
					int cur_tm = old_time - atom.start_time;
					if (cur_tm < 0) cur_tm = atom.total_time; // エラーなら最終時間に合わせる
					if (cur_tm > atom.total_time) cur_tm = atom.total_time;
					// use 'long long'(64bit) or 'double'(80bit) type, since total_time, to and from is 32 bit.
					int v = atom.from + (long long)(atom.to-atom.from)*cur_tm/int(atom.total_time);
					cmd.SetFlagvar(*it++, v);
					if (atom.total_time != -1 && cur_tm < atom.total_time) active_timers++;
				}
			}
			if (active_timers) active_timers = 1;
			cmd.SetSysvar(active_timers);
		} else if (cmd.cmd3 == 0x1f4) {
			TimerAtom& atom = timer_var[cmd.args[0].value];
			atom.from = cmd.args[1].value;
			atom.to = cmd.args[2].value;
			atom.total_time = cmd.args[3].value;
			atom.start_time = old_time;
			cmd.clear();
		} else if (cmd.cmd3 == 0x3e8) {
			/* rand() */
			int min = 0, max;
			if (cmd.args.size() == 2) {
				min = cmd.args[0].value;
				max = cmd.args[1].value;
			} else {
				max = cmd.args[1].value;
			}
			if (min > max) {
				int tmp = max;
				max = min;
				min = tmp;
			}
			int r = random();
			if (min == max) r = min;
			else r = (r % (max-min)) + min;
			cmd.SetSysvar(r);
		} else if (cmd.cmd3 == 0x3ea) {
			int val = cmd.args[0].value;
			if (val < 0) val = -val;
			cmd.SetSysvar(val);
		} else if (cmd.cmd3 == 0x3ec) {
			/* min だよなあ・・・*/
			int min = cmd.args[0].value;
			int max = cmd.args[1].value;
			if (max < min) min = max;
			cmd.SetSysvar(min);
		} else if (cmd.cmd3 == 0x3ef) {
			/* min */
			int min = cmd.args[0].value;
			int max = cmd.args[1].value;
			if (max < min) min = max;
			cmd.SetSysvar(min);
		} else if (cmd.cmd3 == 0x320) {
			/* range conversion : 比率に丸める */
			// アルゴリズムは間違えてるような気がする
			// 
			if (cmd.args.size() >= 7) {
				int val = cmd.args[0].value;
				int offset = cmd.args[1].value;
				int r_min = cmd.args[2].value;
				int v_min = cmd.args[3].value;
				int v_max = cmd.args[4].value;
				int r_max = cmd.args[5].value;
				int mode = cmd.args[6].value;
				// rldev : mode == 1,3 : 'acceralating curve', 2,3: 'decelerating curve'
				// 複数の引数リスト(r_minからmodeまでのリスト)もつこともあり、その場合は
				// "cancel out in some way" らしい
				if (mode == 1 || mode == 3) val += offset;
				else if (mode == 2 || mode == 4) val -= offset;
if (cmd.args.size() != 7)
	fprintf(stderr,"\n%d/%d: cmd 01-04:0320 : XXXX NOT SUPPORTED LIST : DOUBLE RANGE CONVERSION!   XXXXXXXXXXX\n",cmd.scn,cmd.pos);
				if (val < v_min) val = v_min;
				if (val > v_max) val = v_max;
				val = (r_max-r_min)*(val-v_min)/(v_max-v_min) + r_min;
				cmd.SetSysvar(val);
			}
		} else if (cmd.cmd3 == 0x3f1) {
			/* range 内に丸める */
			int min = cmd.args[0].value;
			int val = cmd.args[1].value;
			int max = cmd.args[2].value;
			if (min > max) {
				int tmp = max;
				max = min;
				min = tmp;
			}
			if (val < min) val = min;
			if (val > max) val = max;
			cmd.SetSysvar(val);
		} else if (cmd.cmd3 == 0x16c && cmd.cmd4 == 0) {
		/* なんかよくわからないけどカーソル形状変更にしとく */
			SetCursor(cmd.args[0].value);
			cmd.clear();
		} else if (cmd.cmd3 == 0x0bc1) { // メニューからのロード
			cmd.cmd_type = CMD_LOADREQ;
		} else if ( (cmd.cmd3 >= 0x8d4 && cmd.cmd3 <= 0x8d8) || cmd.cmd3 == 0x8db || cmd.cmd3 == 0x93f || cmd.cmd3 == 0xa39) {
			// テキストウィンドウの色設定
			int r, g, b, a, flag;
			if (cmd.cmd3 == 0xa39) { // 元設定を取り出す
				config.GetOriginalParam("#WINDOW_ATTR", 5, &r, &g, &b, &a, &flag);
			} else {
				config.GetParam("#WINDOW_ATTR", 5, &r, &g, &b, &a, &flag);
			}
			if (cmd.cmd3 == 0xa39 || cmd.cmd3 == 0x93f) { // 設定を変数に取り出す
				if (cmd.args.size() != 5) {
					fprintf(stderr,"cmd 01-04:%4d : invalid arg size\n", cmd.cmd3);
				} else {
					vector<VarInfo> args(cmd.args);
					cmd.SetFlagvar(args[0], r);
					cmd.SetFlagvar(args[1], g);
					cmd.SetFlagvar(args[2], b);
					cmd.SetFlagvar(args[3], a);
					cmd.SetFlagvar(args[4], flag);
				}
			} else {
				switch(cmd.cmd3) {
				case 0x8d4: r = cmd.args[0].value; break;
				case 0x8d5: g = cmd.args[0].value; break;
				case 0x8d6: b = cmd.args[0].value; break;
				case 0x8d7: a = cmd.args[0].value; break;
				case 0x8d8: flag = cmd.args[0].value; break;
				case 0x8db: 
					r = cmd.args[0].value;
					g = cmd.args[1].value;
					b = cmd.args[2].value;
					a = cmd.args[3].value;
					flag = cmd.args[4].value;
					break;
				}
				config.SetParam("#WINDOW_ATTR", 5, r, g, b, a, flag);
				SetWindowColor(r, g, b, a, flag);
				cmd.clear();
			}
		} else if (cmd.cmd3 == 0xa28 || cmd.cmd3 == 0xa29 || cmd.cmd3 == 0xa2c || cmd.cmd3 == 0xa2d || cmd.cmd3 == 0xa2e) {
			int v = 0;
			switch(cmd.cmd3) {
			case 0xa28: case 0xa2d: config.GetOriginalParam("#INIT_MESSAGE_SPEED", 1, &v); break;
			case 0xa29: config.GetOriginalParam("#INIT_MESSAGE_SPEED_MOD", 1, &v); break;
			case 0xa2c: config.GetOriginalParam("#MESSAGE_KEY_WAIT_USE", 1, &v); break;
			case 0xa2e: config.GetOriginalParam("#MESSAGE_KEY_WAIT_TIME", 1, &v); break;
			}
			cmd.SetSysvar(v);
		} else if (cmd.cmd3 == 0x913 || cmd.cmd3 == 0x914 || cmd.cmd3 == 0x92f || cmd.cmd3 == 0x8af || cmd.cmd3 == 0x8b0 || cmd.cmd3 == 0x8cb) {
			// テキスト表示速度関連
			int m, speed;
			config.GetParam("#INIT_MESSAGE_SPEED", 1, &speed);
			config.GetParam("#INIT_MESSAGE_SPEED_MOD", 1, &m);
if (cmd.cmd3 == 0x913 || cmd.cmd3 == 0x92f) fprintf(stderr,"TEXT speed %d\n",speed);
else if (cmd.cmd3 == 0x914) fprintf(stderr,"TEXT mode %d\n",m);
else if (cmd.cmd3 == 0x8af || cmd.cmd3 == 0x8cb) fprintf(stderr,"TEXT %d, %d <- speed %d\n",m,speed,cmd.args[0].value);
else fprintf(stderr,"TEXT %d, %d <- mode %d\n",m,speed,cmd.args[0].value);
			if (cmd.cmd3 == 0x913 || cmd.cmd3 == 0x92f) cmd.SetSysvar(speed);
			else if (cmd.cmd3 == 0x914) cmd.SetSysvar(m);
			else {
				if (cmd.cmd3 == 0x8af || cmd.cmd3 == 0x8cb) speed = cmd.args[0].value;
				else m = cmd.args[0].value;
				if (speed < 10) speed = 10;
				else if (speed > 1000) speed = 1000;
				config.SetParam("#INIT_MESSAGE_SPEED", 1, speed);
				config.SetParam("#INIT_MESSAGE_SPEED_MOD", 1, m);
				if (m) speed = -1;
				SetTextSpeed(speed);
				cmd.clear();
			}
		} else if (cmd.cmd3 == 0x92e || cmd.cmd3 == 0x930 || cmd.cmd3 == 0x8ca || cmd.cmd3 == 0x8cc) {
			// テキストオートモード関連
			int m, wait;
			config.GetParam("#MESSAGE_KEY_WAIT_USE", 1, &m);
			config.GetParam("#MESSAGE_KEY_WAIT_TIME", 1, &wait);
if (cmd.cmd3 == 0x92e) fprintf(stderr,"AUTO mode %d\n",m);
else if (cmd.cmd3 == 0x930) fprintf(stderr,"AUTO wait %d\n",wait);
else if (cmd.cmd3 == 0x8ca) fprintf(stderr,"AUTO %d,%d <- mode %d\n",m,wait,cmd.args[0].value);
else fprintf(stderr,"AUTO %d,%d <- wait %d\n",m,wait,cmd.args[0].value);

			if (cmd.cmd3 == 0x92e) cmd.SetSysvar(m);
			else if (cmd.cmd3 == 0x930) cmd.SetSysvar(wait);
			else {
				if (cmd.cmd3 == 0x8ca) m = cmd.args[0].value;
				else wait = cmd.args[1].value;
				if (wait < 0) wait = 0;
				else if (wait > 60000) wait = 60000;
				config.SetParam("#MESSAGE_KEY_WAIT_USE", 1, m);
				config.SetParam("#MESSAGE_KEY_WAIT_TIME", 1, wait);
				if (m) SetTextWait(wait);
				else SetTextWait(-1);
				cmd.clear();
			}
		} else if (cmd.cmd3 == 0x51f && cmd.cmd4 == 0) { // replace_name2 の設定
			int n = cmd.args[0].value;
			if (n>=0 && n<26) {
				replace_name2[n] = cmd.Str(cmd.args[1]);
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x51e && cmd.cmd4 == 0) { // replace_name2 を得る
			int n = cmd.args[0].value;
			if (n >= 0 && n < 26) {
				cmd.SetStrvar(cmd.args[1], replace_name2[n]);
			} else {
				cmd.SetStrvar(cmd.args[1], "");
			}
		} else if (cmd.cmd3 == 0x514 && cmd.cmd4 == 0) { // replace_name を得る
			int n = cmd.args[0].value;
			if (n >= 0 && n < 26) {
				cmd.SetStrvar(cmd.args[1], replace_name[n]);
			} else {
				cmd.SetStrvar(cmd.args[1], "");
			}
		}
	}

	return;
}
extern int print_blit;
bool TextImpl::Wait(unsigned int current_time, Cmd& cmd) {
	if (current_time != 0xffffffffUL) old_time = current_time;
/*
if (event.presscount(MOUSE_UP)) {
if (text) text->Pic()->ReBlit();
}
if (event.presscount(MOUSE_DOWN)) {
print_blit^=1;
}
*/

	if (status == NORMAL && status_mask == NORMAL) return false;
	
	if (status_mask & WAIT_EXTRN_MASK) return true;
	if (status_mask & (BACKLOG_MASK|BACKLOG_MASK_FWD) ) {
		if (status_mask & BACKLOG_WAIT_MASK) ;
		else {
			if ( (status == WAIT_TEXT && text != 0) || status == WAIT_SELECT_INBOX || status == WAIT_SELECT_OUTBOX) {
				if(text && text->wid->status != WidText::PREPARE && text->wid->status != WidText::WAIT && text->wid->status != WidText::WAIT2) {
					text->wid->Flush(); // 表示を最後の状態にする
				}
				if (status == WAIT_TEXT && text != 0 && kcursor) kcursor->show();
			}
		}
		if (status_mask & BACKLOG_MASK) {
			cmd.cmd_type = CMD_BACKLOGREQ;
		} else {
			cmd.cmd_type = CMD_BACKLOGREQ_FWD;
		}
		status_mask = Status(status_mask & ~(BACKLOG_MASK|BACKLOG_MASK_FWD));
		return false;
	}
	if ( (status_mask & BACKLOG_WAIT_MASK) && (status_mask & BACKLOG_MASK_KOE)) {
		if (drawn_backlog_item.koe != -1) {
			cmd.cmd_type = CMD_OTHER;
			cmd.cmd1 = 1;
			cmd.cmd2 = 0x17;
			cmd.cmd3 = 0;
			cmd.cmd4 = 1;
			cmd.args.clear();
			cmd.args.push_back(VarInfo(drawn_backlog_item.koe));
			cmd.args.push_back(VarInfo(0));
		}
		status_mask = Status(status_mask & ~BACKLOG_MASK_KOE);
		return false;
	}
	if (skip_mode & SKIP_IN_MENU) return false;
	if (status_mask & SAVEMASK) {
		cmd.cmd_type = CMD_SAVEREQ;
		status_mask = Status(status_mask & ~SAVEMASK);
		return false;
	}
	if (status_mask & LOADMASK) {
		cmd.cmd_type = CMD_LOADREQ;
		status_mask = Status(status_mask & ~LOADMASK);
		return false;
	}
	if (status_mask & SKIPEND_MASK) {
		if ( (skip_mode & SKIP_TEXT) && (skip_mode & SKIPEND_TEXT)) {
			if (skip_mode & SKIPEND_KEY) { // shift skip 中
				SkipMode new_mode = SkipMode(skip_mode & (~SKIPEND_TEXT));
				if (new_mode & (SKIP_GRP_NOEFFEC || SKIP_GRP_NODRAW))
					new_mode = SkipMode(new_mode & (~SKIP_GRP_FAST));
				cmd.SetSysvar(TYPE_SYS_SKIPMODE, new_mode);
			} else {
				cmd.SetSysvar(TYPE_SYS_SKIPMODE, SKIP_NO);
			}
		}
		status_mask = Status(status_mask & ~SKIPEND_MASK);
	}
	if (status_mask & SKIPMASK) {
		if (skip_mode != SKIP_NO) {
			cmd.SetSysvar(TYPE_SYS_SKIPMODE, skip_mode | SKIPEND_TEXT);
		} else {
			cmd.SetSysvar(TYPE_SYS_SKIPMODE, SKIP_TEXT | SKIP_GRP_FAST | SKIPEND_TEXT);
		}
		status_mask = Status(status_mask & ~SKIPMASK);
		return false;
	}
	if (event.presscount(MOUSE_RIGHT)) {
		if ( (status == WAIT_TEXT && text != 0) || status == WAIT_SELECT_INBOX || status == WAIT_SELECT_OUTBOX) {
			if(text && text->wid->status != WidText::PREPARE && text->wid->status != WidText::WAIT && text->wid->status != WidText::WAIT2) {
				text->wid->Flush(); // 表示を最後の状態にする
			}
			cmd.cmd_type = CMD_MENUREQ;
			if (!(status_mask & STATSAVE_MASK)) {
				status_saved = status;
				status_mask = Status(status_mask | STATSAVE_MASK);
			}
			return false;
		} else if (status == WAIT_CLICK_MOUSEPOS) {
			status = WAIT_CLICK_MOUSEPOSEND_R;
		}
	}
	if (event.presscount(MOUSE_UP)) {
		if ( (status == WAIT_TEXT && text != 0) || status == WAIT_SELECT_INBOX || status == WAIT_SELECT_OUTBOX) {
			if(text && text->wid->status != WidText::PREPARE && text->wid->status != WidText::WAIT && text->wid->status != WidText::WAIT2) {
				text->wid->Flush(); // 表示を最後の状態にする
			}
			cmd.cmd_type = CMD_BACKLOGREQ;
			if (!(status_mask & STATSAVE_MASK)) {
				status_saved = status;
				status_mask = Status(status_mask | STATSAVE_MASK);
			}
			return false;
		}
	}
	if (status_mask & CLEARSCR_MASK) {
		if ( (status == WAIT_TEXT && text != 0 ) || status == WAIT_SELECT_INBOX || status == WAIT_SELECT_OUTBOX) {
			if (skip_mode) skip_mode = SKIP_NO;
			if (text && text->wid->status != WidText::PREPARE && text->wid->status != WidText::WAIT && text->wid->status != WidText::WAIT2) {
				text->wid->Flush(); // 表示を最後の状態にする
				return true;
			}
			status_mask = Status(status_mask & (~CLEARSCR_MASK) | CLEARSCR_WAIT_MASK);
			if (text) text->hide();
			if (kcursor) kcursor->hide();
			if (sel_widget) sel_widget->hide();
			if (backlog_widget) backlog_widget->hide();
			return true;
		}
		status_mask = Status(status_mask & (~CLEARSCR_MASK));
		return false;
	}
	if (status_mask & CLEARSCR_WAIT_MASK) {
		return true;
	}
	if (status == WAIT_TEXT) {
		if (text == 0) { status = NORMAL; return false;}
		if (skip_mode & SKIP_TEXT) {
		} else if (text->wid->status != WidText::PREPARE) {
			return true;
		}
		if (kcursor) kcursor->hide();
		text_stream.Clear();
		status = NORMAL;
		cmd.cmd_type = CMD_TEXTEND;
		return false;
	}
	if (status == WAIT) {
		if (skip_mode & SKIP_TEXT) ;
		else if (wait_time > current_time) return true;
		status = NORMAL;
	} else if (status == WAIT_CLICK) {
		if (skip_mode & SKIP_TEXT) ;
		else if (wait_time > current_time) return true;
		status = NORMAL;
		cmd.SetSysvar(0);
	} else if (status == WAIT_ABORT) {
		cmd.SetSysvar(1);
		status = NORMAL;
	} else if (status == WAIT_CLICK_MOUSEPOS || status == WAIT_CLICK_MOUSEPOSEND_L || status == WAIT_CLICK_MOUSEPOSEND_R) {
		if (status == WAIT_CLICK_MOUSEPOS && (skip_mode & SKIP_TEXT) == 0) return true; // keep wait
		else {
			int x, y;
			event.MousePos(x,y);
			if (status == WAIT_CLICK_MOUSEPOS) x = y = 0; // skip mode
			cmd.clear();
			cmd.SetFlagvar(wait_savedvar[0], x);
			cmd.SetFlagvar(wait_savedvar[1], y);
			if (status == WAIT_CLICK_MOUSEPOSEND_R) cmd.SetSysvar(-1);
			else cmd.SetSysvar(0);
			status = NORMAL;
		}
	} else if (status == WAIT_SELECT_INBOX || status == WAIT_SELECT_OUTBOX) {
		return true;
	} else if ( int(status) >= WAIT_SELECT_VALUE) {
		int sel_val = int(status) - WAIT_SELECT_VALUE;
		cmd.SetSysvar(sel_val);
		selects.clear();
		delete sel_widget;
		sel_widget = 0;
		status = NORMAL;
		// CreateSelect() で作成された cur_backlog_item を backlog_item へ反映させる
		cur_backlog_item.text.InsertColor(sel_backlog_pos[sel_val], sel_backlog_pos[sel_val+1], 0xff, 0, 0);
		backlog_item = cur_backlog_item;
		cur_backlog_item.Clear();
	}
	return false;
}

void clearbtn_press(void* pointer, WidButton* button) {
	if (pointer == 0) return;
	TextImpl* t = (TextImpl*)pointer;
	t->status_mask = TextImpl::Status(t->status_mask | TextImpl::CLEARSCR_MASK);
	return;
}
void TextImpl::PressFuncSkip(void* pointer, WidButton* from) {
	if (pointer == 0) return;
	TextImpl* t = (TextImpl*)pointer;
	t->status_mask = TextImpl::Status(t->status_mask | TextImpl::SKIPMASK);
	return;
}
void TextImpl::PressFuncLoad(void* pointer, WidButton* from) {
	if (pointer == 0) return;
	TextImpl* t = (TextImpl*)pointer;
	t->status_mask = TextImpl::Status(t->status_mask | TextImpl::LOADMASK);
	return;
}
void TextImpl::PressFuncSave(void* pointer, WidButton* from) {
	if (pointer == 0) return;
	TextImpl* t = (TextImpl*)pointer;
	t->status_mask = TextImpl::Status(t->status_mask | TextImpl::SAVEMASK);
	return;
}
void TextImpl::PressFuncBacklog(void* pointer, WidButton* from) {
	if (pointer == 0) return;
	TextImpl* t = (TextImpl*)pointer;
	t->status_mask = TextImpl::Status(t->status_mask | TextImpl::BACKLOG_MASK);
	return;
}
void TextImpl::PressFuncBacklogFwd(void* pointer, WidButton* from) {
	if (pointer == 0) return;
	TextImpl* t = (TextImpl*)pointer;
	t->status_mask = TextImpl::Status(t->status_mask | TextImpl::BACKLOG_MASK_FWD);
	return;
}
void movebtn_drag(int from_x, int from_y, int x, int y, void* pointer, WidButton* button) {
	if (pointer == 0) return;
	fprintf(stderr,"drag.\n");
}
#define BTNCNT 10
static char* btnname[BTNCNT] = {
	"MOVE",
	"CLEAR",
	"READJUMP",
	"AUTOMODE",
	"MSGBK",
	"MSGBKLEFT",
	"MSGBKRIGHT",
	"EXBTN_000",
	"EXBTN_001",
	"EXBTN_002"
};
static int btnpos[BTNCNT] = { // g00 ファイル内のボタン情報の位置
//	0, 1, 13, 12, 2, 3, 4, 5, 6, 7 // princess bride?
	0, 1, 13, 14, 2, 3, 4, 5, 6, 7 // tomoyo after?
};
static WidButton::PressFunc btnpress[BTNCNT] = {
	0, clearbtn_press, &TextImpl::PressFuncSkip,0,&TextImpl::PressFuncBacklogFwd,&TextImpl::PressFuncBacklog,&TextImpl::PressFuncBacklogFwd,&TextImpl::PressFuncSave,&TextImpl::PressFuncLoad,0
};
static WidButton::DragFunc btndrag[BTNCNT] = {
	movebtn_drag, 0,0,0,0, 0,0,0,0, 0
};

void TextImpl::SetTextSpeed(int speed) {
	// 100 : 10char / sec
	// 10 : 100char / sec
	// text widget:
	if (speed <= 0) speed = -1;
	else if (speed > 1000) speed = 1;
	else speed = 1000 / speed;
	int i;
	for (i=0; i<32; i++)
		if (widgets[i]) widgets[i]->wid->SetSpeed(speed);
}
void TextImpl::SetTextWait(int wait) {
	int i;
	for (i=0; i<32; i++)
		if (widgets[i]) widgets[i]->wid->SetWait(wait);
}

void TextImpl::SetWindowColor(int r, int g, int b, int a, bool is_transparent) {
	char key[1024];
	int w;

	for (w=0; w<32; w++) {
		if (widgets[w] == 0) continue;
		sprintf(key, "#WAKU.%03d.000.BACK", w);
		const char* back = config.GetParaStr(key);
		if (back == 0 || back[0] == 0) continue;
		sprintf(key, "%s.g00", back);
		Surface* back_s = parent.Root().NewSurface(key);
		if (back_s == 0) continue;
		Rect rect(*back_s);
		Surface* new_s = parent.Root().NewSurface(rect.width(), rect.height(), ALPHA_MASK);
		DSurfaceMove(back_s, rect, new_s, rect);
		DSurfaceFillA(new_s, rect, r, g, b, a);
		widgets[w]->wid->Pic()->SetSurface(new_s, 0, 0);
		widgets[w]->wid->Pic()->SetSurfaceFreeFlag(1);
		if (!is_transparent)
			widgets[w]->wid->Pic()->SetSurfaceAttribute(PicBase::BLIT_MULTIPLY);
		parent.Root().DeleteSurface(back_s);
	}
	return;
}

void TextImpl::SetCursor(int cursor_no) {
	char key[1024];
	sprintf(key, "#CURSOR.%03d.NAME", cursor_no);
	string path = config.GetParaStr(key);
	if (path.length() == 0) return; // 名前なし
	path += ".pdt";
	int w,h,cont,speed;
	sprintf(key, "#CURSOR.%03d.SIZE", cursor_no);
	config.GetParam(key, 2, &w, &h);
	sprintf(key, "#CURSOR.%03d.CONT", cursor_no);
	config.GetParam(key, 1, &cont);
	sprintf(key, "#CURSOR.%03d.SPEED", cursor_no);
	config.GetParam(key, 1, &speed);

	// speed で１周、cont 回変化
	if (kcursor) delete kcursor;

	kcursor = new WidTimeCursor(event, speed/cont, &parent, path.c_str(), 0, 0, w, 0, cont, Rect(0,0,w,h));
	int i;
	for (i=0; i<32; i++) {
		if (widgets[i]) widgets[i]->wid->SetCursor(kcursor);
	}
}

void kconv(const unsigned char* src, unsigned char* dest) {
	/* input : sjis output: euc */
    unsigned int low = 0;
	while(*src) {
		unsigned int high = *src++;
		if (high < 0x80) {
			/* ASCII */
			*dest++ = high; continue;
		} /*else if (high < 0xa0) {
			/* SJIS */
		/*	high -= 0x71;
		} else if (high < 0xe0) {
			/* hankaku KANA */
		//	*dest++ = 0x8e; *dest++ = high;
	//		continue;
	//	} else { /* high >= 0xe0 : SJIS */
	/*		high -= 0xb1;
		}
		/* SJIS convert */
	/*	high = (high<<1) + 1;

		unsigned int low = *src++;
		if (low == 0) break; /* incorrect code */
		/*if (low > 0x7f) low--;
		if (low >= 0x9e) {
			low -= 0x7d;
			high++;
		} else {
			low -= 0x1f;
		}*/
        else{
            low = *src++;
            *dest++ = high ; *dest++ = low ;
        }
	}
	*dest = 0;
}
void kconv_rev(const unsigned char* src, unsigned char* dest) {
	/* input : euc output: sjis */
	while(*src) {
		unsigned int high = *src++;
		if (high < 0x80) {
			/* ASCII */
			*dest++ = high; continue;
		} else if (high == 0x8e) { /* hankaku KANA */
			high = *src;
			if (high >= 0xa0 && high < 0xe0)
				*dest++ = *src++;
			continue;
		} else {
			unsigned int low = *src++;
			if (low == 0) break; /* incorrect code , EOS */
			if (low < 0x80) continue; /* incorrect code */
			/* convert */
			low &= 0x7f; high &= 0x7f;
			low += (high & 1) ? 0x1f : 0x7d;
			high = (high-0x21)>>1;
			high += (high > 0x1e) ? 0xc1 : 0x81;
			*dest++ = high;
			if (low > 0x7f) low++;
			*dest++ = low;
		}
	}
	*dest = 0;
}
string kconv(const string& s) {
	char* out = new char[s.length()*2+100];
	kconv((const unsigned char*)s.c_str(), (unsigned char*)out);
	string ret = out;
	delete[] out;
	return ret;
}
string kconv_rev(const string& s) {
	char* out = new char[s.length()*2+100];
	kconv_rev((const unsigned char*)s.c_str(), (unsigned char*)out);
	string ret = out;
	delete[] out;
	return ret;
}

/**************************************************************::
**
**	Text
*/
Text::Text(Event::Container& _event, PicContainer& _parent, AyuSysConfig& config) {
	pimpl = new TextImpl(_event, _parent, config, backlog, backlog_item);
}
Text::~Text() {
	delete pimpl;
}
void Text::InitWindow(void) {
	pimpl->InitWindow();
}
void Text::Exec(Cmd& cmd) {
	pimpl->Exec(cmd);
}
bool Text::Wait(unsigned int current_time, Cmd& cmd) {
	return pimpl->Wait(current_time, cmd);
}
void Text::SetSkipMode(SkipMode mode) {
	pimpl->SetSkipMode(mode);
}
void Text::Save(std::string& str, bool select_save) {
	pimpl->Save(str, select_save);
}
void Text::Load(const char* str) {
	pimpl->Load(str);
}

void Text::hide(void) {
	pimpl->hide();
}
void Text::show(void) {
	pimpl->show();
}
void Text::show(int num) {
	pimpl->show(num);
}
void Text::DrawBacklog(BacklogItem& item, Cmd& cmd) {
	pimpl->DrawBacklog(item, cmd);
}
/**************************************************************::
**
**	BacklogItem
*/

BacklogItem::BacklogItem(void) {
	scn = -1;
	pos = -1;
	koe = -1;
	face = "";
	text.kanji_type = TextStream::sjis;
}
void BacklogItem::Clear(void) {
	scn = -1;
	pos = -1;
	koe = -1;
	text.Clear();
}
void BacklogItem::AddTextPos(Cmd& cmd) {
	if (scn == -1 && pos == -1) {
		scn = cmd.scn;
		pos = cmd.pos;
		return;
	}
	DeleteTextPos();
}
void BacklogItem::DeleteTextPos(void) {
	scn = 0;
	pos = -1;
}
BacklogItem& BacklogItem::operator =(const BacklogItem& p) {
	scn = p.scn;
	pos = p.pos;
	koe = p.koe;
	face = p.face;
	text = p.text;
}
void BacklogItem::SetSavepos(int p) {
	Clear();
	scn = SaveSelect;
	pos = p;
}

Rect TextWindow::WakuSize(PicContainer& pic, int waku_no, const AyuSysConfig& config) {
	char key[1024];
	sprintf(key, "#WAKU.%03d.000.NAME", waku_no);
	const char* name = config.GetParaStr(key);
	if (!name) return Rect(0,0,0,0);
	std::string str = name; str += ".g00";
	Surface* s = pic.Root().NewSurface(str.c_str());
	if (!s) return Rect(0,0,0,0);
	Rect r(*s);
	pic.Root().DeleteSurface(s);
	return r;
}
void TextWindow::MakeWaku(PicContainer& pic, Event::Container& event, int waku_no, int window_no, bool* use_btn, const AyuSysConfig& config, void* callback) {
	char key[1024];
	std::string str;
	/* 枠を作成 */
	sprintf(key, "#WAKU.%03d.000.NAME", waku_no);
	const char* name = config.GetParaStr(key);
	if (name && name[0] == 0) name = 0;
	sprintf(key, "#WAKU.%03d.000.BACK", waku_no);
	const char* back = config.GetParaStr(key);
	if (back && back[0] == 0) back = 0;
	sprintf(key, "#WAKU.%03d.000.BTN", waku_no);
	const char* btn = config.GetParaStr(key);
	if (btn && btn[0] == 0) btn = 0;

	if (name == 0 && back == 0 && btn == 0) return;

	/* まず、テキスト背景を設定 */
	if (back) {
		str = back; str += ".g00";
		int rc,gc,bc,ac, flag;
		char key[1024];
		sprintf(key, "#WINDOW.%03d.ATTR", window_no);
		if (config.GetParam(key, 5, &rc, &gc, &bc, &ac, &flag) == -1) {
			config.GetParam("#WINDOW_ATTR", 5, &rc, &gc, &bc, &ac, &flag);
		}
		Surface* back_s = pic.Root().NewSurface(str.c_str());
		if (back_s) {
			Rect rect(*back_s);
			Surface* s = pic.Root().NewSurface(rect.width(), rect.height(), ALPHA_MASK);
			DSurfaceMove(back_s, rect, s, rect);
			DSurfaceFillA(s, rect, rc, gc, bc, ac); // 透明度設定
			pic.SetSurface(s, 0, 0);
			pic.SetSurfaceFreeFlag(1);
			if (flag == 0) wid->Pic()->SetSurfaceAttribute(PicBase::BLIT_MULTIPLY);
			pic.Root().DeleteSurface(back_s);
		}
	}
	/* その前に枠飾りを設定 */
	if (name) {
		str = name; str += ".g00";
		Surface* s = pic.Root().NewSurface(str.c_str());
		if (s) {
			Rect rect(*s);
			pic.Root().DeleteSurface(s);
			PicBase* p = pic.create_leaf(Rect(0, 0, rect.width(), rect.height()),0);
			p->SetSurface(str.c_str(), 0, 0);
			p->ZMove(ZMOVE_BOTTOM);
			p->show();
		}
	}
	if (btn == 0) return;
	if (use_btn == 0) return;
	// ボタンの作成
	// 使用するボタンについては、必要に応じて show() すること

	/* ボタンの位置情報を求める */
	str = btn; str += ".g00";
	ARCINFO* info = file_searcher.Find(FILESEARCH::PDT, str.c_str(), "g00");
	if (info == 0) return; // cannot find file
	const char* data = info->Read();
	/* g00 ファイルのヘッダ部分に位置情報は入っている */
	/* 存在しなければボタン画像ではない */
	if (data == 0 || *data != 2) {
		delete info;
		return;
	}
	int index_count = read_little_endian_int(data+5); // 0x70 == 112 ( 8 個ずつグループなので、14個のボタン ) が標準
	int i;
	for (i=0; i<BTNCNT; i++) {
		if (!use_btn[i]) continue;
		if (btnpos[i]*8 >= index_count) {
			continue; // ボタンが存在しない
		}
		int x, y, w, h;
		sprintf(key, "#WAKU.%03d.000.%s_BOX", waku_no, btnname[i]);
		if (config.GetParam(key, 5, 0, &x, &y, &w, &h) == -1) continue;
		int sx, sy, sdx, sdy, cnt;
		const char* d = data + 9 + btnpos[i]*24*8;
		sx = read_little_endian_int(d);
		sy = read_little_endian_int(d+4);
		sdx = read_little_endian_int(d+24) - sx;
		sdy = read_little_endian_int(d+24 + 4) - sy;
		cnt = 2;
		if (sx+sdx*2 == read_little_endian_int(d+2*24) && sy+sdy*2 == read_little_endian_int(d+2*24+4)) cnt = 3;
		WidButton* wid = new WidButton(event, &pic, str.c_str(), sx, sy, sdx, sdy, cnt, Rect(x, y, x+w, y+h), 1);
		if (btnpress[i]) { wid->press_func = btnpress[i]; wid->press_pointer = callback;}
		if (btndrag[i]) { wid->drag_func = btndrag[i]; wid->drag_pointer = callback;}
	}
	delete info;
	return;
}

TextWindow::TextWindow(PicContainer& parent, Event::Container& event, int win_no, const AyuSysConfig& config, void* callback) :
	wid(0), name_visible(true),name(0),name_container(0), face(0) {
	int i; for (i=0; i<8; i++) face_pics[i]=0;
	char key[1024];
	bool use_btn[BTNCNT];
	int size, rep1, rep2, cntw, cnth, mposx, mposy, posd, posx, posy, minx, miny, waku_no, ruby;
	sprintf(key, "#WINDOW.%03d.MOJI_SIZE", win_no); if (config.GetParam(key, 1, &size) == -1) return;
	sprintf(key, "#WINDOW.%03d.MOJI_REP", win_no);  if (config.GetParam(key, 2, &rep1, &rep2) == -1) return;
	sprintf(key, "#WINDOW.%03d.MOJI_CNT", win_no);  if (config.GetParam(key, 2, &cntw, &cnth) == -1) return;
	sprintf(key, "#WINDOW.%03d.POS", win_no);       if (config.GetParam(key, 3, &posd, &posx, &posy) == -1) return;
	sprintf(key, "#WINDOW.%03d.MOJI_POS", win_no);  if (config.GetParam(key, 4, &mposy, 0, &mposx, 0) == -1) return;
	sprintf(key, "#WINDOW.%03d.MOJI_MIN", win_no);  if (config.GetParam(key, 2, &minx, &miny) == -1) return;
	sprintf(key, "#WINDOW.%03d.WAKU_SETNO", win_no);if (config.GetParam(key, 1, &waku_no) == -1) return;
	sprintf(key, "#WINDOW.%03d.LUBY_SIZE", win_no); if (config.GetParam(key, 1, &ruby) == -1) return;

	/* テキストウィジット：画面の右下一杯まで使用 */
	/* posd == 2 なら画面下にひっつくように配置 */
	Rect r(0,0);
	if (posd == 2) {
		r = WakuSize(parent, waku_no, config);
		r = Rect(0, parent.Height()-r.height(), r.width(), parent.Height());
		posx = 0;
		posy = parent.Height()-r.height();
	} else /* posd == 0 ? */
		r = Rect(posx, posy, parent.Width(), parent.Height());

	/* テキストウィンドウの作成 */
	int w = size*cntw; int h = (size+ruby+2)*cnth;
	wid = new WidText(event, &parent, r, Rect(mposx, mposy, mposx+w, mposy+h), size);
	wid->stream.kanji_type = TextStream::sjis;
	/* 顔ウィンドウの作成 */
	for (i=0; i<8; i++) {
		int x,y;
		sprintf(key, "#WINDOW.%03d.FACE.%03d", win_no, i);
		if (config.GetParam(key, 2, &x, &y) == -1) continue;
		/* 顔ウィンドウを作成する */
		if (x >= 0 && y >= 0) {
			face_pics[i] = wid->PicNode()->create_leaf(Rect(x,y), PicBase::FIT_SURFACE);
		} else {
			face_pics[i] = parent.create_leaf(Rect(x+posx,y+posy), PicBase::FIT_SURFACE);
		}
		face_pics[i]->show();
	}
	face = face_pics[0];
	// ボタンの設定
	for (i=0; i<BTNCNT; i++) {
		int num;
		sprintf(key, "#WINDOW.%03d.%s_USE", win_no, btnname[i]);
		config.GetParam(key, 1, &num);
		use_btn[i] = (num==0) ? false : true;
	}
	// make name window
	int shadow, name_mod, name_size, name_min, name_center, name_posx, name_posy, name_mposx, name_mposy;
	sprintf(key, "#WINDOW.%03d.MOJI_SHADOW", win_no);  config.GetParam(key, 1, &shadow);
	sprintf(key, "#WINDOW.%03d.NAME_MOD", win_no);  config.GetParam(key, 1, &name_mod);
	sprintf(key, "#WINDOW.%03d.NAME_MOJI_SIZE", win_no);  config.GetParam(key, 1, &name_size);
	sprintf(key, "#WINDOW.%03d.NAME_MOJI_MIN", win_no);  config.GetParam(key, 1, &name_min);
	sprintf(key, "#WINDOW.%03d.NAME_MOJI_POS", win_no);  config.GetParam(key, 2, &name_mposx, &name_mposy);
	sprintf(key, "#WINDOW.%03d.NAME_CENTERING", win_no);  config.GetParam(key, 1, &name_center);
	sprintf(key, "#WINDOW.%03d.NAME_POS", win_no);  config.GetParam(key, 2, &name_posx, &name_posy);
	// if name_mode==0 name is in the text window
	// if name_mode == 1 open name window
	// if name_mode == 2 name is not used
	if (name_mod) {
		if (name_mod == 1) {
			int w = name_size*name_min; int h = name_size;
			int name_waku;
			sprintf(key, "#WINDOW.%03d.NAME_WAKU_SETNO", win_no);
			if (config.GetParam(key, 1, &name_waku) != -1 && name_waku != -1) {
				Rect waku_r = WakuSize(parent, name_waku, config);
				waku_r.rmove(r.lx, r.ty); // テキストウィンドウ位置に動かす
				waku_r.rmove(name_posx, name_posy-waku_r.height()); // NAME_POS へ位置補正
				name_container = parent.create_node(waku_r, 0);
				MakeWaku(*name_container, event, name_waku, win_no, 0, config, callback);
				Rect name_r(0,0,w,h);
				name_r.rmove(name_mposx, name_mposy);
				name = new WidLabel(name_container, name_r, true, 0, name_size);
				name->show();
			} else { // 名前専用枠なし
				Rect name_r(0, 0, w, h);
				name_r.rmove(r.lx, r.ty);
				name_r.rmove(name_posx, name_posy-name_size);
				name_container = parent.create_node(name_r, 0);
				name = new WidLabel(name_container, Rect(0,0,w,h), true, 0, name_size);
				name->show();
				name_container->show();
			}
		} else { // name_mod == 2 or 3
			name_container = parent.create_node( Rect(0,0,1,1), 0);
		}
	}
	MakeWaku(*wid->PicNode(), event,waku_no, win_no, use_btn, config, callback);
}
void TextImpl::InitWindow(void) {
	int i,j,k;
	int w;
	std::string str;

	for (w=0; w<32; w++) {
		widgets[w] = new TextWindow(parent, event, w, config, (void*)this);
		if (widgets[w]->wid == 0) {
			delete widgets[w];
			widgets[w] = 0;
		}
	}
	SetCursor(0);
	for (i=0; i<26; i++) {
		char buf[1024];
		sprintf(buf, "#NAME.%c", i+'A');
		const char* s = config.GetParaStr(buf);
		if (s) replace_name[i] = s;
	}
	// replace_name2 : 初期設定
	// 渚、秋生、渚 (CLANNAD)
	char name_nagisa[3] = {0x8f,0x8d,0};
	char name_akio[5] = {0x8f, 0x48, 0x90, 0xb6, 0};
	replace_name2[0] = name_nagisa;
	replace_name2[1] = name_akio;
	replace_name2[2] = name_nagisa;
	text = 0;
	/* テキスト速度の設定 */
	int speed, mod, wait, auto_mod;
	config.GetParam("#INIT_MESSAGE_SPEED", 1, &speed);
	config.GetParam("#INIT_MESSAGE_SPEED_MOD", 1, &mod);
	config.GetParam("#MESSAGE_KEY_WAIT_USE", 1, &auto_mod);
	config.GetParam("#MESSAGE_KEY_WAIT_TIME", 1, &wait);
	if (mod) speed = -1;
	if (!auto_mod) wait = -1;
	SetTextSpeed(speed);
	SetTextWait(wait);
	return;
}
