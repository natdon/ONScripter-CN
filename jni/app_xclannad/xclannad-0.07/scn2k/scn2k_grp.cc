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
#include"window/widget.h"
#include"system/file.h"
#include"system/system_config.h"
#include"font/text.h"
#include<set>

using namespace std;

extern void DSurfaceFill(Surface* dest, const Rect& rect, int r, int g, int b, int a=0xff);
extern void DSurfaceMove(Surface* src_o, const Rect& srcrect, Surface* dst_o, const Rect& dstpos);
extern Rect DSurfaceRenderText(TextGlyphStream::iterator start, TextGlyphStream::iterator end, const Rect& srcrect,
        Surface* dst, const Rect& dstrect);
extern XKFont::HorizLayout* DefaultLayout(int text_size);

/*******************************************************************
** GrpObj(interface)
*/

struct SEL {
	Rect from;
	Rect to;
	int time;
	int sel_no;
	int args[8];
	SEL() : from(0,0), to(0,0) {}
};

struct GrpObj {
	string name;
	string gan_name;
	PicContainer* pic_parent;
	PicBase* picture;
	WidAnmTime* anm;
	int _posx, _posy;
	int posx[9],posy[9];
	Rect clip_area;
	unsigned char alpha;
	int order;
	int surface_num;

	string print_moji;
	int print_size, print_r, print_b, print_g;

	int dig_number, dig_digit;

	// zoom / rotate 関係
	int zoom; // 256 で 1 倍
	int rotate; // 0-360度

	vector<Rect> src_pos;
	enum GrpType { FILLRECT = 1, FILE = 2, GAN = 3, MOJI = 4, DIGIT = 5} gtype;
	enum Attribute { NONE=0, WIPEON=1, SATURATE=2, HIDDEN=4, HIDDEN_GROUP=8,
		UPDATE_PICTURE = 8, UPDATE_POS = 16, UPDATE_ALPHA = 32, UPDATE_SNUM = 64, UPDATE_CLIP = 128, UPDATE_VISIBLE = 256,
		UPDATE_ALL = (8|16|32|64|128|256),
		ANM_PLAYSTART = 0x8000, ANM_PLAYING = 0x10000,
		DIG_ZERO = 0x10000*2, DIG_SIGN = 0x10000*4, DIG_PACK=0x10000*8,DIG_SPACE=0x10000*16
		};
	Attribute attr;

	GrpImpl* parent_pimpl;

	GrpObj(void);
	~GrpObj(void);

	void SetPos(int index, int x, int y);
	void GetPos(int index, int& x, int& y);
	int PosX(void);
	int PosY(void);
	void SetAlpha(void);
	void SetAlpha(int alpha);
	void SetSurfaceNum(int num = -1);
	void SetZoomRotate(int zoom=-1, int rotate=-1);
	void SetClipArea(int x, int y, int width, int height);
	void GetSrcGeom(int& width, int& height);
	void SetUpdate(void);
	TextStream ParseMoji(const char* str, int def_r ,int def_g, int def_b, int def_size);
	void UpdateMoji(void);
	void UpdateDigit(void);
	void UpdateSurface(void);
	void ZoomRotate(void);
	void Update(void);
	void CreateSurface(PicContainer* parent);
	void CreateGan(Event::Container& event, int event_number);
	void CreateGanSpecial(Event::Container& event, int event_number, int time);
	PicBase* DeletePic(void);
};

/*******************************************************************
** GrpObj(interface)
*/


struct GrpObjMap : std::map<int, GrpObj> {
	typedef pair<const int, GrpObj> value_type;
	class GrpImpl* parent;
	GrpObj& operator[](const int& k) {
		iterator it = lower_bound(k);
		if (it == end() || it->first != k) {
			GrpObj obj;
			obj.parent_pimpl = parent;
			it = insert(it, value_type(k, obj));
		}
		return it->second;
	}
	GrpObjMap(class GrpImpl* p) {
		parent = p;
	}
};

class GrpImpl {
#define MAXPDT 256
#define WORKPDT 255
	Event::Container& event;
	const Flags& flags;
	PicBase* screen;
	PicBase* screen_front;
	Surface* surface, *surface_update;

	Surface* dsurface[MAXPDT]; // 書き込み可能な Surface
	Surface* ssurface[MAXPDT]; // ファイルの内容等、読み込みのみ可能な状態の Surface
	PicContainer& parent;

	// 画像効果の保存用
	WidAnmTime* anm1, *anm2;
	typedef enum { NORMAL, WAIT_ANM, WAIT_SHAKE, WAIT_SE, WAIT_MOVIE} Status;
	Status status;
	SkipMode skip_mode;

	std::string bg_name;
	std::map<int, SEL> anmtype;
	GrpObjMap grpobj;
	GrpObjMap bs_obj;
	void CreateObj(int number);
	void ZMoveObj(int number);
	void SetObjChanged(int number);
	void SetObjChangedGroup(int number);
	void SwapObj(int a1, int a2);
	void DeleteObjPic(int num);// object の surface のみ削除
	void DeleteObj(int num);
	void DeleteObjRange(int num_b, int num_e);

	std::set<int> changed_obj;
	string reserved_load_surface0;
	vector<PicBase*> deleted_pic;
	void RefreshObj(void);

	Surface* Dsurface(int pdt);
	Surface* Ssurface(int pdt);

	// cgmode 用画像処理関連
	void LoadCgm(AyuSysConfig& config);
	std::map<std::string, int> cgm_info;
	set<int>& cgm_data;
	
	class MuSys& music;

public:
	AyuSysConfig& config;
	void LoadSurface(const char* str, int pdt);
private:
	void LoadSurface(const char* str);
	void LoadSurface(void);
	void AddSurface(const char* str);

	void StartAnm(int type);
	void StartShake(int total, const int* pattern);
	void AbortAnm(void);
	static bool Pressed(int x, int y, void* pointer);
public:
	GrpImpl(Event::Container& _event, PicContainer& _parent, const Flags& _flag, set<int>& _cgm_data, class MuSys& mu, AyuSysConfig& config);
	~GrpImpl();
	bool Wait(unsigned int current_time, Cmd& cmd);
	void Exec(Cmd& cmd);
	void InitSel(AyuSysConfig& config);
	void Save(std::string& str);
	void Load(const char* str);
	void SaveSys(std::string& str);
	void LoadSys(const char* str);
	void SetSkipMode(SkipMode _mode);
};
/*******************************************************************
** GrpObj(implementation)
*/

GrpObj::GrpObj(void) :
	name(""), gan_name(""), pic_parent(0), picture(0), anm(0),
	_posx(0), _posy(0), clip_area(0,0,0,0),
	alpha(255), order(0), surface_num(0), print_moji(""), print_size(0), print_r(-1),print_g(-1),print_b(-1),
	dig_number(0), dig_digit(0),
	zoom(-1), rotate(-1), attr(GrpObj::HIDDEN), parent_pimpl(0) {
	int i;
	for (i=0; i<9; i++) {
		posx[i] = posy[i] = 0;
	}
}
GrpObj::~GrpObj() {
	if (picture) delete picture;
	if (parent_pimpl == 0) {
		fprintf(stderr,"\n**************\nFATAL : UNINITIALIZED GrpObj IS FOUND!!! \n**************\n");
	}
}
int GrpObj::PosX() {
	return _posx;
}
int GrpObj::PosY() {
	return _posy;
}
void GrpObj::SetUpdate(void) {
	attr = Attribute (attr | UPDATE_PICTURE);
}
void GrpObj::SetPos(int index, int x,int y) {
	if (index < 0 || index > 8) {
		fprintf(stderr,"GrpObj::SetPos: Invalid index %d <- %d,%d\n",index,x,y);
		return;
	}
	if (x == posx[index] && y == posy[index]) return;
	attr = Attribute(attr | UPDATE_POS);
	_posx += x-posx[index];
	_posy += y-posy[index];
	posx[index] = x;
	posy[index] = y;
}
void GrpObj::GetPos(int index, int& x, int& y) {
	if (index < 0 || index > 8) {
		fprintf(stderr,"GrpObj::GetPos: Invalid index %d\n",index);
		x = 0; y = 0;
		return;
	}
	x = posx[index];
	y = posy[index];
	return;
}
void GrpObj::SetAlpha(int new_alpha) {
	if (alpha == new_alpha) return;
	alpha = new_alpha;
	attr = Attribute(attr | UPDATE_ALPHA);
	return;
}
void GrpObj::SetSurfaceNum(int num) {
	if (num != -1) {
		if (surface_num == num) return;
		surface_num = num;
	}
	attr = Attribute(attr | UPDATE_SNUM);
	return;
}

void GrpObj::SetClipArea(int x, int y, int w, int h) {
	Rect new_clip(x,y,x+w,y+h);
	if (clip_area == new_clip) return;
	clip_area = new_clip;
	attr = Attribute(attr | UPDATE_CLIP);
	return;
}
PicBase* GrpObj::DeletePic(void) {
	PicBase* p = picture;
	anm = 0;
	picture = 0;
	src_pos.clear();
	attr = Attribute(attr & (HIDDEN | HIDDEN_GROUP));
	return p;
}
void GrpObj::GetSrcGeom(int& width, int& height) {
	if (src_pos.empty()) {
		width = 0; height = 0;
		if (name.length() == 0) {
			return;
		}
		/* ボタンの位置情報を求める */
		/* g00 ファイルのヘッダ部分に位置情報は入っている */
		string path(name);
		path += ".g00";
		ARCINFO* info = file_searcher.Find(FILESEARCH::PDT, path.c_str(), "g00");
		if (info == 0) { // ファイルが見つからない
			fprintf(stderr,"GrpObj::GetSrcGeom : Cannot find file %s\n", path.c_str());
			return;
		}
		const char* data = info->Read();
		int srclen = read_little_endian_int(data+5);
		if (data && *data == 2) { // 画像ファイル内にボタン情報が存在する
			int srclen = read_little_endian_int(data+5);
			int i;
			for (i=0; i<srclen; i++) {
				int x1 = read_little_endian_int(data+9+i*24+0);
				int y1 = read_little_endian_int(data+9+i*24+4);
				int x2 = read_little_endian_int(data+9+i*24+8);
				int y2 = read_little_endian_int(data+9+i*24+12);
				src_pos.push_back(Rect(x1, y1, x2+1, y2+1));
				if (width < src_pos.back().width()) width = src_pos.back().width();
				if (height < src_pos.back().height()) height = src_pos.back().height();
			}
		} else { // 画像ファイルから大きさ取得
			width = read_little_endian_short(data+1);
			height = read_little_endian_short(data+3);
			src_pos.push_back(Rect(0,0,width,height));
		}
		delete info;
	}
	int sn = surface_num;
	if (sn < 0 || sn > src_pos.size()) sn = 0;
	width = src_pos[sn].width();
	height = src_pos[sn].height();
	return;
}
void GrpObj::Update(void) {
	if (attr & UPDATE_PICTURE) {
		UpdateSurface();
		attr = Attribute( (attr | UPDATE_ALL) & (~UPDATE_PICTURE));
	}
	if (picture == 0) return;
	if (attr & UPDATE_POS) {
		if (attr & SATURATE) {
			int w=0, h=0;
			GetSrcGeom(w,h);
			picture->Move(_posx-w/2, _posy-h/2);
		} else {
			picture->Move(_posx, _posy);
		}
	}
	if (attr & UPDATE_ALPHA) {
		if (alpha <= 0) {
			picture->SetSurfaceAlpha(0, Rect(0,0));
			picture->hide();
		} else if (alpha >= ALPHA_MAX) {
			picture->SetSurfaceAlpha(0, Rect(0,0));
			if (attr & HIDDEN) picture->hide();
			else picture->show();
		} else {
			picture->SetSurfaceAlpha(&alpha, Rect(0,0,1,1));
			if (attr & HIDDEN) picture->hide();
			else picture->show();
		}
	}
	if ( (attr & UPDATE_SNUM) && (!src_pos.empty())) {
		if (surface_num < 0 || surface_num >= src_pos.size()) surface_num = 0;
		picture->SetSurfacePos(src_pos[surface_num].lx, src_pos[surface_num].ty);
	}
	if (attr & UPDATE_CLIP) {
		picture->SetClipArea(clip_area);
	}
	attr = Attribute(attr & (~UPDATE_ALL));
	if (attr & ANM_PLAYSTART) {
		if (anm) {
			anm->Play();
			attr = Attribute(attr | ANM_PLAYING);
		}
		attr = Attribute(attr & (~ANM_PLAYSTART));
	}
}
void GrpObj::CreateSurface(PicContainer* parent) {
	if (picture) {
		PicBase* p = DeletePic();
		delete p;
	}
	src_pos.clear();
	// picture を作成
	pic_parent = parent;
	picture = parent->create_leaf(Rect(_posx,_posy,_posx+1,_posy+1), 0);
	picture->hide();
	UpdateSurface();
}
  
