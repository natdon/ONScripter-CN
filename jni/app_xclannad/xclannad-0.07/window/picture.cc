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

#include<stdio.h>
#include<vector>
#include<list>
#include<algorithm>

#include"rect.h"
#include"event.h"
#include"font/font.h"
#include"font/text.h"
#include"SDL_rotozoom.h"
#include"system/file.h"

#include"picture.h"

using namespace std;

int print_blit=0;
inline void dprintf(const char* fmt, ...) {}
/* render.cc */
void DSurfaceBlitAlpha(Surface* src_o, const Rect& srcrect_o, Surface* dst_o, const Rect& dstrect_o, const unsigned char* alpha, const Rect& alpharect);
void DSurfaceBlitSaturate(Surface* src_o, const Rect& srcrect, Surface* dst_o, const Rect& dstrect, unsigned char alpha);
void DSurfaceBlitMultiply(Surface* src_o, const Rect& srcrect, Surface* dst_o, const Rect& dstrect);
void DSurfaceFill(Surface* src, const Rect& rect, int r, int g, int b, int a=0xff); // クリア
#if 0 /* DEBUG */
#include<sys/types.h>
#include<sys/time.h>
#define dprintf printf
static struct timeval tv;
void gettm(void) {
	gettimeofday(&tv, 0);
}
int calctm(void) {
	struct timeval tv2;
	gettimeofday(&tv2, 0);
	int n = (tv2.tv_sec-tv.tv_sec)*100000 + (tv2.tv_usec-tv.tv_usec)/10;
	return n;
}
#endif

/******************************************
** PicBase
*/
PicBase::PicBase(const Rect& _rel_pos, PicContainer* _parent, int _attr) :
		parent(_parent), rel_pos(_rel_pos), rel_solid_area(0,0,0,0), clip_area(0,0,0,0),
		is_hidden(true), is_hidden_now(true), is_cached(false),  attribute(_attr), surface_alpha_rect(0,0) {

	if (parent) root = parent->root;
	else root = 0;
	surface_back = 0;
	surface_own = 0;
	surface_alpha = 0;
	surface_x = 0; surface_y = 0;
	surface_w = -1; surface_h = -1;
	widget = 0;
	attribute |= NO_PICTURE;
	if ( (attribute & CACHE_BACK) && root) {
		surface_back = root->NewSurface(rel_pos.width(), rel_pos.height(), NO_MASK);
	}
	
	if (parent) {
		parent->children.push_back(this);
		z_pos = parent->children.end(); z_pos--;
		distance_root = parent->DistanceRoot() + 1;
	} else {
		distance_root = 1;
	}
}
PicBase::~PicBase() {
	ClearAnm();
	if (widget) {
		fprintf(stderr,"Warning: PicBase::~PicBase: surface is disallocated but widget is still alive.\n");
		widget->deactivate();
	}
	if (surface_back) root->DeleteSurface(surface_back);
	if (surface_own && (attribute & SURFACE_FREE)) root->DeleteSurface(surface_own);
	if (surface_alpha && (attribute & ALPHA_FREE)) delete surface_alpha;
	iterator it;
	if (parent) { // 自分を親から削除
		parent->children.remove(this);
		// root の update 情報から自分を削除
		parent->Root().DeleteUpdatePic(this);
		// 自分の領域を書き直す
		Rect old_ppos = rel_pos;
		parent->QueryAbsPos(old_ppos);
		parent->ReBlit(old_ppos);
	}
}
void PicBase::Blit(const Rect& rpos_orig) {
	// 実際に描画する領域を得る
	Rect rpos = rpos_orig;
	// 親widget上に設定されたclip area 内に描画を限定する
	if (clip_area.width() != 0) {
		Rect clip = clip_area;
		clip = child_pos(clip, this);
		rpos.intersect(clip);
	}
	Rect apos = QueryAbsPos(rpos);
	if (rpos.empty()) return;
	// 必要に応じて保存、描画
	if (attribute & CACHE_BACK) root->BlitSurface(root->surface, apos, surface_back, rpos);
	if (! (attribute & NO_PICTURE)) {
		rpos.rmove(surface_x, surface_y);
		if (surface_w >= 0 && surface_h >= 0) {
			Rect clip(surface_x, surface_y, surface_x+surface_w, surface_y+surface_h);
			rpos.intersect(clip);
		}
//if (apos.ty < 200) fprintf(stderr,"Blit: %08x : (%d,%d,%d,%d) -> (%d,%d,%d,%d)\n",surface_own,rpos_orig.lx,rpos_orig.ty,rpos_orig.rx,rpos_orig.by,apos.lx,apos.ty,apos.rx,apos.by);
		root->BlitSurface(surface_own, rpos, surface_alpha, surface_alpha_rect, root->surface, apos, attribute);
		rpos.rmove(-surface_x, -surface_y);
	} else if (parent == 0) { // 親がいないなら背景消去の責任をもつ
		DSurfaceFill(root->surface, apos, 0, 0, 0);
	}
	PicContainer* cur = dynamic_cast<PicContainer*>(this);
	if (cur && (!cur->children.empty())) {
		cur->BlitChildren(rpos);
	}
}
void PicBase::SimpleBlit(Surface* screen) {
	// 実際に描画する領域を得る
	Rect rpos(0, 0, rel_pos.width(), rel_pos.height());
	Rect apos = QueryAbsPos(rpos);
	if (rpos.empty()) return;
	rpos.rmove(surface_x, surface_y);
	if (surface_w >= 0 && surface_h >= 0) {
		Rect clip(surface_x, surface_y, surface_x+surface_w, surface_y+surface_h);
		rpos.intersect(clip);
	}
//if (apos.ty < 200) fprintf(stderr,"S-Blit: %08x : (%d,%d,%d,%d) -> (%d,%d,%d,%d)\n",surface_own,rel_pos.lx,rel_pos.ty,rel_pos.rx,rel_pos.by,apos.lx,apos.ty,apos.rx,apos.by);
	root->BlitSurface(surface_own, rpos, surface_alpha, surface_alpha_rect, screen, apos, attribute);
}

Rect PicBase::QueryAbsPos(Rect& rpos) {
	rpos.intersect(Rect(0, 0, rel_pos.width(), rel_pos.height()));
	if (parent == 0) { // root container
		return rpos;
	}
	// 親の座標に変換後、Query する
	Rect ppos = parent_pos(rpos);
	Rect apos = parent->QueryAbsPos(ppos);
	rpos = child_pos(ppos, this);
	return apos;
};
void PicBase::ReBlit(const Rect& rpos_c) {
	Rect rpos = rpos_c;
	Rect apos = QueryAbsPos(rpos);
	
	root->Update(this, rpos, apos);
}
void PicBase::ExecReBlit(const Rect& rpos_c) {
	Rect rpos = rpos_c;
	Rect abs_r = QueryAbsPos(rpos);
	Rect ppos = parent_pos(rpos);
if(print_blit) fprintf(stderr,"back.");
	if (parent) parent->BlitBack(z_pos, ppos);
if(print_blit) fprintf(stderr,"self.");
	if (!is_hidden_now) Blit(rpos);
if(print_blit) fprintf(stderr,"front.");
	if (parent) parent->BlitFront(z_pos, ppos);
if(print_blit) fprintf(stderr,"end.");
}

void PicBase::ZMove(PicBase* move_to) {
	if (parent == 0) {
		fprintf(stderr,"Warning: PicBase::ZMove is called by root.\n");
		return;
	}
	if (move_to == ZMOVE_TOP) {
		if (this == parent->children.back()) return;
	} else if (move_to == ZMOVE_BOTTOM) {
		if (this == parent->children.front()) return;
	} else if (move_to == this) {
		fprintf(stderr,"Error: PicBase::ZMove : move in front of itself!\n");
		return;
	} else if (move_to && move_to->parent != parent) {
		fprintf(stderr,"Error: PicBase::ZMove was called with a no-brother picture\n");
		return;
	}
	// move_to と zpos のうち、後ろの方の picture から書きなおす必要がある
	iterator redraw_zpos = z_pos; redraw_zpos++;
	if (move_to == ZMOVE_BOTTOM) { // 最背面へ
		parent->children.erase(z_pos);
		parent->children.push_front(this);
		z_pos = parent->children.begin();
		redraw_zpos = parent->children.begin();
	} else if (move_to == ZMOVE_TOP) { // 最前面へ
		redraw_zpos = z_pos; redraw_zpos++;
		parent->children.erase(z_pos);
		parent->children.push_back(this);
		z_pos = parent->children.end(); z_pos--;
	} else {
	int dis_to = distance(move_to->parent->children.begin(), move_to->z_pos);
	int dis_cur = distance(parent->children.begin(), z_pos);
		if (dis_cur < dis_to) redraw_zpos = move_to->z_pos;
		parent->children.erase(z_pos);
		iterator insert_pos = move_to->z_pos; insert_pos++;
		parent->children.insert(insert_pos, this);
		z_pos = move_to->z_pos; z_pos++;
	}
	if (! is_hidden_now) {
		is_cached = false;
		ReBlit();
		/* @@@ parent->Blit() と Blit() の違いが分からないので修正 06/12/02
		Rect ppos = rel_pos;
		parent->QueryAbsPos(ppos);
		parent->ReBlit(ppos);
		*/
	}
};
void PicBase::RMove(int add_x, int add_y) {
	Rect old_ppos = rel_pos;
	rel_pos.rmove(add_x, add_y);
	parent->QueryAbsPos(old_ppos);
	parent->ReBlit(old_ppos);
	ReBlit();

	if (widget) {
		Rect new_ppos = rel_pos;
		Rect new_apos = parent->QueryAbsPos(new_ppos);
		widget->SetRegion(new_apos);
	}
}
void PicBase::Move(int new_rx, int new_ry) {
	RMove(new_rx-rel_pos.lx, new_ry-rel_pos.ty);
}
void PicBase::SetEventWidget(PicWidget* new_widget) {
	widget = new_widget;
	if (widget) {
		Rect new_ppos = rel_pos;
		Rect apos = parent->QueryAbsPos(new_ppos);
		widget->SetRegion(apos);
	}
}
void PicBase::show_all(void) {
	PicContainer*  cont = dynamic_cast<PicContainer*>(this);
	if (cont && (!cont->children.empty())) cont->set_showflag();
	show();
}
bool PicBase::IsParent(PicBase* to) {
	if (parent == 0) return false;
	if (parent == to) return true;
	return parent->IsParent(to);
}
void PicBase::show(void) {
	/* 自分の親がすべて shown か？ */
	PicContainer* cur;
	for (cur = parent; cur != 0; cur = cur->parent)
		if (cur->is_hidden) break;
	if (cur) { // 親が隠れているので表示はしない
		is_hidden = false;
		is_hidden_now = true;
		return;
	}
	if (is_hidden == false) return; // すでに表示されているのでなにもしない
	if (widget) {
		widget->activate();
	}
	is_hidden = false;
	is_hidden_now = false;
	is_cached = false;
	cur = dynamic_cast<PicContainer*>(this);
	if (cur && (!cur->children.empty())) cur->set_nowhiddenflag(false);
	ReBlit();
}
void PicBase::hide(void) {
	if (is_hidden) return;
	if (widget) {
		widget->deactivate();
	}
	is_hidden = true;
	is_hidden_now = true;
	is_cached = false;
	PicContainer* cur = dynamic_cast<PicContainer*>(this);
	if (cur && (!cur->children.empty())) cur->set_nowhiddenflag(true);
	ReBlit();
}
void PicBase::SetSurfaceAlpha(const unsigned char* alpha, const Rect& alpha_r) {
	if (attribute & ALPHA_FREE) {
		if (surface_alpha) delete[] surface_alpha;
		surface_alpha = 0;
	}
	surface_alpha = alpha;
	surface_alpha_rect = alpha_r;
	if (!is_hidden) ReBlit();
}
void PicBase::SetSurfaceColorKey(int r, int g, int b) {
	surface_alpha = 0;
	surface_alpha_rect = Rect(0,0);
	attribute &= ~(BLIT_SATURATE | BLIT_MULTIPLY);
	if (surface_own) {
		int key = SDL_MapRGB( ((SDL_Surface*)surface_own)->format, r, g, b);
		key |= 0xff000000;
		SDL_SetColorKey( (SDL_Surface*)surface_own, SDL_SRCCOLORKEY, key);
	}
	if (!is_hidden) ReBlit();
}
void PicBase::SetSurfaceAlphaFile(const char* file) {

	/* ファイルを元に alpha 画像を作成する */
	/* ファイル： パルフェの 'fil' ファイル */
	ARCINFO* info = file_searcher.Find(FILESEARCH::PDT, file,"fil");
	if (info == 0) return;
	char* new_alpha = info->CopyRead();
	int alpha_size = info->Size();
	delete info;
	Rect sr(0,0); int w,h;
	if (surface_own == 0 || new_alpha == 0) {
err_ret:
		if (new_alpha) delete[] new_alpha;
		SetSurfaceAlpha(0,Rect(0,0));
		return;
	}
	sr = Rect(*surface_own);
	w = sr.width();
	h = sr.height();
	if (alpha_size < w*h) goto err_ret;
	int i,j;
	if ( ((SDL_Surface*)surface_own)->format->Amask == 0) { // mask を surface に繰り込む
		Surface* dest = root->NewSurface(w,h, ALPHA_MASK);
		for (i=0; i<h; i++) {
			char* a = new_alpha + w*i;
			char* s = (char*)((SDL_Surface*)surface_own)->pixels + ((SDL_Surface*)surface_own)->pitch*i;
			char* d = (char*)((SDL_Surface*)dest)->pixels + ((SDL_Surface*)dest)->pitch*i;
			int sbpp = ((SDL_Surface*)surface_own)->format->BytesPerPixel;
			int dbpp = ((SDL_Surface*)dest)->format->BytesPerPixel;

			for (j=0; j<w; j++) {
				int d = read_little_endian_int(s);
				d &= 0xffffff;
				if (d == 0) ;
				else if (*a == 0) d |= 0xff000000;
				else d |= (int(*a) << 24);
				s += sbpp; d += dbpp; a++;
			}
		}
		delete new_alpha;
		root->DeleteSurface(surface_own);
		surface_own = dest;
		SetSurfaceAlpha(0, Rect(0,0));
	} else { // 外部にマスク作成
		/* マスクのうち、0xff であるべき部分を画像から判別、変更する */
		for (i=0; i<h; i++) {
			char* a = new_alpha + w*i;
			char* s = (char*)((SDL_Surface*)surface_own)->pixels + ((SDL_Surface*)surface_own)->pitch*i;
			int bpp = ((SDL_Surface*)surface_own)->format->BytesPerPixel;
			for (j=0; j<w; j++) {
				if ( ((*(int*)s) & 0xffffff) == 0) *a = 0;
				else if (*a == 0) *a = 0xff;
				s += bpp; a++;
			}
		}
		SetSurfaceAlpha( (unsigned char*)new_alpha, Rect(0,0,w,h));
		attribute |= ALPHA_FREE;
	}
}
void PicBase::SetSurface(const char* filename, int x, int y) {
	Surface* s = root->NewSurface(filename);
	SetSurface(s, x, y, SURFACE_FREE);
}
void PicBase::SetSurface(Surface* new_surface, int x, int y, int new_attr) {
	if (surface_own && (attribute & SURFACE_FREE)) {
		root->DeleteSurface(surface_own);
	}
	attribute &= ~(SURFACE_FREE | BLIT_SATURATE | BLIT_MULTIPLY | NO_PICTURE | SOLID);
	attribute |= new_attr;
	surface_own = new_surface;
	surface_x = x;
	surface_y = y;
	surface_w = -1;
	surface_h = -1;

	if (attribute & FIT_SURFACE) {
		// surface の大きさに自分の大きさを変更
		parent->ReBlit(rel_pos);
		if (surface_own == 0) {
			rel_pos = Rect(rel_pos.lx, rel_pos.ty);
		} else {
			Rect r(*surface_own);
			int w = r.width(), h = r.height();
			w -= x; h -= y;
			rel_pos = Rect(rel_pos.lx, rel_pos.ty, rel_pos.lx+w, rel_pos.ty+h);
		}
	}

	rel_solid_area = Rect(0,0,0,0);
	if (! surface_own) attribute |= NO_PICTURE;
	else if (root->with_mask(surface_own) == 0) {
		attribute |= SOLID;
		rel_solid_area = rel_pos;
	}
	if (!is_hidden) ReBlit();
}
void PicBase::SetSurfacePos(int x, int y) {
	if (surface_x == x && surface_y == y && surface_w == -1 && surface_h == -1) return;
	surface_x = x; surface_y = y;
	surface_w = -1; surface_h = -1;
	if (!is_hidden_now) ReBlit();
}
int PicBase::SurfacePosX(void) {
	return surface_x;
}
int PicBase::SurfacePosY(void) {
	return surface_y;
}
void PicBase::SetSurfaceRect(const Rect& r) {
	if (surface_x == r.lx && surface_y == r.ty && surface_w == r.width() && surface_h == r.height()) return;
	surface_x = r.lx; surface_y = r.ty;
	surface_w = r.width(); surface_h = r.height();
	parent->ReBlit(rel_pos);
	rel_pos = Rect(rel_pos.lx, rel_pos.ty, rel_pos.lx+surface_w, rel_pos.ty+surface_h);
	if (widget) {
		Rect new_ppos = rel_pos;
		Rect apos = parent->QueryAbsPos(new_ppos);
		widget->SetRegion(apos);
	}
	if (!is_hidden_now) ReBlit();
}
void PicBase::SetClipArea(const Rect& r) {
	if (clip_area == r) return;
	clip_area = r;
	parent->ReBlit(rel_pos);
}