void GrpObj::UpdateSurface(void) {
	if (pic_parent == 0 || picture == 0) return;
	int width = 0, height = 0;
	if (gtype == FILE || gtype == GAN) {
		if (name.length() == 0) return;
		// ファイル名が存在する場合、ファイルを読み込み
		GetSrcGeom(width, height);
		if (width <= 0 || height <= 0) return;
		// surface の設定
		if (surface_num == 0 && ( (zoom > 0 && zoom != 256) || rotate > 0)) {
			ZoomRotate();
		} else {
			// 普通に surface を設定
			string path(name);
			path += ".g00";
			picture->SetSurface(path.c_str(), 0, 0);
			picture->SetSurfaceRect(Rect(0,0,width,height));
		}
		if (attr & SATURATE)
			picture->SetSurfaceAttribute(PicBase::BLIT_SATURATE);
	} else if (gtype == MOJI) { // テキスト描画
		if (print_moji.length() == 0) return;
		UpdateMoji();
	} else if (gtype == DIGIT) { // 数値を画像表示
		UpdateDigit();
	}
}
void GrpObj::ZoomRotate(void) {
	picture->SetSurface( (Surface*)0,0,0);

	// 回転、縮小拡大は座標原点が画像の中心になる
	string path(name);
	path += ".g00";
	Surface* surface_orig = pic_parent->Root().NewSurface(path.c_str());
	if (surface_orig == 0) return;

	Surface* zoom_surface = pic_parent->Root().RotZoomSurface(surface_orig, double(zoom)/256.0, rotate);
	Rect zoom_r (*zoom_surface);
	picture->SetSurface(zoom_surface, 0, 0);
	picture->SetSurfaceFreeFlag();
	//picture->Move(PosX() + - zoom_r.width()/2, PosY() + - zoom_r.height()/2);
// 中心座標がわからん・・・
	picture->Move(320 - zoom_r.width()/2, 240 - zoom_r.height()/2);
	picture->SetSurfaceRect(Rect(0, 0, zoom_r.width(), zoom_r.height()));

	pic_parent->Root().DeleteSurface(surface_orig);
}

static char* wstrchr(const char* s, unsigned int chr) {
	int ws, wc;
	while(*s != 0) {
		if (*s < 0 && s[1] != 0) {
			wc = int((unsigned char)(s[0]))*0x100 + int((unsigned char)(s[1]));
			ws = 2;
		} else {
			wc = (unsigned char)(s[0]);
			ws = 1;
		}
		if (wc == chr) return (char*)s;
		s += ws;
	}
	return 0;
}
TextStream GrpObj::ParseMoji(const char* str, int def_r ,int def_g, int def_b, int def_size) { // 制御シーケンス付き文字列をparse
	TextStream ts;
	ts.kanji_type = TextStream::sjis;
	ts.SetColor(def_r, def_g, def_b);
	char* copy_str = new char[strlen(str)+1];
	char* next_str;
	char* retptr;
	int var;

	while( (next_str = wstrchr(str, '#')) != 0) {
		int len = next_str - str;
		strncpy(copy_str, str, len);
		copy_str[len] = 0;
		ts.Add(copy_str);
		str = next_str + 1;

		switch(str[0]) {
		case '#': // separator
			str += 1;
			break;
		case 'D': case 'd': // return
			ts.AddReturn();
			str += 1;
			break;
		case 'C': case 'c': // color
			str += 1;
			var = strtol(str, &next_str,10);
			if (var == 0 && str == next_str) { // no parameter
				ts.SetColor(def_r, def_g, def_b);
			} else {
				int r,g,b; char key[1024];
				sprintf(key, "#COLOR_TABLE.%03d", var);
				if (parent_pimpl->config.GetParam(key, 3, &r, &g, &b)) { // color not found
					r = g = b = 0;
				}
				ts.SetColor(r,g,b);
				str = next_str;
			}
			break;
		case 'S': case 's': // size
			str += 1;
			var = strtol(str, &next_str, 10);
			if (var == 0 && str == next_str) { // no parameter
				ts.SetSize(1);
			} else {
				if (def_size == 0) def_size = 20;
				if (var <= 0) var = 1;
				ts.SetSize(double(var)/def_size);
			}
			break;
		case 'X': case 'x': // xpos : not supported
		case 'Y': case 'y': // ypos : not supported
		default:
			ts.Add("#");
			break;
		}
	}
	ts.Add(str);
	return ts;
}
void GrpObj::UpdateMoji(void) { // 文字の大きさ、色などを変更
	if (print_moji.length() == 0) return;
	if (pic_parent == 0) return;
	/* テキストの大きさを得る */
	int r, g, b;
	if (print_r == -1 || print_g == -1 || print_b == -1) {// 色設定なし
		r = g = b = 0;  // とりあえず黒(clannad のSave/Loadメニュー用)
	} else {
		r = print_r;
		g = print_g;
		b = print_b;
	}
	TextStream ts = ParseMoji(print_moji.c_str(), r, g, b, print_size);
	TextGlyphStream gs;
	vector<int> lh;
	// とりあえず drawable width は充分に大きく(2048)取る
	DefaultLayout(print_size-2)->Layout(ts, gs, lh, 2048); // print_size そのままだと弱干大きすぎるので -2
	int width = gs.width();
	int height = gs.height();
	Surface* surface = pic_parent->Root().NewSurface(width, height, ALPHA_MASK);
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);
	DSurfaceRenderText(gs.begin(), gs.end(), Rect(0, 0, width, height), surface, Rect(0,0));
	picture->SetSurface(surface, 0, 0);
	picture->SetSurfaceRect(Rect(0,0,width,height));
	picture->SetSurfaceFreeFlag();
}
void GrpObj::UpdateDigit(void) {
	// 画像表示の数値文字列を表示する
	if (name.length() == 0) return;
	// ファイル名が存在する場合、ファイルを読み込み
	string path(name);
	path += ".g00";
	Surface* surface_orig = pic_parent->Root().NewSurface(path.c_str());
	if (surface_orig == 0) return;

	int width, height;
	int i;
	GetSrcGeom(width, height);
	if (width <= 0 || height <= 0) return;
	if (src_pos.size() < 14) {
		// 必要な数の object がない
		// 表示できない分の空の rect を追加しておく
		for (i=src_pos.size(); i<14; i++)
			src_pos.push_back(Rect(0,0,0,0));
		pic_parent->Root().DeleteSurface(surface_orig);
		return;
	}
	// 桁数の計算
	char num_str[20];
	if (dig_number < 0) sprintf(num_str, "%d", -dig_number);
	else sprintf(num_str, "%d", dig_number);
	int sign_count = 0;
	int space_count = 0;
	int total_count;
	int dig_count = strlen(num_str);
	if (dig_number < 0 && (attr&DIG_SIGN) == 0) dig_count++;
	if (dig_count < dig_digit) space_count = dig_digit - dig_count;
	if (attr & DIG_SIGN) sign_count = 1;
	total_count = dig_count + space_count + sign_count;

	Surface* surface = pic_parent->Root().NewSurface(width*total_count, height, ALPHA_MASK);
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0, 0);

	/* surface にコピーする */
	int cur_x = 0;
	if ( (attr & DIG_PACK) && !(attr & DIG_ZERO)) { // 始めに空白を挿入
		cur_x += space_count * width;
	}
	int plus = 10, minus = 11, plusminus = 12;
	if (dig_number < 0) {
		DSurfaceMove(surface, src_pos[minus], surface, Rect(cur_x,0));
		cur_x += width;
	} else if (attr & DIG_SIGN) {
		if (dig_number == 0)
			DSurfaceMove(surface, src_pos[plusminus], surface, Rect(cur_x,0));
		else
			DSurfaceMove(surface, src_pos[plus], surface, Rect(cur_x,0));
		cur_x += width;
	}
	if (attr & DIG_ZERO) { // ゼロ・パディング
		for (i=0; i<space_count; i++) {
			DSurfaceMove(surface, src_pos[0], surface, Rect(cur_x, 0));
			cur_x += width;;
		}
	} else if (!(attr & DIG_PACK)) { // PACK オプションなし
		cur_x += space_count * width;
	}
	for (i=0; num_str[i] != 0; i++) {
		DSurfaceMove(surface_orig, src_pos[num_str[i]-'0'], surface, Rect(cur_x, 0));
		cur_x += width;
	}
	
	/* picture に設定 */
	picture->SetSurface(surface, 0, 0);
	picture->SetSurfaceRect(Rect(0,0,width*total_count,height));
	picture->SetSurfaceFreeFlag();

	pic_parent->Root().DeleteSurface(surface_orig);
}
void GrpObj::CreateGan(Event::Container& event, int event_number) {
	if (picture == 0) {
		fprintf(stderr,"GrpObj::CreateGan() is called before Create()\n");
		return;
	}
	if (anm) {
		anm->Abort();
		delete anm;
	}
	if (gan_name.empty()) return;
	/* アニーメション情報 (.GAN ファイル)を求める */
	string path(gan_name);
	path += ".gan";
	ARCINFO* info = file_searcher.Find(FILESEARCH::GAN, path.c_str(), "gan");
	if (info == 0) {
		fprintf(stderr,"GrpObj::CreateGan: Cannot Find 'GAN' file %s\n", path.c_str());
		return;
	}
	const char* data = info->Read();
	if (read_little_endian_int(data) != 10000 || read_little_endian_int(data+4) != 10000) {
		fprintf(stderr,"GrpObj::CreateGan: Invalid'GAN' file %s\n", path.c_str());
		delete info;
		return;
	}

	picture->SetSurfaceAttribute(PicBase::BLIT_SATURATE);
	attr = Attribute(attr | UPDATE_POS | SATURATE);

	const char* buf = data + 16;
	buf += strlen(buf) + 1; // 画像ファイル名が入っている
	buf += 4; // 定数 20000
	int pics = read_little_endian_int(buf); buf += 4; // 複数のアニメーション情報が入っている場合、情報数 
	// 以下、pics 回繰り返し
	// アニメーションを行う実体を作成
	AnmAlphaMove* wid = new AnmAlphaMove(event, picture);

	if (event_number && event_number < pics) { // 複数のアニメーション情報がある場合、先の情報を読み飛ばす */
		int i; for (i=0; i<event_number; i++) {
			buf += 4; // 定数 30000
			int ptns = read_little_endian_int(buf); buf += 4;
			buf += ptns*52;
		}
	}
	buf += 4; // 定数 30000
	int ptns = read_little_endian_int(buf); buf += 4;
	int total_time = 0;
	int i;
	for (i=0; i<ptns; i++) {
		int p = read_little_endian_int(buf+i*52+0*8+4);
		int x = read_little_endian_int(buf+i*52+1*8+4);
		int y = read_little_endian_int(buf+i*52+2*8+4);
		int t = read_little_endian_int(buf+i*52+3*8+4);
		int a = read_little_endian_int(buf+i*52+4*8+4);
		x += PosX();
		y += PosY();
		if (p == -1) { a = 0; p = 0; } // p == -1 ならなにも表示しない
		if (p >= src_pos.size()) {
			fprintf(stderr,"Reading GAN file %s (G00 %s) : not enough pictures in .G00 file\n", path.c_str(), name.c_str());
			a = 0; p = 0;
		}
		total_time += t;
		wid->ptns.push_back(AnmAlphaMove::Ptn(Rect(x,y), src_pos[p], a, total_time));
	}
	wid->SetPtn(); // パターン登録終了
	attr = Attribute(attr | ANM_PLAYSTART);
	anm = wid;
};
void GrpObj::CreateGanSpecial(Event::Container& event, int event_number, int time) {
	if (picture == 0) {
		fprintf(stderr,"GrpObj::CreateGan() is called before Create()\n");
		return;
	}
	if (anm) {
		anm->Abort();
		delete anm;
	}

	// アニメーションを行う実体を作成
	AnmAlphaMove* wid = new AnmAlphaMove(event, picture);

	int i;
	switch(event_number) {
	case 0: // pattern を 0 から最後まで変化させる
		for (i=0; i<src_pos.size(); i++) {
			wid->ptns.push_back(AnmAlphaMove::Ptn(Rect(PosX(), PosY()), src_pos[i], 255, time*i));
		}
		wid->SetPtn(); // パターン登録終了
		anm = wid;
		attr = Attribute(attr | ANM_PLAYSTART);
		break;
	default:
		break;
	}
	return;
};