void PicBase::SetSurfaceAttribute(int new_attribute) {
	attribute &= ~(BLIT_SATURATE | BLIT_MULTIPLY);
	attribute |= new_attribute & (BLIT_SATURATE | BLIT_MULTIPLY);
	if (new_attribute & (BLIT_SATURATE | BLIT_MULTIPLY)) {
		rel_solid_area = Rect(0,0);
	}
}
void PicBase::SetSurfaceFreeFlag(bool flag) {
	if (flag) attribute |= SURFACE_FREE;
	else attribute &= ~SURFACE_FREE;

}

/******************************************
** PicContainer
*/
PicContainer::PicContainer(const Rect& rel_pos, PicContainer* parent, int attr) :
	PicBase(rel_pos, parent, attr) {
}
PicContainer::~PicContainer() {
	iterator end = children.end();
	for (iterator it = children.begin(); it != end; ) {
		iterator it_next = it; it_next++;
		if ((*it)->widget) delete (*it)->widget; // picture にwidget が付属しているなら、そちらをdelete
		else delete (*it);
		it = it_next;
	}
}
void PicContainer::BlitBack(iterator z, Rect rpos) {
	rpos.intersect(Rect(0, 0, rel_pos.width(), rel_pos.height()));
	if (rpos.empty()) return;
	iterator end = children.end(), begin = children.begin(); iterator it = begin;

	Rect ppos = parent_pos(rpos);
	if (is_hidden_now) goto parent_redraw;
	// cache されている領域を探す
	// z自身がキャッシュしていれば、ここで終了
	if ( ((*z)->attribute & CACHE_BACK) && ( (*z)->is_cached) && (*z)->rel_pos.is_inner(rpos)) {
		Rect cpos = child_pos(rpos, *z);
		Rect apos = (*z)->QueryAbsPos(cpos);
		root->BlitSurface( (*z)->surface_back, cpos, root->surface, apos);
		return;
	}
	// z より下の子がキャッシュ、あるいは SOLID 描画できないか？
	for (it = z; it != begin;) { // 子がcontainerの場合のチェックは省略
		it--;
		if ( (*it)->is_hidden_now) continue;
		if ( (*it)->rel_pos.is_crossed(rpos)) {
			if ( ((*it)->attribute & CACHE_BACK) && ((*it)->is_cached) && (*it)->rel_pos.is_inner(rpos)) {
				Rect cpos = child_pos(rpos, *it);
				Rect apos = (*it)->QueryAbsPos(cpos);
				root->BlitSurface( (*it)->surface_back, cpos, root->surface, apos);
				goto children_redraw;
			}
			if ( (*it)->rel_solid_area.is_inner(rpos)) {
				goto children_redraw;
			}
		}
	}
	// 自分自身がキャッシュ、あるいは SOLID 描画できないか？
	if (rel_solid_area.is_inner(ppos)) {
		goto self_redraw;
	}
	if ( (attribute & CACHE_BACK) && is_cached) {
		Rect cpos = child_pos(rpos, *z);
		Rect apos = (*z)->QueryAbsPos(cpos);
		Rect draw_rpos = (*z)->parent_pos(cpos);
if(print_blit) fprintf(stderr,"cahce.");
		root->BlitSurface(surface_back, draw_rpos, root->surface, apos);
		goto self_redraw;
	}
parent_redraw:
	if (parent) {
		Rect ppos = parent_pos(rpos);
if(print_blit) fprintf(stderr,"parent-back.");
		parent->BlitBack(z_pos, ppos);
	}
	if (is_hidden_now) return;
self_redraw:
if(print_blit) fprintf(stderr,"back-self.");
	BlitSelf(rpos); // 子は描画せず、自分だけ描画
children_redraw:
	for (; it != z; it++) {
		if ( (*it)->is_hidden_now) continue;
		if ( (*it)->rel_pos.is_crossed(rpos)) {
			Rect cpos = child_pos(rpos, *it);
			(*it)->Blit(cpos);
		}
	}
}
void PicContainer::BlitChildren(Rect rpos) {
if (print_blit) fprintf(stderr,"bc.");
	iterator end = children.end();
	for (iterator it = children.begin(); it != end; it++) {
if ( (*it)->is_hidden_now) if(print_blit) fprintf(stderr,"bch %08x;",*it);
		if ( (*it)->is_hidden_now) continue;
		if ( (*it)->rel_pos.is_crossed(rpos)) {
			Rect cpos = child_pos(rpos, *it);
			(*it)->Blit(cpos);
		}
	}
}
void PicContainer::BlitFront(iterator z, Rect rpos) {
	rpos.intersect(Rect(0, 0, rel_pos.width(), rel_pos.height()));
	if (rpos.empty()) return;
	iterator end = children.end(); iterator it;
	z++;
	for (it = z; it != end; it++) {
		if ( (*it)->is_hidden_now) continue;
		if ( (*it)->rel_pos.is_crossed(rpos)) {
			Rect cpos = child_pos(rpos, *it);
			(*it)->Blit(cpos);
		}
	}
	if (parent) {
		Rect ppos = parent_pos(rpos);
		parent->BlitFront(z_pos, ppos);
	}
};
void PicContainer::BlitSelf(Rect rpos) {
	// 実際に描画する領域を得る
	rpos.intersect(Rect(0, 0, rel_pos.width(), rel_pos.height()));
	if (rpos.empty()) return;
	Rect apos = QueryAbsPos(rpos);
	// 必要に応じて保存、描画
if(print_blit) fprintf(stderr,"self-back.");
	if (attribute & CACHE_BACK) root->BlitSurface(root->surface, apos, surface_back, rpos);
	if (! (attribute & NO_PICTURE)) {
		rpos.rmove(surface_x, surface_y);
		if (surface_w >= 0 && surface_h >= 0) {
			Rect clip(0, 0, surface_w, surface_h);
			clip.rmove(rpos.lx, rpos.ty);
			rpos.intersect(clip);
		}
if(print_blit) fprintf(stderr,"self-blit.");
		root->BlitSurface(surface_own, rpos, surface_alpha, surface_alpha_rect, root->surface, apos, attribute);
	} else if (parent == 0) { // 親がいないなら背景消去の責任をもつ
		DSurfaceFill(root->surface, apos, 0, 0, 0);
	}
}