void GrpObj::SetZoomRotate(int new_zoom, int new_rotate) {
	if (zoom == new_zoom && rotate == new_rotate) return;
	if (new_zoom != -1) zoom = new_zoom;
	if (new_rotate != -1) rotate = new_rotate;
	if (zoom < 0) zoom = 256;
	if (rotate < 0) rotate = 0;
	else if (rotate > 360) rotate %= 360;

	attr = Attribute(attr | UPDATE_PICTURE);
	return;
}
/******************************************************************
**
**	class ScnGrp*
*/
/* Princess Bride: 背景画の一部のみ移動、の実装 */
struct ScnGrpMove : public WidAnmTime {
	Surface* dest;
	Surface* src;
	PicRoot& root;
	Rect dest_r, from, to;
	ScnGrpMove(Event::Container& container, PicBase* _pic, PicRoot& root, Surface* dest, const Rect& _dest_r, Surface* src, const Rect& from, const Rect& to, int total_time);
	void Exec(int count);
};
ScnGrpMove::ScnGrpMove(Event::Container& container, PicBase* _pic, PicRoot& _root, Surface* _dest, const Rect& _dest_r, Surface* _src, const Rect& _from, const Rect& _to, int total_time) :
	WidAnmTime(container, _pic, total_time),
	dest(_dest), src(_src), root(_root),dest_r(_dest_r), from(_from), to(_to) {
	int dx = to.lx - from.lx;
	int dy = to.ty - from.ty;
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	if (dx < dy) dx = dy;
	if (dx == 0) dx = 1;
	SetAllCount(dx);
}
void ScnGrpMove::Exec(int count) {
	Rect r(0,0,dest_r.width(),dest_r.height());
	int dx = to.lx - from.lx;
	int dy = to.ty - from.ty;
	int x = dx*count/all_count + from.lx;
	int y = dy*count/all_count + from.ty;
	r.rmove(x, y);
	root.BlitSurface(src, r, dest, dest_r);
	iterator it;
	for (it=pic.begin(); it!=pic.end(); it++)
		(*it)->SetSurface(dest, 0, 0);
}

/* Princess Bride: カードがおちるアニメーション */

struct ScnGrpAnmAtom {
	string name;
	int time;
	ScnGrpAnmAtom(const char* _n, int _t) : name(_n), time(_t) {}
};
struct ScnGrpAnm : public WidAnmTime, vector<ScnGrpAnmAtom> {
	GrpImpl& owner;
	ScnGrpAnm(Event::Container& container, PicBase* _pic, GrpImpl& _owner) :
		WidAnmTime(container, _pic, 0), owner(_owner) {
	}
	void CalcTotal(void);
	void Exec(int count);
};
void ScnGrpAnm::CalcTotal(void) {
	/* total time を計算 */
	if (empty()) return;
	int tm = 0;
	vector<ScnGrpAnmAtom>::iterator it;
	for (it=begin(); it != end(); it++) tm += it->time;
	total_time = tm;
	SetAllCount(tm);
}
void ScnGrpAnm::Exec(int count) {
	int tm = 0; vector<ScnGrpAnmAtom>::iterator it;
	for (it=begin(); it != end(); it++) {
		tm += it->time;
		if (count < tm) break;
	}
	if (it == end()) it--;
	owner.LoadSurface(it->name.c_str(), 0);
}


/*****************************************************
*
*  GrpImpl(implementation) : 定義
*
*/

#include"music2/music.h"

GrpImpl::GrpImpl(Event::Container& _event, PicContainer& _parent, const Flags& f, set<int>& _cgm_data, class MuSys& _mu, AyuSysConfig& _config) :
	event(_event),
	flags(f), 
	parent(_parent),
	status(NORMAL),
	skip_mode(SKIP_NO),
	music(_mu),
	cgm_data(_cgm_data),
	grpobj(this),
	bs_obj(this),
	config(_config)
{
	int i;
	for (i=0; i<MAXPDT; i++) {
		ssurface[i] = 0;
		dsurface[i] = 0;
	}
	screen = parent.create_leaf(Rect(0, 0, parent.Width(), parent.Height()), 0);
	screen_front = parent.create_leaf(Rect(0, 0, parent.Width(), parent.Height()), 0);
	surface = parent.Root().NewSurface(parent.Width(), parent.Height(), NO_MASK);
	surface_update = parent.Root().NewSurface(parent.Width(), parent.Height(), NO_MASK);
	DSurfaceFill(surface, Rect(*surface), 0, 0, 0);
	DSurfaceFill(surface_update, Rect(*surface), 0, 0, 0);
	screen->SetSurface(surface, 0, 0);
	screen->show();
	screen_front->hide();
	screen_front->ZMove(screen);

	LoadCgm(config);

	anm1 = 0; anm2 = 0;
}

GrpImpl::~GrpImpl() {

	map<int,GrpObj>::iterator it;
	for (it=grpobj.begin(); it!=grpobj.end(); it++) {
		PicBase* p = it->second.DeletePic();
		delete p;
	}

	delete screen;
	delete screen_front;
	parent.Root().DeleteSurface(surface);
	int i;
	for (i=0; i<MAXPDT; i++) {
		if (ssurface[i]) parent.Root().DeleteSurface(ssurface[i]);
		if (dsurface[i]) parent.Root().DeleteSurface(dsurface[i]);
	}
}

Surface* GrpImpl::Dsurface(int pdt) {
	if (pdt == 0) return surface;
	if (dsurface[pdt] == 0) { // とりあえず画面の大きさということにする
		if (pdt == WORKPDT)
			dsurface[pdt] = parent.Root().NewSurface(parent.Width(), parent.Height(), ALPHA_MASK);
		else
			dsurface[pdt] = parent.Root().NewSurface(parent.Width(), parent.Height(), NO_MASK);
	}
	if (ssurface[pdt]) { // ssurface が存在すれば、dsurface にコピーして返す
		DSurfaceMove(ssurface[pdt], Rect(*ssurface[pdt]), dsurface[pdt], Rect(0,0));
		parent.Root().DeleteSurface(ssurface[pdt]);
		ssurface[pdt] = 0;
	}
	return dsurface[pdt];
}
#include<SDL.h>
Surface* GrpImpl::Ssurface(int pdt) {
	if (pdt == 0) return surface;
	if (ssurface[pdt]) {
		return ssurface[pdt];
	}
	return Dsurface(pdt);
}

void GrpImpl::LoadSurface(const char* str, int pdt) {
	string s = str;
	if (cgm_info.find(s) != cgm_info.end()) {
		cgm_data.insert(cgm_info[s]);
	}
	Surface* bg = parent.Root().NewSurface(s.c_str());
	if (bg == 0) {
		s += ".g00";
		bg = parent.Root().NewSurface(s.c_str());
	}
	if (bg) {
		if (ssurface[pdt]) parent.Root().DeleteSurface(ssurface[pdt]);
		ssurface[pdt] = bg;
		if (pdt == 0) {
			/* とりあえず Princess Bride のアニメーション効果専用 */
			Rect r(*ssurface[0]);
			Rect dr(*surface);
			int x = (dr.width()-r.width())/2;
			int y = (dr.height()-r.height())/2;
			DSurfaceMove(ssurface[0], r, surface, Rect(x,y));
			parent.Root().DeleteSurface(ssurface[0]);
			ssurface[0] = 0;
			screen->SetSurface(surface, 0, 0);
		}
	} else {
		if (str[0] != 0)
			fprintf(stderr,"Cannot find surface %d <- '%s'\n",pdt,str);
	}
	return;
}
void GrpImpl::InitSel(AyuSysConfig& config) {
	int i; int args[16]; char key[1024];
	for (i=0; i<999; i++) {
		sprintf(key, "#SEL.%03d",i);
		if (config.GetParam(key, 15, &args[0], &args[1],
			&args[2], &args[3], &args[4], &args[5], &args[6], &args[7],
			&args[8], &args[9], &args[10], &args[11], &args[12], &args[13],
			&args[14])) {

			sprintf(key, "#SELR.%03d", i);
			if (config.GetParam(key, 16, &args[0], &args[1],
				&args[2], &args[3], &args[4], &args[5], &args[6], &args[7],
				&args[8], &args[9], &args[10], &args[11], &args[12], &args[13],
				&args[14], &args[15]))  continue;
		}
		SEL& s = anmtype[i];
		s.from = Rect(args[0], args[1], args[2]+1, args[3]+1);
		s.to = Rect(args[4], args[5]);
		s.time = args[6];
		s.sel_no = args[7];
		int j; for (j=0; j<8; j++) s.args[j] = args[8+j];
	}
	return;
}
void GrpImpl::SetSkipMode(SkipMode _mode) {
	if ( (skip_mode & SKIP_IN_MENU) && (_mode & SKIP_IN_MENU) == 0) {
		RefreshObj();
	} else if ( (skip_mode & SKIP_IN_MENU) == 0 && (_mode & SKIP_IN_MENU) ) {
	}
	skip_mode = _mode;
}
void GrpImpl::SetObjChanged(int num) {
	changed_obj.insert(num);
}
void GrpImpl::SetObjChangedGroup(int num) {
	if (num % 1000 != 0) {
		SetObjChanged(num);
		return;
	}
	std::map<int, GrpObj>::iterator begin,end,it;
	begin = grpobj.lower_bound(num);
	end = grpobj.lower_bound(num+1000);
	for (it=begin;it!=end;it++) {
		changed_obj.insert(it->first);
	}
}
void GrpImpl::RefreshObj(void) {
	if (!deleted_pic.empty()) {
		vector<PicBase*>::iterator it;
		for (it=deleted_pic.begin(); it!=deleted_pic.end(); it++) {
			if (*it) delete *it;
		}
		deleted_pic.clear();
	}
	if (!changed_obj.empty()) {
		set<int>::iterator it;
		for (it=changed_obj.begin(); it != changed_obj.end(); it++) {
			if (grpobj.find(*it) == grpobj.end()) continue;
			GrpObj& obj = grpobj[*it];
			GrpObj& parent_obj = grpobj[ ((*it)/1000) * 1000];
			if (obj.picture == 0) continue;
			if (obj.alpha == 0 || (obj.attr & GrpObj::HIDDEN) || (parent_obj.attr & GrpObj::HIDDEN_GROUP) ) {
				if (obj.attr & GrpObj::ANM_PLAYING) {
					obj.attr = GrpObj::Attribute(obj.attr & ~(GrpObj::ANM_PLAYING));
					if (obj.anm) obj.anm->Abort();
				}
				obj.picture->hide();
			} else {
				obj.Update();
				obj.picture->show();
			}
		}
		changed_obj.clear();
	}
	if (reserved_load_surface0.length() != 0) {
		LoadSurface(reserved_load_surface0.c_str(), 0);
		reserved_load_surface0 = "";
	}
	screen->ReBlit();
}


#include<SDL.h>
void GrpImpl::StartAnm(int type) {
	SEL sel;

	if (anmtype.find(type) == anmtype.end()) {
		if (anmtype.find(0) == anmtype.end()) {
			sel.sel_no = 1;
			sel.from = Rect(*surface);
			sel.to = Rect(0,0);
			sel.time = 0;
		} else {
			sel = anmtype[0];
		}
	} else {
		sel = anmtype[type];
	}
	if (anm1) {
		fprintf(stderr,"Warning: StartAnm() called before anm1 finished\n");
		anm1->Abort();
		delete anm1;
		anm1 = 0;
	}
	map<int,GrpObj>::iterator it;
	// 現在表示中のobjectを消去
	deleted_pic.push_back(screen);
	for (it=grpobj.begin(); it!=grpobj.end(); it++) {
		if (! (it->second.attr & GrpObj::WIPEON)) { // 画像切り替え時に object 削除
			deleted_pic.push_back(it->second.DeletePic());
		} else {
			GrpObj& new_obj = bs_obj[it->first];
			if (new_obj.name.empty()) { // 新しい object が存在しなければ内容を引き継ぐ
				new_obj = it->second;
				it->second.DeletePic();
			} else {
				new_obj.attr = GrpObj::Attribute(new_obj.attr | GrpObj::WIPEON);
				deleted_pic.push_back(it->second.DeletePic());
			}
		}
	}
	grpobj.clear(); // 全オブジェクト削除

	// 全画像オブジェクトの前にscreen 移動
	// 新しい screen_front を作成しておく
	screen = screen_front;
	screen->hide();
	screen->SetSurface(surface_update, 0, 0);
	parent.Root().BlitSurface(Dsurface(1), Rect(*surface_update), surface_update, Rect(0,0));

	screen_front = parent.create_leaf(Rect(0, 0, parent.Width(), parent.Height()), 0);
	screen_front->hide();
	screen_front->ZMove(screen);
	
	// 新しい object へ更新、surface_update へ新しい object を表示
	// (object 作成時は picture は hide されている)
	for (it=bs_obj.begin(); it!=bs_obj.end(); it++) {
		grpobj[it->first] = it->second;
		it->second.DeletePic();
		CreateObj(it->first);
		GrpObj& g = grpobj[it->first];
		GrpObj& parent_obj = grpobj[ (it->first/1000) * 1000];
		if (g.picture) {
			g.Update();
			if (g.alpha == 0 || (g.attr & GrpObj::HIDDEN) || (parent_obj.attr & GrpObj::HIDDEN_GROUP) ) ;
			else g.picture->SimpleBlit(surface_update);
			g.picture->hide();
		}
	}
	bs_obj.clear();
	// 画像効果開始
	switch(sel.sel_no) {
		default:
		case 0: case 50: // 0 と 50 の違いが良くわからない
			if (skip_mode & SKIP_GRP_NOEFFEC)
				anm1 = new WidAnmAlpha(event, screen, ALPHA_MAX, ALPHA_MAX, 0);
			else if (skip_mode & SKIP_GRP_FAST)
				anm1 = new WidAnmAlpha(event, screen, 0, ALPHA_MAX, sel.time/4);
			else
				anm1 = new WidAnmAlpha(event, screen, 0, ALPHA_MAX, sel.time);
			break;
	}
	if (anm1) anm1->Play();
	if (skip_mode & SKIP_GRP_NOEFFEC) AbortAnm();
}
void GrpImpl::StartShake(int total, const int* pattern) {
	if (anm2) {
		fprintf(stderr,"Warning: StartShake() called before another animation finished\n");
		anm2->Abort();
		delete anm2;
		anm2 = 0;
	}
	if (skip_mode & SKIP_GRP_NOEFFEC) return;
	AnmAlphaMove* new_anm = new AnmAlphaMove(event, &parent); // shake screen では元画面の座標を揺らす
	int i; int tm = 0;
	for (i=0; i<total; i+=3) {
		int x = pattern[i];
		int y = pattern[i+1];
		new_anm->ptns.push_back(AnmAlphaMove::Ptn(Rect(x,y), Rect(0,0), 255, tm));
		tm += pattern[i+2];
	}
	new_anm->ptns.push_back(AnmAlphaMove::Ptn(Rect(0,0), Rect(0,0), 255, tm));
	new_anm->SetPtn(); // パターン登録終了
	new_anm->Play();
	anm2 = new_anm;
}
void GrpImpl::AbortAnm(void) {
	if (anm1 == 0) return;
	anm1->Abort();
	delete anm1;
	anm1 = 0;
	/* 画像効果終了 */
	/* 古い画面への画像効果があれば消去 */
	if (anm2 && anm2->pic[0] != screen) {
		anm2->Abort();
		delete anm2;
		anm2 = 0;
	}
	/* pdt1 -> pdt0 へコピー */
	DSurfaceMove(dsurface[1], Rect(*dsurface[1]), surface, Rect(0,0));
	screen->SetSurface(surface, 0, 0);
	// 画像効果開始時に存在したobjectを消去
	// 新しい object 表示
	RefreshObj();
	return;
}
void GrpImpl::LoadSurface(const char* str) {
	if (anm1) AbortAnm(); // 前の描画が終わってなければ強制終了
	LoadSurface(str, 1);
	bg_name = str;
}
void GrpImpl::LoadSurface(void) {
	if (anm1) AbortAnm(); // 前の描画が終わってなければ強制終了
	LoadSurface(bg_name.c_str(), 1);
}
void GrpImpl::AddSurface(const char* str) {
	if (anm1) AbortAnm(); // 前の描画が終わってなければ強制終了
	LoadSurface(bg_name.c_str());

	string s = str;
	Surface* front = parent.Root().NewSurface(s.c_str());
	if (front == 0) {
		s += ".g00";
		front = parent.Root().NewSurface(s.c_str());
	}
	if (front) {
		parent.Root().BlitSurface(front, Rect(*front), Dsurface(1), Rect(0,0));
		parent.Root().DeleteSurface(front);
	} else {
		fprintf(stderr,"Cannot find surface %s\n",str);
	}
}

void GrpImpl::CreateObj(int index) {
	std::map<int, GrpObj>::iterator cur = grpobj.find(index);
	if (cur == grpobj.end()) return;
	GrpObj& g = grpobj[index];
	g.CreateSurface(&parent);
	g.order = index;
	if (g.picture == 0) return; // エラー：surface が存在しない
	g.picture->hide();
	SetObjChanged(index);
	ZMoveObj(index);
	return;
}
void GrpImpl::ZMoveObj(int index) {
	std::map<int, GrpObj>::iterator cur = grpobj.find(index);
	if (cur == grpobj.end()) return;
	GrpObj& g = grpobj[index];
	if (g.picture == 0) return;
	// 自分より前に object があれば、その前に表示
	// そうでなければ screen の前に表示
	std::map<int, GrpObj>::iterator cur_backobj = grpobj.end();
	std::map<int, GrpObj>::iterator it;
	for (it = grpobj.begin(); it != grpobj.end(); it++) {
		if (it == cur) continue;
		if (it->second.picture == 0) continue;
		if (it->second.order < g.order) {
			if (cur_backobj == grpobj.end()) {
				cur_backobj = it;
			} else if (cur_backobj->second.order < it->second.order) {
				cur_backobj = it;
			}
		}
	}
	if (cur_backobj == grpobj.end()) {
		g.picture->ZMove(screen);
	} else {
		g.picture->ZMove(cur_backobj->second.picture);
	}
	return;
}
void GrpImpl::SwapObj(int index1, int index2) {
	// デフォルト値から order が変更されていた場合のみ、order は保存される
	// まずは両方のobjectをswap
	if (grpobj.find(index1) == grpobj.end()) {
		if (grpobj.find(index2) == grpobj.end()) return; // どちらの object も存在しない
		grpobj[index1] = grpobj[index2];
		if (grpobj[index1].order == index2)
			grpobj[index1].order = index1;
		grpobj[index2].DeletePic();
		grpobj.erase(index2);
		ZMoveObj(index1);
		return;
	} else if (grpobj.find(index2) == grpobj.end()) { // index2 が存在しない場合
		grpobj[index2] = grpobj[index1];
		if (grpobj[index2].order == index1)
			grpobj[index2].order = index2;
		grpobj[index1].DeletePic();
		grpobj.erase(index1);
		ZMoveObj(index2);
		return;
	} else {
		GrpObj obj = grpobj[index1];
		grpobj[index1] = grpobj[index2];
		grpobj[index2].DeletePic();
		if (grpobj[index1].order == index2)
			grpobj[index1].order = index1;
		ZMoveObj(index1);
		grpobj[index2] = obj;
		if (grpobj[index2].order == index1)
			grpobj[index2].order = index2;
		ZMoveObj(index2);
		obj.DeletePic();
	}
}

bool GrpImpl::Pressed(int x, int y, void* pointer) { // マウスクリックでキャンセル
	GrpImpl* g = (GrpImpl*)pointer;
	if (g->status == WAIT_MOVIE)
		g->music.StopMovie();
	if (g->status == WAIT_ANM)
		g->AbortAnm();
	if (g->status == WAIT_SHAKE && g->anm2 != 0) {
		delete g->anm2;
		g->anm2 = 0;
	}
	return false; // event deleted
}

/* mode.cgm の decode 用 */
static unsigned char decode_char[256] = {
	0x8b, 0xe5, 0x5d, 0xc3, 0xa1, 0xe0, 0x30, 0x44, 
	0x00, 0x85, 0xc0, 0x74, 0x09, 0x5f, 0x5e, 0x33, 
	0xc0, 0x5b, 0x8b, 0xe5, 0x5d, 0xc3, 0x8b, 0x45, 
	0x0c, 0x85, 0xc0, 0x75, 0x14, 0x8b, 0x55, 0xec, 
	0x83, 0xc2, 0x20, 0x52, 0x6a, 0x00, 0xe8, 0xf5, 
	0x28, 0x01, 0x00, 0x83, 0xc4, 0x08, 0x89, 0x45, 
	0x0c, 0x8b, 0x45, 0xe4, 0x6a, 0x00, 0x6a, 0x00, 
	0x50, 0x53, 0xff, 0x15, 0x34, 0xb1, 0x43, 0x00, 
	0x8b, 0x45, 0x10, 0x85, 0xc0, 0x74, 0x05, 0x8b, 
	0x4d, 0xec, 0x89, 0x08, 0x8a, 0x45, 0xf0, 0x84, 
	0xc0, 0x75, 0x78, 0xa1, 0xe0, 0x30, 0x44, 0x00, 
	0x8b, 0x7d, 0xe8, 0x8b, 0x75, 0x0c, 0x85, 0xc0, 
	0x75, 0x44, 0x8b, 0x1d, 0xd0, 0xb0, 0x43, 0x00, 
	0x85, 0xff, 0x76, 0x37, 0x81, 0xff, 0x00, 0x00, 
	0x04, 0x00, 0x6a, 0x00, 0x76, 0x43, 0x8b, 0x45, 
	0xf8, 0x8d, 0x55, 0xfc, 0x52, 0x68, 0x00, 0x00, 
	0x04, 0x00, 0x56, 0x50, 0xff, 0x15, 0x2c, 0xb1, 
	0x43, 0x00, 0x6a, 0x05, 0xff, 0xd3, 0xa1, 0xe0, 
	0x30, 0x44, 0x00, 0x81, 0xef, 0x00, 0x00, 0x04, 
	0x00, 0x81, 0xc6, 0x00, 0x00, 0x04, 0x00, 0x85, 
	0xc0, 0x74, 0xc5, 0x8b, 0x5d, 0xf8, 0x53, 0xe8, 
	0xf4, 0xfb, 0xff, 0xff, 0x8b, 0x45, 0x0c, 0x83, 
	0xc4, 0x04, 0x5f, 0x5e, 0x5b, 0x8b, 0xe5, 0x5d, 
	0xc3, 0x8b, 0x55, 0xf8, 0x8d, 0x4d, 0xfc, 0x51, 
	0x57, 0x56, 0x52, 0xff, 0x15, 0x2c, 0xb1, 0x43, 
	0x00, 0xeb, 0xd8, 0x8b, 0x45, 0xe8, 0x83, 0xc0, 
	0x20, 0x50, 0x6a, 0x00, 0xe8, 0x47, 0x28, 0x01, 
	0x00, 0x8b, 0x7d, 0xe8, 0x89, 0x45, 0xf4, 0x8b, 
	0xf0, 0xa1, 0xe0, 0x30, 0x44, 0x00, 0x83, 0xc4, 
	0x08, 0x85, 0xc0, 0x75, 0x56, 0x8b, 0x1d, 0xd0, 
	0xb0, 0x43, 0x00, 0x85, 0xff, 0x76, 0x49, 0x81, 
	0xff, 0x00, 0x00, 0x04, 0x00, 0x6a, 0x00, 0x76
};