void PicContainer::set_showflag(void) {
	iterator end = children.end();
	for (iterator it = children.begin(); it != end; it++) {
		(*it)->is_hidden = false;
		PicContainer* next = dynamic_cast<PicContainer*>(*it);
		if (next && (!next->children.empty())) next->set_showflag();
	}
}
void PicContainer::set_nowhiddenflag(bool is_hide) {
	iterator end = children.end();
	for (iterator it = children.begin(); it != end; it++) {
		if (is_hide) (*it)->is_hidden_now = true;
		else (*it)->is_hidden_now = (*it)->is_hidden;
		if ( (*it)->widget) {
			if ((*it)->is_hidden_now) (*it)->widget->deactivate();
			else (*it)->widget->activate();
		}
		PicContainer* next = dynamic_cast<PicContainer*>(*it);
		if (next && (!next->children.empty())) next->set_nowhiddenflag(is_hide);
	}
}
void PicContainer::RMove(int add_x, int add_y) { // event widget の移動があり得るので子についてもRMoveを呼び出す
	PicBase::RMove(add_x, add_y);
	iterator end = children.end();
	for (iterator it = children.begin(); it != end; it++) {
		(*it)->RMove(0,0);
	}
}

PicBase* PicContainer::create_leaf(const Rect& rel_pos, int attr) {
	return new PicBase(rel_pos, this, attr);
}
PicContainer* PicContainer::create_node(const Rect& rel_pos, int attr) {
	return new PicContainer(rel_pos, this, attr);
}

/***************************************************************
**
** PicWidget
*/

PicWidget::PicWidget(void) {
	pic = 0;
}
PicWidget::~PicWidget() {
	if (pic) {
		pic->SetEventWidget(0);
		delete pic;
	}
	pic = 0;
}
void PicWidget::SetPic(PicBase* new_pic) {
	if (pic) {
		pic->SetEventWidget(0);
		delete pic;
	}
	pic = new_pic;
	if (pic) pic->SetEventWidget(this);
}
PicBase* PicWidget::Pic(void) {
	if (pic == 0) {
		fprintf(stderr,"Error: PicWidget::Pic returns zero.\n");
	}
	return pic;
}
PicContainer* PicWidget::PicNode(void) {
	PicContainer* node = dynamic_cast<PicContainer*>(pic);
	if (node == 0) {
		fprintf(stderr,"Error: PicWidget::PicNode returns zero.\n");
	}
	return node;
}

/******************************************
** FileToSurface
*/

#include<list>
#include<map>
#include<string>
using namespace std;
struct SurfaceIndex {
	typedef list<SurfaceIndex*>::iterator qiterator;
	string filename;
	Surface* surface;
	qiterator qpos;
	int ref_count;
};

class FileToSurface {
	typedef list<SurfaceIndex*>::iterator qiterator;