void GrpImpl::LoadCgm(AyuSysConfig& config) {
	/* cgm ファイル読み込み */
	const char* fname = config.GetParaStr("#CGTABLE_FILE");
	if (fname == 0) return;
	ARCINFO* info = file_searcher.Find(FILESEARCH::ALL, fname, "");
	if (info == 0) return;
	char* data = info->CopyRead();
	int sz = info->Size();
	delete info;

	
	if ( strncmp(data, "CGTABLE", 7) != 0) return;
	int cgm_size = read_little_endian_int(data+0x10);

	int i,j;
	// xor 解除
	for (i=0;i<sz-0x20; i++) {
		data[i+0x20]^=decode_char[i&0xff];
	}
	// 展開
	int dest_size = cgm_size * 36;
	char* dest = new char[dest_size+1024];
	char* src = data + 0x28;
	char* dest_orig = dest;
	ARCINFO::Extract2k(dest,src,dest+dest_size,data+sz);
	dest = dest_orig;
	for (i=0; i<cgm_size; i++) {
		char* s = dest + i * 36;
		int n = read_little_endian_int(dest + i * 36 + 32);
		cgm_info[s] = n;
	}
	delete[] dest_orig;
}

/*****************************************************
*
*   GrpImpl :: Save, Load : セーブファイル処理
*
*/
void GrpImpl::Save(std::string& str) {
}
void GrpImpl::Load(const char* str) {
	status = NORMAL;
	if (anm1) {
		AbortAnm();
	}
	if (anm2) {
		anm2->Abort();
		delete anm2;
		anm2 = 0;
	}
	map<int,GrpObj>::iterator it;
	for (it=grpobj.begin(); it!=grpobj.end(); it++) {
		PicBase* p = it->second.DeletePic();
		delete p;
	}
	grpobj.clear();
	
	bg_name = "";
	music.StopCDROM(100);
}
void GrpImpl::SaveSys(string& save) {
	char buf[1024];
	save = "\n[Graphics]\n";
	save += "CGM_CG=";

	set<int>::iterator it;
	for (it=cgm_data.begin(); it != cgm_data.end(); it++) {
		sprintf(buf,"%d,",*it);
		save += buf;
	}
	save += "\n";
}
void GrpImpl::LoadSys(const char* save) {
	cgm_data.clear();
	save = strstr(save, "\n[Graphics]\n");

	if (save) {
		save += strlen("\n[Graphics]\n");
		do {
			if (save[0] == '[') break; // next section
			if (strncmp(save, "CGM_CG=",7) == 0) {
				save += 7;
				while(isdigit(*save)) {
					int n = atoi(save);
					cgm_data.insert(n);
					save = strchr(save, ',');
					if (save) save++;
				}
			}
			save = strchr(save, '\n');
			if (save) save++;
		} while (save);
	}
	return;
}


/*****************************************************
*
*   GrpImpl :: Wait , Exec : コマンド実行部
*
*/
static vector<int> drawn_images;
static int draw_n = 0;
extern bool grpdump_req;
bool GrpImpl::Wait(unsigned int current_time, Cmd& cmd) {
	if (grpdump_req) {
		grpdump_req = 0;
		std::map<int,GrpObj>::iterator it;
		fprintf(stderr,"front %08x(%d) / %08x(%d)\n",screen,screen->IsHidden(),screen_front,screen_front->IsHidden());
		for (it=grpobj.begin(); it != grpobj.end(); it++) {
			GrpObj& obj = it->second;
			GrpObj& parent_obj = grpobj[ ((it->first)/1000) * 1000];
			if (obj.picture) {
				if (!obj.name.empty()) {
					fprintf(stderr,"obj %06d(%08x): name %10s  pos %d,%d alpha %d (%d/%d/%d)\n",
					it->first,obj.picture,obj.name.c_str(),
					obj.PosX(),obj.PosY(),obj.alpha,obj.attr&GrpObj::HIDDEN ? 1 : 0,parent_obj.attr&GrpObj::HIDDEN_GROUP ? 1 : 0,obj.picture->IsHidden());
				} else if (!obj.print_moji.empty()) {
					fprintf(stderr,"obj %06d(%08x): name %10s  pos %d,%d alpha %d (%d/%d/%d)\n",
					it->first,obj.picture,obj.print_moji.c_str(),
					obj.PosX(),obj.PosY(),obj.alpha,obj.attr&GrpObj::HIDDEN ? 1 : 0,parent_obj.attr&GrpObj::HIDDEN_GROUP ? 1 : 0,obj.picture->IsHidden());
				} else {
					fprintf(stderr,"obj %06d(%08x): name %10s  pos %d,%d alpha %d (%d/%d/%d)\n",
					it->first,obj.picture,"<EMPTY>",
					obj.PosX(),obj.PosY(),obj.alpha,obj.attr&GrpObj::HIDDEN ? 1 : 0,parent_obj.attr&GrpObj::HIDDEN_GROUP ? 1 : 0,obj.picture->IsHidden());
				}
			}
		}
		std::list<PicBase*>::iterator it2;
		for (it2=parent.children.begin(); it2!=parent.children.end();it2++) {
			fprintf(stderr,"%08x(%d)\n",*it2,(*it2)->IsHidden());
		}
		RefreshObj();

	}
#if 0
	if (event.presscount(MOUSE_UP)) {
        	std::list<PicBase*>::iterator lit;
		draw_n++; int i=0;
		for (lit=parent.children.end(); lit!=parent.children.begin(); ) {
			lit--;
			(*lit)->hide();
			i++;
			if (i >= draw_n) break;
		}
		if (drawn_images.empty()) {
			map<int, GrpObj>::iterator it;
			for (it=grpobj.begin(); it!=grpobj.end(); it++) {
				if (it->second.picture) {
					drawn_images.push_back(it->first);
					PicBase* p = it->second.DeletePic();
					delete p;
				}
			}
		} else {
			vector<int>::iterator it;
			for (it=drawn_images.begin(); it!=drawn_images.end(); it++) {
				CreateObj(*it);
			}
			drawn_images.clear();
		}
	}
#endif
	if (status == WAIT_ANM) {
		if (anm1) {
			if (!anm1->IsEnd()) return true;
			AbortAnm();
		}
	} else if (status == WAIT_SHAKE) {
		if (anm2) {
			if (!anm2->IsEnd()) return true;
			delete anm2;
			anm2 = 0;
		}
		status = NORMAL;
	} else if (status == WAIT_SE) {
		if (music.IsStopSE()) status = NORMAL;
		return true;
	} else if (status == WAIT_MOVIE) {
		if (music.IsStopMovie()) {
			music.StopMovie();
			status = NORMAL;
			screen->ReBlit();
		}
		return true;
	}
	if (anm2) {
		if (anm2->IsEnd()) {
			delete anm2;
			anm2 = 0;
		}
	}
	return false;
}