	list<SurfaceIndex*> queue;
	map<string, SurfaceIndex*> findex;
	map<Surface*, SurfaceIndex*> mindex;
	int count;
	int count_max;
	const PicRoot& root;
	bool DeleteData(SurfaceIndex* data);
	Surface* LoadSurface(string name, char*& mem);
public:
	FileToSurface(const PicRoot& root);
	~FileToSurface(void);
	Surface* Load(string name);
	bool Free(Surface* s);
};

FileToSurface::FileToSurface(const PicRoot& _root) : root(_root) {
	count = 0;
	count_max = 32; // キャッシュ量（決め打ち）
	count_max = 3;
};
FileToSurface::~FileToSurface() {
	qiterator it;
	for (it=queue.begin(); it != queue.end(); it++) {
		if ( (*it)->ref_count) {
			fprintf(stderr, "Warning: FileToSurface: delete referenced surface named '%s'\n",(*it)->filename.c_str());
		}
		root.DeleteSurfaceImpl( (*it)->surface);
		delete *it;
	}
}
inline bool FileToSurface::DeleteData(SurfaceIndex* data) {
	if ( data->ref_count) return false;
	findex.erase(data->filename);
	mindex.erase(data->surface);
	queue.erase(data->qpos);
	root.DeleteSurfaceImpl(data->surface);
	delete data;
	count--;
	return true;
}
inline Surface* FileToSurface::LoadSurface(string name, char*& mem) {
	ARCINFO* info = file_searcher.Find(FILESEARCH::PDT, name.c_str(),"pdt");
	if (info == 0) return 0;
	GRPCONV* conv = GRPCONV::AssignConverter(info);
	if (conv == 0) { delete info;return 0;}
	mem = (char*)malloc(conv->Width() * conv->Height() * 4 + 1024);
	Surface* s = 0;
	if (conv->Read(mem)) {
		MaskType is_mask = conv->IsMask() ? ALPHA_MASK : NO_MASK;
		if (is_mask == ALPHA_MASK) { // alpha がすべて 0xff ならマスク無しとする
			int len = conv->Width()*conv->Height();
			unsigned int* d = (unsigned int*)mem;
			int i; for (i=0; i<len; i++) {
				if ( (*d&0xff000000) != 0xff000000) break;
				d++;
			}
			if (i == len) {
				is_mask = NO_MASK;
			}
		}
		s = root.NewSurfaceFromRGBAData(conv->Width(), conv->Height(), mem, is_mask);
	}
	delete conv; delete info; // delete data;
	return s;
}
Surface* FileToSurface::Load(string name) {
	if (findex.find(name) != findex.end()) {
		findex[name]->ref_count++;
		return findex[name]->surface;
	}
	char* mem;
	Surface* surface = LoadSurface(name, mem);
	if (surface == 0) return 0;

	while (count >= count_max) { // count_max 以上のデータを可能なら削除する
		qiterator it;
		for (it=queue.begin(); it != queue.end(); it++) {
			if (DeleteData(*it)) break;
		}
		if (it == queue.end()) break; // 全データが使用中なら終了
	}
	SurfaceIndex* new_index = new SurfaceIndex;
	new_index->filename = name;
	new_index->surface = surface;
	findex[name] = new_index;
	mindex[surface] = new_index;
	queue.push_back(new_index);
	new_index->qpos = queue.end(); new_index->qpos--;
	new_index->ref_count = 1;
	count++;
	return surface;
}
bool FileToSurface::Free(Surface* s) {
	if (mindex.find(s) == mindex.end()) {
		return false;
	}
	SurfaceIndex* index = mindex[s];
	if (index->ref_count == 0) DeleteData(index);
	else index->ref_count--;
	return true;
}

/******************************************
** PicRoot
*/
#include<SDL.h>

#include"surface.h"

#define DefaultRmask 0xff0000
#define DefaultGmask 0xff00
#define DefaultBmask 0xff
#define DefaultAmask 0xff000000
#define DefaultBpp 32

PicRoot::PicRoot(void) {
	hw_surface = (Surface*)SDL_GetVideoSurface();
	SDL_PixelFormat* fmt_SDL = hw_surface->format;
	if (fmt_SDL->BitsPerPixel == DefaultBpp && fmt_SDL->Rmask == DefaultRmask && fmt_SDL->Gmask == DefaultGmask && fmt_SDL->Bmask == DefaultBmask) { 
		surface = hw_surface;
	} else {
		surface = (Surface*)SDL_CreateRGBSurface(0, hw_surface->w, hw_surface->h, DefaultBpp, DefaultRmask, DefaultGmask, DefaultBmask, 0);
	}

	Rect rpos(0, 0, surface->w, surface->h);
	root = new PicContainer(rpos, 0, 0);
	root->InitRoot(this);
	root->show();
	ftosurface = new FileToSurface(*this);
	width = surface->w;
	height = surface->h;
	return;
}
PicRoot::~PicRoot() {
	// if (surface) DeleteSurfaceImpl(surface); // SDL_GetVideoSurface() した surface は開放の必要がないらしい
	surface = 0;
	delete root;
	delete ftosurface;
}
void PicRoot::Update(PicBase* pic, const Rect& rpos, const Rect& apos) {
	update_rects.push_back(UpdateItem(pic, rpos, apos));
}
bool PicRoot::UpdateItem::less(const PicRoot::UpdateItem& a, const PicRoot::UpdateItem& b) {
	return a.pic->DistanceRoot() < b.pic->DistanceRoot();
}
void PicRoot::DeleteUpdatePic(PicBase* pic) {
	vector<UpdateItem>::iterator it = update_rects.begin();
	while(it != update_rects.end()) {
		if (it->pic == pic) {
			update_rects.erase(it);
			it = update_rects.begin();
			continue;
		}
		it++;
	}
	return;
}
void PicRoot::ExecUpdate(void) {
	/* 共通する領域を消去する */
	sort(update_rects.begin(), update_rects.end(), UpdateItem::less);
	vector<UpdateItem>::iterator it;
	vector<UpdateItem>::iterator end = update_rects.end();
if(print_blit){
	fprintf(stderr,"ExecUpdate Start: \n\t");
	for (it=update_rects.begin(); it != end; it++) {
		fprintf(stderr,"(%d,%d,%d,%d), ",it->apos.lx,it->apos.ty,it->apos.rx,it->apos.by);
	}
	fprintf(stderr,"\n");
}
	for (it=update_rects.begin(); it != end; it++) {
		if (it->rpos.width() == 0) continue;

		Rect apos = it->apos;
		PicBase* pic = it->pic;

		vector<UpdateItem>::iterator jt = it; jt++;
		for (; jt != end; jt++) {
			if (apos.is_inner(jt->apos)) {
				if (jt->pic == pic || jt->pic->IsParent(pic)) { // 親が共通、かつ領域も共通
					jt->rpos = Rect(0,0); // empty rect をセット
					jt->apos = Rect(0,0);
				}
			} else if (jt->apos.is_inner(apos)) { // 相手に自分が包含される
				if (jt->pic == pic || jt->pic->IsParent(pic)) { // 親が共通、かつ領域も共通
					it->rpos = Rect(0,0);
					it->apos = Rect(0,0);
					break;
				}
			}
		}
	}
if(print_blit){
	fprintf(stderr,"->\t");
	for (it=update_rects.begin(); it != end; it++) {
		fprintf(stderr,"(%d,%d,%d,%d), ",it->apos.lx,it->apos.ty,it->apos.rx,it->apos.by);
	}
	fprintf(stderr,"\n");
}

	int num = update_rects.size();
	SDL_Rect* r = new SDL_Rect[num];
	Rect confine = Rect(0, 0, surface->w, surface->h);
	int n = 0;
	int i;
	for (i=0; i<num; i++) {
		UpdateItem& item = update_rects[i];
		Rect& ur = item.apos;
		if (ur.width() == 0) continue;
if(print_blit)fprintf(stderr,"%08x: %d,%d,%d,%d",item.pic, item.apos.lx, item.apos.ty, item.apos.rx, item.apos.by);

		item.pic->ExecReBlit(item.rpos);
if(print_blit)fprintf(stderr,"\n");
		ur.intersect(confine);
		r[n].x = ur.lx;
		r[n].y = ur.ty;
		r[n].w = ur.rx - ur.lx;
		r[n].h = ur.by - ur.ty;
		if (surface != hw_surface) SDL_BlitSurface(surface, &r[n], hw_surface, &r[n]);
		n++;
	}
if(print_blit)fprintf(stderr,"\n");
	SDL_UpdateRects(hw_surface, n, r);
	delete[] r; update_rects.clear();
}

Surface* PicRoot::NewSurface(int w, int h, MaskType with_mask) const {
	Surface* s;
	if (with_mask == ALPHA_MASK) {
		s = (Surface*)SDL_CreateRGBSurface(SDL_SRCALPHA, w, h, DefaultBpp, DefaultRmask, DefaultGmask, DefaultBmask, DefaultAmask);
	} else {
		s = (Surface*)SDL_CreateRGBSurface(0, w, h, DefaultBpp, DefaultRmask, DefaultGmask, DefaultBmask, 0);
	}
	return s;
}