void GrpImpl::DeleteObjPic(int num) { // object の surface のみ削除
	if (grpobj.find(num) == grpobj.end()) return;
	deleted_pic.push_back(grpobj[num].DeletePic());
}
void GrpImpl::DeleteObj(int num) {
	if (grpobj.find(num) == grpobj.end()) return;
	deleted_pic.push_back(grpobj[num].DeletePic());
	grpobj.erase(num);
}
void GrpImpl::DeleteObjRange(int num_first, int num_end) {
	std::map<int, GrpObj>::iterator begin,end,it;
	begin = grpobj.lower_bound(num_first);
	end = grpobj.lower_bound(num_end);
	for (it=begin;it!=end;it++) {
		deleted_pic.push_back(it->second.DeletePic());
	}
	grpobj.erase(begin, end);
}
void GrpImpl::Exec(Cmd& cmd) {
	if (cmd.cmd_type == CMD_TEXTEND) {
		music.StopKoe(500); // テキスト終了で声を止める
		cmd.clear();
		return;
	}
	if (cmd.cmd_type == CMD_WAITFRAMEUPDATE) {
		// wait する場合は RefreshObj() しておく
		RefreshObj();
	}
	if (cmd.cmd_type != CMD_OTHER) return;
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x1e && cmd.cmd3 == 0) {
		cmd.cmd_type = CMD_SAVECMDGRP_START; // grp stack clear
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x21) {
		if (cmd.cmd3 == 0x46) {
			const char* name = cmd.Str(cmd.args[0]);
			int pdt = cmd.args[1].value;
			eprintf("load surface %s pdt %d\n",name, pdt);
			if (pdt == 0)
				reserved_load_surface0 = name; // 画像読み込みは 01-1f:0000 まで待つ
			else if (pdt == 1)
				LoadSurface(name); // 背景絵読み込み？
			else
				LoadSurface(name, pdt);
			cmd.cmd_type = CMD_SAVECMDGRP;
		} else if (cmd.cmd3 == 0x49) {
			const char* name = cmd.Str(cmd.args[0]);
			int sel = cmd.args[1].value;
			eprintf("set background %s sel %d\n",name, sel);
			if (name[0] == '?') {
				LoadSurface();
			} else {
				LoadSurface(name);
			}
			StartAnm(sel);
			status = WAIT_ANM;
			event.RegisterGlobalPressFunc(&Pressed, (void*)this);
			if (name[0] == '?')
				cmd.cmd_type = CMD_SAVECMDGRP_ONCE;
			else
				cmd.cmd_type = CMD_SAVECMDGRP_START;
		} else if (cmd.cmd3 == 0x4b) {
			int pos = cmd.args[0].value;
			const char* name = cmd.Str(cmd.args[1]);
			int sel = cmd.args[2].value;
			eprintf("set foreground %s sel %d pos %d\n",name, sel, pos);
			AddSurface(name);
			StartAnm(sel);
			event.RegisterGlobalPressFunc(&Pressed, (void*)this);
			status = WAIT_ANM;
			cmd.cmd_type = CMD_SAVECMDGRP_ONCE;
		} else if (cmd.cmd3 == 0x4c) {
			/* 0x46 との違いがわからない */
			/* とりあえず bg として登録しないでみる */
			/* 735 / 19438 : unsupported command; 0x23 - cmd 01-21:004c:00[ 2] 
        		** "?",0
			*/
			/* arg1 = "?" arg2 = 0 */
			const char* name = cmd.Str(cmd.args[0]);
			int sel = cmd.args[1].value;
			if (name[0] == '?') {
				LoadSurface();
			} else {
				LoadSurface(name, 1);
			}
			StartAnm(sel);
			status = WAIT_ANM;
			event.RegisterGlobalPressFunc(&Pressed, (void*)this);
			if (name[0] == '?')
				cmd.cmd_type = CMD_SAVECMDGRP_ONCE;
			else
				cmd.cmd_type = CMD_SAVECMDGRP_START;
		} else if (cmd.cmd3 == 0x20) {
			// shake screen
			char key[1024];
			sprintf(key, "#SHAKE.%03d", cmd.args[0].value);
			if (config.SearchParam(key) != 2) {
				fprintf(stderr,"Cannot find shake pattern %d; use default pattern\n",cmd.args[0].value);
				strcpy(key, "#SHAKE.000"); // default key
			}
			int num; const int* pattern;
			pattern = config.GetParamArray(key, num);
			if (pattern) {
				StartShake(num, pattern);
				status = WAIT_SHAKE;
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x64 && cmd.cmd4 == 2) { // copy (KANOGI)
			int sx = cmd.args[0].value;
			int sy = cmd.args[1].value;
			int w = cmd.args[2].value - sx;
			int h = cmd.args[3].value - sy;
			Rect rect(sx, sy, sx+w, sy+h);
			int src = cmd.args[4].value;
			int dx = cmd.args[5].value;
			int dy = cmd.args[6].value;
			int dest = cmd.args[7].value;
			unsigned char alpha;
			eprintf("copy surface %d:(%d,%d) size(%d,%d) -> %d:(%d,%d)\n",src,sx,sy,w,h,dest,dx,dy);
			printf("copy surface %d:(%d,%d) size(%d,%d) -> %d:(%d,%d)\n",src,sx,sy,w,h,dest,dx,dy);
			if (src == dest) {
				DSurfaceMove(Ssurface(src), rect, Dsurface(WORKPDT), rect);
				src = WORKPDT;
			}
			parent.Root().BlitSurface(Ssurface(src), rect, Dsurface(dest), Rect(dx,dy));
			if (dest == 0) screen->ReBlit(Rect(dx,dy,dx+w,dy+h));
			cmd.clear();
		} else if (cmd.cmd3 == 0x4b1 && cmd.cmd4 == 2) {
			int x = cmd.args[0].value;
			int y = cmd.args[1].value;
			int w = cmd.args[2].value;
			int h = cmd.args[3].value;
			Rect rect(x,y,x+w,y+w);
			int pdt = cmd.args[4].value;
			int r = cmd.args[5].value;
			int g = cmd.args[6].value;
			int b = cmd.args[7].value;
			eprintf("clear %d:(%d,%d) size (%d,%d) r %d g %d b %d\n",pdt,x,y,w,h,r,g,b);
			DSurfaceFill(Dsurface(pdt), rect, r, g, b, 0xff);
			// if (pdt == 0) screen->ReBlit(rect);
			cmd.cmd_type = CMD_SAVECMDGRP;
		} else if (cmd.cmd3 == 0x4b1 && cmd.cmd4 == 3) { // alpha つきfill
			int x = cmd.args[0].value;
			int y = cmd.args[1].value;
			int w = cmd.args[2].value;
			int h = cmd.args[3].value;
			Rect rect(x,y,x+w,y+h);
			int pdt = cmd.args[4].value;
			int r = cmd.args[5].value;
			int g = cmd.args[6].value;
			int b = cmd.args[7].value;
			int a = cmd.args[8].value;
			eprintf("alpha-clear %d:(%d,%d) size (%d,%d) r %d g %d b %d a %d\n",pdt,x,y,w,h,r,g,b,a);
			if (a <= 0) ;
			else if (a >= 255) DSurfaceFill(Dsurface(pdt), rect, r, g, b);
			else {
				DSurfaceFill(Dsurface(WORKPDT), rect, r, g, b, a);
				parent.Root().BlitSurface(Dsurface(WORKPDT), rect, Dsurface(pdt), rect);
			}
			// if (pdt == 0) screen->ReBlit(rect);
			cmd.clear();
		} else if (cmd.cmd3 == 0x44c && cmd.cmd4 == 2) {
			int sx = cmd.args[0].value;
			int sy = cmd.args[1].value;
			int w = cmd.args[2].value;
			int h = cmd.args[3].value;
			int src = cmd.args[4].value;
			int dx = cmd.args[5].value;
			int dy = cmd.args[6].value;
			int dest = cmd.args[7].value;
			eprintf("copy surface %d:(%d,%d) size(%d,%d) -> %d:(%d,%d)\n",src,sx,sy,w,h,dest,dx,dy);
			parent.Root().BlitSurface(Ssurface(src), Rect(sx,sy,sx+w,sy+h), Dsurface(dest), Rect(dx,dy));
			//DSurfaceMove(Ssurface(src), Rect(sx,sy,sx+w,sy+h), Dsurface(dest), Rect(dx,dy));
			// if (dest == 0) screen->ReBlit(Rect(dx,dy,dx+w,dy+h));
			cmd.cmd_type = CMD_SAVECMDGRP;
		} else if (cmd.cmd3 == 0x44c && cmd.cmd4 == 3) { // alpha つきcopy
			int sx = cmd.args[0].value;
			int sy = cmd.args[1].value;
			int w = cmd.args[2].value;
			int h = cmd.args[3].value;
			Rect rect(sx, sy, sx+w, sy+h);
			int src = cmd.args[4].value;
			int dx = cmd.args[5].value;
			int dy = cmd.args[6].value;
			int dest = cmd.args[7].value;
			unsigned char alpha;
			if (cmd.args[8].value < 0) alpha = 0;
			else if (cmd.args[8].value > 255) alpha = 255;
			else alpha = cmd.args[8].value;
			eprintf("copy surface %d:(%d,%d) size(%d,%d) -> %d:(%d,%d)\n",src,sx,sy,w,h,dest,dx,dy);
			if (src == dest) {
				DSurfaceMove(Ssurface(src), rect, Dsurface(WORKPDT), rect);
				src = WORKPDT;
			}
			if (alpha != 0)
				parent.Root().BlitSurface(Ssurface(src), rect, &alpha, Rect(0,0,1,1), Dsurface(dest), Rect(dx,dy), 0);
			// if (dest == 0) screen->ReBlit(Rect(dx,dy,dx+w,dy+h));
			cmd.clear();
		} else if (cmd.cmd3 == 0x640 && cmd.cmd4 == 3) { // saturate mode で alpha 付き copy
			int sx = cmd.args[0].value;
			int sy = cmd.args[1].value;
			int w = cmd.args[2].value;
			int h = cmd.args[3].value;
			Rect rect(sx, sy, sx+w, sy+h);
			int src = cmd.args[4].value;
			int dx = cmd.args[5].value;
			int dy = cmd.args[6].value;
			int dest = cmd.args[7].value;
			unsigned char alpha;
			if (cmd.args[8].value < 0) alpha = 0;
			else if (cmd.args[8].value > 255) alpha = 255;
			else alpha = cmd.args[8].value;
			eprintf("copy surface w/ saturate %d:(%d,%d) size(%d,%d) -> %d:(%d,%d)\n",src,sx,sy,w,h,dest,dx,dy);
			if (src == dest) {
				DSurfaceMove(Ssurface(src), rect, Dsurface(WORKPDT), rect);
				src = WORKPDT;
			}
			if (alpha != 0) {
				// saturate mode : screen (picture) を一時的に作成
				PicBase* screen_tmp = parent.create_leaf(Rect(0, 0, parent.Width(), parent.Height()), 0);
				screen_tmp->SetSurface(Ssurface(src), 0, 0, PicBase::BLIT_SATURATE);
				screen_tmp->SetSurfaceRect(rect);
				screen_tmp->Move(dx, dy);
				screen_tmp->SetSurfaceAlpha(&alpha, Rect(0,0,1,1));
				screen_tmp->SimpleBlit(Dsurface(dest));
				delete screen_tmp;
			}
			cmd.clear();
		} else if (cmd.cmd3 == 0x196 && cmd.cmd4 == 0) {
			Rect r_from(cmd.args[0].value, cmd.args[1].value);
			Rect r_to(cmd.args[2].value, cmd.args[3].value);
			int src_pdt = cmd.args[4].value;
			Rect r(cmd.args[5].value,cmd.args[6].value,cmd.args[7].value+1,cmd.args[8].value+1);
			int tm = cmd.args[9].value;
			fprintf(stderr,"??? cmd time %d\n",tm);
			// anm1 = new ScnGrpMove(event, screen, parent.Root(), surface, r, Ssurface(2), r_from, r_to, tm);
			// status = WAIT_ANM;
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x22) {
		if (cmd.cmd3 == 0x0c30 && cmd.cmd4 == 0) { // スクロールする画像効果(Princess Bride)
			if (anm2) {
				anm2->Abort();
				delete anm2;
			}
			PicBase* pic; Surface* s;
			Rect r(cmd.args[1].value, cmd.args[2].value, cmd.args[3].value+1, cmd.args[4].value+1);
			const char* name = cmd.Str(cmd.args[5]);
			Rect sr_start(cmd.args[6].value,cmd.args[7].value);
			Rect sr_end(cmd.args[8].value,cmd.args[9].value);
			int tm = cmd.args[10].value;
			LoadSurface(name, 2); /* PDT2 に読み込み、と決め打ち */
			
			anm2 = new ScnGrpMove(event, screen, parent.Root(), Dsurface(1), r, Ssurface(2), sr_start, sr_end, tm);
			cmd.cmd_type = CMD_SAVECMDGRP;
		}
		if ( (cmd.cmd3 == 0xc1c && cmd.cmd4 == 0) || (cmd.cmd3 == 0x835 && cmd.cmd4 == 0) ) {
			// カードが落ちるアニメーション
			int i;
			ScnGrpAnm* new_anm = new ScnGrpAnm(event, screen, *this);
			if (cmd.cmd3 == 0x835) {
				AbortAnm();
				anm1 = new_anm;
				status = WAIT_ANM;
				event.RegisterGlobalPressFunc(&Pressed, (void*)this);
			} else {
 				anm2 = new_anm;
			}
			for (i=0; i<cmd.argc; i++) {
				const char* name = cmd.Str(cmd.args[i*3+1]);
				int tm = cmd.args[i*3+2].value;
				new_anm->push_back(ScnGrpAnmAtom(name,tm));
			}
			new_anm->CalcTotal();
			cmd.clear();
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 4) {
		if (cmd.cmd3 == 0xd8 || cmd.cmd3 == 0xd3 || cmd.cmd3 == 0xd2 || cmd.cmd3 == 0xd7) {
			cmd.clear(); // いつも 0xd8 / 0xd7 と組で出てくる
		}
		if (cmd.cmd3 == 0x5e0) { // 画像既視フラグを得る
			string s = cmd.Str(cmd.args[0]);
			if (cgm_info.find(s) == cgm_info.end()) {
				fprintf(stderr,"cmd 01-04:05e0 : cannot find cgm-info of '%s'\n",s.c_str());
				return;
			}
			int n = cgm_info[s];
			if (cgm_data.find(n) == cgm_data.end()) cmd.SetSysvar(0);
			else cmd.SetSysvar(1);
		}
	}
#if 1
	/* object 操作 */
	if ( (cmd.cmd1 == 1 || cmd.cmd1 == 2)  && (cmd.cmd2 == 0x3d || cmd.cmd2 == 0x3e) && (cmd.cmd3 == 0x0a || cmd.cmd3 == 0x0b || cmd.cmd3 == 0x0e)) { // clear object
		if (cmd.cmd3 == 0x0a || cmd.cmd3 == 0x0b) {
			if (cmd.cmd1 == 2 && cmd.args.size() == 2) {
				int num = cmd.args[0].value*1000 + cmd.args[1].value + 500;
				DeleteObj(num);
			} else if (cmd.args.size() == 1) { // group ごと消去
				int num_first = cmd.args[0].value * 1000;
				int num_end = num_first+1000;
				DeleteObjRange(num_first, num_end);
			}
		} else { // 0x0e
			if (cmd.cmd1 == 1 && cmd.args.size() == 2) {
				SwapObj(cmd.args[0].value * 1000, cmd.args[1].value * 1000);
			} else if (cmd.cmd1 == 2 && cmd.args.size() == 3) {
				int v1 = cmd.args[0].value*1000 + cmd.args[1].value + 500;
				int v2 = cmd.args[0].value*1000 + cmd.args[2].value + 500;
				SwapObj(v1, v2);
			}
		}
		cmd.clear();
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x3c && cmd.cmd3 == 0x01) { // ??? : CLANNAD
		cmd.clear();
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x3c && cmd.cmd3 == 0) { // ??? : KANOGI : 画像オブジェクトの削除？
		DeleteObjPic(cmd.args[0].value * 1000); // 旧ファイル名のsurfaceを削除
		GrpObj& g = grpobj[cmd.args[0].value * 1000];
		g.attr = GrpObj::Attribute(g.attr | GrpObj::HIDDEN);
		cmd.clear();
	}
	if ( (cmd.cmd1 == 1 || cmd.cmd1 == 2) && (cmd.cmd2 == 0x47 || cmd.cmd2 == 0x48|| cmd.cmd2 == 0x49 || cmd.cmd2 == 0x51 || cmd.cmd2 == 0x52 || cmd.cmd2 == 0x54)) {
		if (cmd.cmd1 == 1) {
			if (cmd.args.size() >= 1) cmd.args[0].value *= 1000; // とりあえず 1000倍しておく
		} else { // cmd.cmd2 == 2
			// オブジェクト番号を指定するコマンド引数が一つ増えているのを消去
                        vector<VarInfo> args = cmd.args;
			cmd.args.clear();
			if (args.size() >= 2) {
				cmd.args.push_back(args[0].value*1000 + args[1].value + 500);
				cmd.args.insert(cmd.args.end(), args.begin()+2, args.end());
			}
		}
		if (cmd.cmd2 == 0x47) {
			/**************:
				0x47 : オブジェクト内容の設定
					0x3e8: G00 ファイル
					0x3eb: GAN ファイル
					0x44c: 矩形領域
					0x4b0: 文字列
					0x514: 天候効果
					0x578: 数字の画像表示
			*/
			int base_argc = 0;
			DeleteObjPic(cmd.args[0].value); // 旧ファイル名のsurfaceを削除
			GrpObj& g = grpobj[cmd.args[0].value];
			if (cmd.cmd3 == 0x3e8) { /* ファイル名設定 */
				g.gtype = GrpObj::FILE;
				string name = cmd.Str(cmd.args[1]);
				if (name.find('?') != -1) {
					name.erase(name.find('?')); // '?' 以降の意味がわからない
				}
				g.name = name;
			} else if (cmd.cmd3 == 0x3eb) { /* ファイル名設定(GAN含む) */
				g.gtype = GrpObj::GAN;
				if (cmd.Str(cmd.args[1]) == string("???"))
					g.name = cmd.Str(cmd.args[2]);
				else
					g.name = cmd.Str(cmd.args[1]);
				g.gan_name = cmd.Str(cmd.args[2]);
			} else if (cmd.cmd3 == 0x4b0) { // 画像を文字列として指定
				g.gtype = GrpObj::MOJI;
				g.print_moji = cmd.Str(cmd.args[1]);
				g.attr = GrpObj::Attribute(g.attr & (~GrpObj::HIDDEN)); // 常に表示がデフォルト？
				cmd.clear();
			} else if (cmd.cmd3 == 0x578) { // 数値を画像として表示
				g.gtype = GrpObj::DIGIT;
				g.name = cmd.Str(cmd.args[1]);
			}
			CreateObj(cmd.args[0].value);
			if (cmd.cmd3 == 0x3e8 || cmd.cmd3 == 0x3eb || cmd.cmd3 == 0x4b0 || cmd.cmd3 == 0x578) {
				// FILE, GAN, MOJI, DIGIT ならば座標等の設定を行う
				if (cmd.cmd4 >= 1+base_argc) {
					if (cmd.args[2+base_argc].value == 0) {
						if (cmd.cmd1 == 1)
							g.attr = GrpObj::Attribute(g.attr | GrpObj::HIDDEN | GrpObj::HIDDEN_GROUP);
						else
							g.attr = GrpObj::Attribute(g.attr | GrpObj::HIDDEN);
					} else {
						if (cmd.cmd1 == 1)
							g.attr = GrpObj::Attribute(g.attr & (~(GrpObj::HIDDEN | GrpObj::HIDDEN_GROUP)));
						else
							g.attr = GrpObj::Attribute(g.attr & (~GrpObj::HIDDEN));
					}
					if (cmd.cmd1 == 1)
						SetObjChangedGroup(cmd.args[0].value);
				}
				if (cmd.cmd4 >= 2+base_argc) { // 座標等も設定
					g.SetPos(0,cmd.args[3+base_argc].value, cmd.args[4+base_argc].value);
				}
				if ( (cmd.cmd3 == 0x3e8 || cmd.cmd3 == 0x3eb) && cmd.cmd4 >= 4+base_argc) { // pattern 番号も設定
					g.SetSurfaceNum(cmd.args[5+base_argc].value);
					base_argc++; // 0x3e8 (FILE) / 0x3eb (GAN) の場合のみこのオプションは存在する
				}
				cmd.clear();
			} else {
				fprintf(stderr,"CreateObj : cmd.cmd3 = %04x ; not supported!\n",cmd.cmd3);
			}
		} else if (cmd.cmd2 == 0x48) {
			// 画面切り替え後の object (back screen object) 設定
			if (cmd.cmd3 == 0x3e8) {
				// cmd.cmd4 == 0 : args = 2, CLANNAD : cg mode
				// cmd.cmd4 == 1 : args = 3, CLANNAD :  春原回想？のところで画面が黒くなってしまうので、とりあえず。
				// cmd.cmd4 == 2 : args = 5, KANOGI : Fore Graphics
				// cmd.cmd4 == 3 : args = 6, KANOGI : CG mode
				GrpObj& g = bs_obj[cmd.args[0].value];
				string name = cmd.Str(cmd.args[1]);
				if (name.find('?') != -1) {
					name.erase(name.find('?')); // '?' 以降の意味がわからない
				}
				g.gtype = GrpObj::FILE;
				g.name = name;
				if (cmd.cmd4 >= 1 && cmd.args[2].value == 0)
					g.attr =  GrpObj::Attribute(g.attr | GrpObj::HIDDEN);
				else
					g.attr =  GrpObj::Attribute(g.attr & ~(GrpObj::HIDDEN));
				if (cmd.cmd4 >= 2)
					g.SetPos(0,cmd.args[3].value, cmd.args[4].value);
				if (cmd.cmd4 >= 3)
					g.SetSurfaceNum(cmd.args[5].value);
				if (cmd.cmd4 <= 3)
					cmd.cmd_type = CMD_SAVECMDGRP;
			}
		} else if (cmd.cmd2 == 0x49) {
			if (cmd.cmd3 == 0) {  // アニメーションを強制終了
				GrpObj& g = grpobj[cmd.args[0].value];
				if (g.anm == 0 || g.anm->IsEnd()) ;
				else g.anm->Abort();
			} else if (cmd.cmd3 == 3) { // アニメーション中か？
				GrpObj& g = grpobj[cmd.args[0].value];
				if (g.anm == 0 || g.anm->IsEnd()) {
					cmd.SetSysvar(0);
				} else {
					cmd.SetSysvar(1);
				}
			} else if (cmd.cmd3 == 1000) {
				// アニメーションを途中で停止した状態にする
				GrpObj& g = grpobj[cmd.args[0].value];
				if (g.anm == 0 || g.anm->IsEnd()) {
					// fprintf(stderr,"AnimPause : no animation in %d (%d)\n",cmd.args[0].value, cmd.args[1].value);
					g.SetSurfaceNum(cmd.args[1].value);
				} else {
					g.anm->Abort();
					g.SetSurfaceNum(cmd.args[1].value);
				}
				SetObjChanged(cmd.args[0].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x7d3) { // surface を増加させる画像効果
				GrpObj& g = grpobj[cmd.args[0].value];
				g.CreateGanSpecial(event, 0, cmd.args[1].value);
				// g.attr = GrpObj::Attribute(g.attr & (~GrpObj::HIDDEN));
				SetObjChanged(cmd.args[0].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0xbbd || cmd.cmd3 == 0xbbb || cmd.cmd3 == 0xbb9) { // アニメーション開始
				GrpObj& g = grpobj[cmd.args[0].value];
				g.CreateGan(event, cmd.args[1].value);
				// g.attr = GrpObj::Attribute(g.attr & (~GrpObj::HIDDEN));
				SetObjChanged(cmd.args[0].value);
				cmd.clear();
			}
		} else if (cmd.cmd2 == 0x51 || cmd.cmd2 == 0x52) {
			GrpObj& g = (cmd.cmd2 == 0x51) ?
				grpobj[cmd.args[0].value] :
				bs_obj[cmd.args[0].value];
			if (cmd.cmd3 == 0x3e8) { /* 座標設定 */
				g.SetPos(0,cmd.args[1].value, cmd.args[2].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x3e9 || cmd.cmd3 == 0x3ea) { /* x / y 座標のみ設定 */
				int x0, y0;
				g.GetPos(0, x0, y0);
				if (cmd.cmd3 == 0x3e9)
					g.SetPos(0,cmd.args[1].value, y0);
				else
					g.SetPos(0,x0, cmd.args[1].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x3eb) { /* alpha */
				g.SetAlpha(cmd.args[1].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x3ec) { /* visible flag */
				if (cmd.cmd1 == 1) {
					if (cmd.args[1].value) g.attr = GrpObj::Attribute(g.attr & (~(GrpObj::HIDDEN | GrpObj::HIDDEN_GROUP)));
					else g.attr = GrpObj::Attribute(g.attr | GrpObj::HIDDEN | GrpObj::HIDDEN_GROUP);
				} else {
					if (cmd.args[1].value) g.attr = GrpObj::Attribute(g.attr & (~GrpObj::HIDDEN));
					else g.attr = GrpObj::Attribute(g.attr | GrpObj::HIDDEN);
				}
				g.attr = GrpObj::Attribute(g.attr | GrpObj::UPDATE_VISIBLE);
 				// グループ単位で次の RefreshObj で表示・消去
				if (cmd.cmd2 == 0x51 && cmd.cmd1 == 1)
					SetObjChangedGroup(cmd.args[0].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x3ee || cmd.cmd3 == 0x7d6) { /* 座標設定その２？ */
					/* 0x7d6 : 画像側の基準座標を args[4,5] に入れているのかもしれない */
				int index = cmd.args[1].value;
				int x = cmd.args[2].value;
				int y = cmd.args[3].value;
				g.SetPos(index+1, x, y);
				cmd.clear();
			} else if (cmd.cmd3 == 0x3f8) { // 画像を文字列として設定：色の設定
				g.print_r = cmd.args[1].value;
				g.print_g = cmd.args[2].value;
				g.print_b = cmd.args[3].value;
				g.SetUpdate();
				// grpobj[cmd.args[0].value].print_a = cmd.args[4].value;
				/* args:229,18,minus-1,0,99,255,-1 */
				/* args:102,26,minus-1,0,99,0,255 */
				cmd.clear();
			} else if (cmd.cmd3 == 0x3fd) { // centering mode などを設定？
				if (cmd.args[1].value == 1) {
					g.attr = GrpObj::Attribute(g.attr | GrpObj::SATURATE);
					cmd.clear();
				} else if (cmd.args[1].value == 0) {
					g.attr = GrpObj::Attribute(g.attr & (~GrpObj::SATURATE));
					cmd.clear();
				}
				g.SetUpdate();
/* cmd3 == 0x41c : ゲームの進行とともに
        args:10,105
        args:10,133
        args:10,144
        args:10,144
 と変化

  cmd3 == 0x418 :
	args: 10, 400 -> 100
	と、alpha の増加とともに変化
*/
/*
487 / 8047 : unsupported command; 0x23 - cmd 01-51:0419:00[ 2] 
        81,-40,
	第二引数の 1/10 がオブジェクトの回転角
*/
			} else if (cmd.cmd3 == 0x400) { // 画像を文字列として指定
				g.print_moji = cmd.Str(cmd.args[1]);
				g.SetUpdate();
				cmd.clear();
			} else if (cmd.cmd3 == 0x401) { // 画像を文字列として設定：文字の大きさなど
				/* args: size, xspace, yspace, vertical, color, shadow */
				/*
        			args:17, 0,0,-1,  0,-1 DT in Tomoyo
        			args:17, 0,0,-1,200,-1 Save/Load in Tomoyo
        			args:20, 0,0, 0,255,-1 "──ありがとう…。" (勝平Ed付近)
        			args:16,-1,0,99,255,-1 "やあ、久しぶり──…。" (同上,Save/Load Menu)
        			args:26,-1,0,99,  0,255 Kuma in CLANNAD
				*/
				g.print_size = cmd.args[1].value;
				/* 前景色を得る */
				int cr,cg,cb; char key[1024];
				sprintf(key, "#COLOR_TABLE.%03d", cmd.args[5].value);
				if (config.GetParam(key, 3, &cr, &cg, &cb)) { // color not found
					cr = cg = cb = 0;
				}
				g.print_r = cr;
				g.print_g = cg;
				g.print_b = cb;
				g.SetUpdate();
				cmd.clear();
			} else if (cmd.cmd3 == 0x408) { // set order
				int order = cmd.args[1].value;
				if (cmd.cmd1 == 1) order *= 1000; // order も 1000 倍する必要がある？
				g.order = order;
				ZMoveObj(cmd.args[0].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x40a && cmd.cmd4 == 1) { // set surface geometry?
				// オブジェクトのどの部分を画面に表示するか（クリップ領域）の設定
				int rx = cmd.args[1].value;
				int ry = cmd.args[2].value;
				g.SetClipArea(cmd.args[1].value, cmd.args[2].value, cmd.args[3].value,cmd.args[4].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x40d) { // set digit number
				g.dig_number = cmd.args[1].value;
				g.SetUpdate();
				cmd.clear();
			} else if (cmd.cmd3 == 0x40e) { // set digit option
				g.dig_digit = cmd.args[1].value;
				int attr = g.attr;
				attr &= ~(GrpObj::DIG_ZERO | GrpObj::DIG_SIGN | GrpObj::DIG_PACK);
				if (cmd.args[2].value) attr |= GrpObj::DIG_ZERO;
				if (cmd.args[3].value) attr |= GrpObj::DIG_SIGN;
				if (cmd.args[4].value) attr |= GrpObj::DIG_PACK;
				g.attr = GrpObj::Attribute(attr);
				g.SetUpdate();
				cmd.clear();
			} else if (cmd.cmd3 == 0x40f) { /* set surface number */
				g.SetSurfaceNum(cmd.args[1].value);
				cmd.clear();
			} else if (cmd.cmd3 == 0x416) { // オブジェクトの拡大率設定
				int zoom = (cmd.args[1].value + cmd.args[2].value)/2; // x,y 別に設定できるらしい
				zoom = zoom*256/100;
				g.SetZoomRotate(zoom, -1);
				cmd.clear();
			} else if (cmd.cmd3 == 0x419) { // オブジェクトの回転設定
				int angle = cmd.args[1].value;
				angle /= 10;
				if (angle < 0) {
					angle %= 360;
					angle += 360;
				}
				angle %= 360;
				g.SetZoomRotate(-1, angle);
				cmd.clear();
			}
			if (cmd.cmd2 == 0x51 && (g.attr & GrpObj::UPDATE_ALL)) {
				SetObjChanged(cmd.args[0].value);
			}
		} else if (cmd.cmd2 == 0x54) {
			/* 座標取得 */
			if (cmd.cmd3 == 0x3e8) {
				GrpObj& obj = grpobj[cmd.args[0].value];
				VarInfo arg1 = cmd.args[1];
				VarInfo arg2 = cmd.args[2];
				int x0, y0;
				obj.GetPos(0,x0, y0);
				cmd.SetFlagvar(arg1, x0);
				cmd.SetFlagvar(arg2, y0);
			} else if (cmd.cmd3 == 0x44c) {
				int w, h;
				GrpObj& obj = grpobj[cmd.args[0].value];
				obj.GetSrcGeom(w, h);
				VarInfo arg1 = cmd.args[1];
				VarInfo arg2 = cmd.args[2];
				cmd.SetFlagvar(arg1, w);
				cmd.SetFlagvar(arg2, h);
			}
		}
		// セーブ用にコマンドを元に戻す
		if (cmd.args.size() != 0 && (cmd.cmd_type == CMD_SAVECMDGRP || cmd.cmd_type == CMD_SAVECMDGRP_ONCE)) {
			if (cmd.cmd1 == 1) cmd.args[0].value /= 1000;
			else if (cmd.cmd1 == 2) {
				vector<VarInfo> args = cmd.args;
				int value = args[0].value;
				cmd.args.clear();
				args[0].value = value / 1000;
				cmd.args.push_back(args[0]);
				args[0].value = value % 1000 - 500;
				cmd.args.push_back(args[0]);
				cmd.args.insert(cmd.args.end(), args.begin()+1, args.end());
			}
		}
	}
#endif
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x1f) { // 本来は grpstack clear らしい
		RefreshObj();
		// Princess Bride の中途 Staff roll
		// このタイミングで描画するのが都合がいいので、
		//シナリオループを抜けて描画を起動
		cmd.cmd_type = CMD_WAITFRAMEUPDATE;
	}

/* XXX : GiGiGi */
/* 122 :  0x23 - cmd 01-04:0924:00[ 0] : V<sys> にBGM再生モードを返す (0/1)
** 256 :  0x23 - cmd 01-04:091a:00[ 0] : V<sys> にBGM音量を返す (0-255?)
** 278 :  0x23 - cmd 01-04:0926:00[ 0] : V<sys> にEff再生モードを返す (0/1)
** 412 :  0x23 - cmd 01-04:091c:00[ 0] : V<sys> にEff音量を返す (0-255?)
** 434 :  0x23 - cmd 01-04:0927:00[ 0] : V<sys> にSE 再生モードを返す (0/1)
** 568 :  0x23 - cmd 01-04:091d:00[ 0] : V<sys> にSE 音量を返す (0-255?)

** 122 :  0x23 - cmd 01-04:08c0:00[ 0] : V<sys> にBGM再生モードを設定 (0/1)
** 256 :  0x23 - cmd 01-04:08b6:00[ 0] : V<sys> にBGM音量を設定 (0-255?)
** 278 :  0x23 - cmd 01-04:08c2:00[ 0] : V<sys> にEff再生モードを設定 (0/1)
** 412 :  0x23 - cmd 01-04:08b8:00[ 0] : V<sys> にEff音量を設定 (0-255?)
** 434 :  0x23 - cmd 01-04:08c3:00[ 0] : V<sys> にSE 再生モードを設定 (0/1)
** 568 :  0x23 - cmd 01-04:08b9:00[ 0] : V<sys> にSE 音量を設定 (0-255?)
*/
	// 本来は音楽関連のコマンド
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x14) {
		if (cmd.cmd3 == 0 && cmd.cmd4 == 0) {
			eprintf("play bgm %s\n",cmd.Str(cmd.args[0]));
			music.PlayCDROM( (char*)cmd.Str(cmd.args[0]), 10000);
			cmd.cmd_type = CMD_SAVECMD_ONCE;
		}
		if (cmd.cmd3 == 2 && (cmd.cmd4 == 2 || cmd.cmd4 == 0) ) { /* ??? : ことみシナリオラストの音楽再生 */
			eprintf("play bgm %s\n",cmd.Str(cmd.args[0]));
			music.PlayCDROM( (char*)cmd.Str(cmd.args[0]), 1);
			cmd.cmd_type = CMD_SAVECMD_ONCE;
		}
		if (cmd.cmd3 == 0 && cmd.cmd4 == 2) {
			eprintf("fade bgm %d? and play bgm %s; %d\n",cmd.args[1].value, cmd.Str(cmd.args[0]), cmd.args[2].value);
			// music.PlayCDROM( (char*)cmd.Str(cmd.args[0]), 10000, cmd.args[2].value);
			music.PlayCDROM( (char*)cmd.Str(cmd.args[0]), 10000);
			cmd.cmd_type = CMD_SAVECMD_ONCE;
		}
		if ( (cmd.cmd3 == 5 || cmd.cmd3 == 0x69) && cmd.cmd4 == 0) {
			if (cmd.cmd3 == 5) {
				music.StopCDROM(0);
				eprintf("stop bgm\n");
			} else {
				music.StopCDROM(cmd.args[0].value);
				eprintf("fade bgm %d\n",cmd.args[0].value);
			}
			cmd.cmd_type = CMD_SAVECMD_ONCE;
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x15) {
		if ((cmd.cmd3 == 2) || (cmd.cmd3 == 0 && cmd.cmd4 == 1) || (cmd.cmd3 == 0 && cmd.cmd4 == 0)) {
			eprintf("play SE %s\n",cmd.Str(cmd.args[0]));
			if (cmd.cmd3 == 2) {
				music.PlaySE(cmd.Str(cmd.args[0]),1);
				cmd.cmd_type = CMD_SAVECMD_ONCE;
			} else {
				music.PlaySE(cmd.Str(cmd.args[0]));
				cmd.clear();
			}
		} else if (cmd.cmd3 == 5) {
			eprintf("Stop SE\n");
			music.StopSE();
			cmd.cmd_type = CMD_SAVECMD_ONCE;
		} else if (cmd.cmd3 == 0x69) {
			eprintf("Stop SE with fade %d\n",cmd.args[0].value);
			music.StopSE(cmd.args[0].value);
			cmd.cmd_type = CMD_SAVECMD_ONCE;
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 4 && (cmd.cmd3 == 0x4bb || cmd.cmd3 == 0x4bc) ) {
		// 音楽を待ってみる(絶対に違うが)。本来、04-803 に対応してなにかの終わりをwaitするっぽい(風子/智代Ed付近)
		// EnableSyscom らしいが、よくわからない (rldev)
//		if (!music.IsStopSE()) status = WAIT_SE;
		cmd.clear();
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 4 && (cmd.cmd3 == 0x8b6 || cmd.cmd3 == 0x91a) ) {
		// 音楽モードで音量を上げるためのコマンド (SetBgmVolume)
		// とりあえず未実装
		cmd.clear();
	}

	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x16) {
		if (cmd.cmd3 == 0 && cmd.cmd4 == 0) {
			eprintf("play SE %d\n",cmd.args[0].value);
			music.PlaySE(cmd.args[0].value);
			cmd.clear();
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x17) {
		if (cmd.cmd3 == 0) {
			eprintf("play koe %d",cmd.args[0].value);
			if (cmd.cmd4 == 1) {
				eprintf(", para? %d",cmd.args[1].value);
			}
			eprintf("\n");
			char buf[1024]; sprintf(buf, "%d",cmd.args[0].value);
			if ( !(skip_mode & SKIP_TEXT)) music.PlayKoe(buf);
			cmd.clear();
		}
	}
	if (cmd.cmd1 == 1 && cmd.cmd2 == 0x1a) {
		if ( (cmd.cmd3 == 0x14 || cmd.cmd3 == 1) && cmd.cmd4 == 0) {
			const char* str = cmd.Str(cmd.args[0]);
			int x = cmd.args[1].value;
			int y = cmd.args[2].value;
			int x2 = cmd.args[3].value;
			int y2 = cmd.args[4].value;
			eprintf("play movie ; name %s pos %d,%d - %d,%d\n",str,x,y,x2,y2);
			music.PlayMovie(str, x, y, x2, y2,1);
			status = WAIT_MOVIE;
			event.RegisterGlobalPressFunc(&Pressed, (void*)this);
			cmd.clear();
		}
	}
	return;
}


/********************************************************
**
**	class Grp
*/

Grp::Grp(Event::Container& _event, PicContainer& _parent, const Flags& f, set<int>& _cgm,class MuSys& mu, AyuSysConfig& config) {
	pimpl = new GrpImpl(_event, _parent, f, _cgm, mu, config);
};
Grp::~Grp() {
	delete pimpl;
}

bool Grp::Wait(unsigned int current_time, Cmd& cmd) {
	return pimpl->Wait(current_time, cmd);
}
void Grp::Exec(Cmd& cmd) {
	pimpl->Exec(cmd);
}
void Grp::SetSkipMode(SkipMode mode) {
	pimpl->SetSkipMode(mode);
}
void Grp::InitSel(AyuSysConfig& config) {
	pimpl->InitSel(config);
}
void Grp::Save(std::string& str) {
	pimpl->Save(str);
}

void Grp::Load(const char* str) {
	pimpl->Load(str);
}
void Grp::SaveSys(std::string& str) {
	pimpl->SaveSys(str);
}

void Grp::LoadSys(const char* str) {
	pimpl->LoadSys(str);
}