Surface* PicRoot::NewSurfaceFromRGBAData(int w, int h, char* data, MaskType with_mask) const {
	int amask = (with_mask == ALPHA_MASK) ? DefaultAmask : 0;
	Surface* s = (Surface*)SDL_CreateRGBSurfaceFrom(data, w, h, DefaultBpp, w*4, DefaultRmask, DefaultGmask, DefaultBmask, amask);
	s->flags &= ~SDL_PREALLOC;
	return s;
};
Surface* PicRoot::NewSurface(const char* f, MaskType with_mask) {
	if (f == 0) return 0;
	Surface* s = ftosurface->Load(f);
	if (s == 0) return 0;
	if (with_mask == COLOR_MASK) {
		SDL_SetColorKey( (SDL_Surface*)s, SDL_SRCCOLORKEY, *(Uint32*)s->pixels);
	}
	/* xkanon の残骸 */
	if (strcmp(f, "grdat") == 0)
        	SDL_SetColorKey(s, SDL_SRCCOLORKEY, 0x55aa66);
	return s;
}
Surface* PicRoot::RotZoomSurface(Surface* from, double zoom, double rotate) {
	Surface* ret = (Surface*)rotozoomSurface( (SDL_Surface*)from, rotate, zoom, SMOOTHING_OFF);
	return ret;
}
void PicRoot::DeleteSurfaceImpl(Surface* s) const {
	SDL_FreeSurface(s);
}
void PicRoot::DeleteSurface(Surface* s) {
	if (!ftosurface->Free(s))
		DeleteSurfaceImpl(s);
}
inline SDL_Rect SDLed(const Rect& rect) {
	SDL_Rect r;
	r.x = rect.lx;
	r.y = rect.ty;
	r.w = rect.rx-rect.lx;
	r.h = rect.by-rect.ty;
	return r;
}

#ifndef ALPHA_MAX
#define ALPHA_MAX 255
#endif
void PicRoot::BlitSurface(Surface* src, const Rect& src_r, const unsigned char* alpha, const Rect& alpha_r, Surface* dest, const Rect& dest_r, int attribute) const {
if (print_blit) fprintf(stderr," s %08x %d:%d:%d:%d;",src, dest_r.lx, dest_r.ty, dest_r.rx, dest_r.by);
	SDL_Rect sr = SDLed(src_r); SDL_Rect dr = SDLed(dest_r);
special_blit:
	if (attribute & PicBase::BLIT_MULTIPLY) {
if (print_blit) fprintf(stderr,"M");
		DSurfaceBlitMultiply(src, src_r, dest, dest_r);
		return;
	} else if (attribute & PicBase::BLIT_SATURATE) {
		if (src->format->Amask != 0) goto normal_blit;
if (print_blit) fprintf(stderr,"S");
		unsigned char a = 255;
		if (alpha && alpha_r.width() >= 1 && alpha_r.height() >= 1) a = *alpha;
		DSurfaceBlitSaturate(src, src_r, dest, dest_r, a);
		return;
	}
normal_blit:
if (print_blit) fprintf(stderr,"N");
	if (alpha == 0 || alpha_r.width() == 0) { // simple blit
if (print_blit) fprintf(stderr,"X");
		SDL_BlitSurface(src, &sr, dest, &dr);
		return;
	}
	if (alpha_r.width() == 1 && alpha_r.height() == 1) {
		if (*alpha == 255) {
if (print_blit) fprintf(stderr,"Y");
			SDL_BlitSurface(src, &sr, dest, &dr);
			return;
		}
#ifdef ENABLE_PATCH
#else
		if (src->format->Amask == 0) { // use per-surface alpha
if (print_blit) fprintf(stderr,"Z");
			SDL_SetAlpha(src, SDL_SRCALPHA, *alpha);
			SDL_BlitSurface(src, &sr, dest, &dr);
			SDL_SetAlpha(src, 0, 0);
			return;
		}
#endif
	}
	// generic alpha blit
if (print_blit) fprintf(stderr,"W");
	DSurfaceBlitAlpha(src, src_r, dest, dest_r, alpha, alpha_r);
	return;
}

bool PicRoot::with_mask(Surface* s) {
	return s->format->Amask != 0;
}

#if USE_X11
#include<SDL_syswm.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#endif /* USE_X11 */
void PicRoot::SetWindowCaption(const char* caption) {
#if USE_X11
//	SDL_WM_SetCaption(caption, 0);
	// SDLの関数では2バイト文字をサポートしてくれないので、同等の内容に修正
	SDL_SysWMinfo info;
	memset(&info,0,sizeof(info));
	SDL_VERSION(&(info.version));
	if (SDL_GetWMInfo(&info) == 1) {
		Display* display = info.info.x11.display;
		Window wm = info.info.x11.wmwindow;
		if (wm == 0) wm = info.info.x11.window;
		if (wm != 0) {
			XTextProperty titleprop;
			XmbTextListToTextProperty(display, (char**)&caption, 1, XCompoundTextStyle, &titleprop);
			XSetWMName(display, wm, &titleprop);
			XSetWMIconName(display, wm, &titleprop);
			XFree(titleprop.value);
		}
		XSync(display, False);
	}
#endif /* USE_X11 */
}

/************************************************************
** PicAnm
*/

void PicBase::ClearAnm(void) {
	while(!anm.empty()) {
		delete anm.back();
	}
}
PicAnm::PicAnm(PicBase* _pic) {
	pic.push_back(_pic);
	pic[0]->anm.push_back(this);
	return;
	
}
PicAnm::PicAnm(vector<PicBase*> _pic) : pic(_pic) {
	if (pic.empty()) return;
	pic[0]->anm.push_back(this);
	return;
}
PicAnm::~PicAnm() {
	vector<PicAnm*>::iterator it = find(pic[0]->anm.begin(), pic[0]->anm.end(), this);
	if (it == pic[0]->anm.end()) {
		fprintf(stderr,"Cannot found this in PicAnm::~PicAnm()");
	} else {
		pic[0]->anm.erase(it);
	}
}
