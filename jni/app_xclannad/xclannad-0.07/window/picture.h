#ifndef __PICTURE__
#define __PICTURE__

#include<vector>
#include<list>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

class PicBase;
class PicContainer;
class PicRoot;

class Surface;

namespace Event {
	class Video;
}

/* PicBase の内容をイベントと連動させるためのインターフェース */
class PicAnm {
public:
	typedef std::vector<PicBase*>::iterator iterator;
	std::vector<PicBase*> pic;
	PicAnm(PicBase* pic);
	PicAnm(std::vector<PicBase*> pic);
	virtual ~PicAnm();
};

class PicBase {
	friend class PicContainer;
	friend class PicWidget;

	typedef std::list<PicBase*> List;
	typedef std::list<PicBase*>::iterator iterator;

	PicContainer* parent;
	class PicWidget* widget;
	Rect rel_pos; // relative position to the parent
	Rect rel_solid_area; // solid area(not alpha-blended) to the parent
	Rect clip_area; // clip area on the parent
	bool is_hidden;
	bool is_hidden_now;
	bool is_cached;
public:
	enum { /*MOBILE=1,*/ CACHE_BACK=2, /* CACHE_SELF=4,*/ NO_PICTURE=8, SOLID = 16, SURFACE_FREE = 32, FIT_SURFACE = 64, BLIT_SATURATE = 128, BLIT_MULTIPLY = 256, ALPHA_FREE=512};
private:
	int attribute;

	PicRoot* root;
	iterator z_pos;
	int surface_x, surface_y, surface_w, surface_h;
	Surface* surface_back;
	Surface* surface_own;
	const unsigned char* surface_alpha;
	Rect surface_alpha_rect;
	int distance_root;

	void Blit(const Rect& rpos);
	void Blit(void) {
		is_cached = true;
		Blit(Rect(0, 0, rel_pos.width(), rel_pos.height()));
	}
	/*
	** rpos : relative position to the widget
	** ppos : relative position to the parent
	**	ppos = parent_pos(rpos)
	**	rpos = child_pos(ppos, parent->this_widget)
	** cpos : relative position to a child widget
	**	cpos = child_pos(rpos, a_child_widget)
	** apos : absolute position in the screen
	**	apos = QueryAbsPos(rpos);
	**	or
	**	Rect ppos = rel_pos;
	**	apos = parent->QueryAbsPos(ppos);
	**		the latter form is used for 'rel_pos',
	**		because rel_pos is defined as the relative position to the parent
	*/
	Rect QueryAbsPos(Rect& ppos); // この picture 内の rel_pos を表示するのに実際に必要な絶対座標を得る

	static Rect child_pos(Rect rpos, PicBase* child) { /* return 'cpos' */
		rpos.intersect(child->rel_pos);
		rpos.rmove( -(child->rel_pos.lx), -(child->rel_pos.ty));
		return rpos;
	}
	Rect parent_pos(Rect rpos) { /* return 'ppos' */
		rpos.rmove(rel_pos.lx, rel_pos.ty);
		rpos.intersect(rel_pos);
		return rpos;
	}
	void SetEventWidget(class PicWidget* widget);
public:
	PicBase(const Rect& rel_pos, PicContainer* parent, int attr);
	virtual ~PicBase();
	void InitRoot(PicRoot* r) { root = r;} // only called from PicRoot::PicRoot

	void ReBlit(const Rect& rpos);
	void ReBlit(void) { ReBlit(Rect(0, 0, rel_pos.width(), rel_pos.height()));}
	void ExecReBlit(const Rect& rpos);
	void SimpleBlit(Surface* screen);

	virtual void RMove(int add_x, int add_y);
	void Move(int new_rx, int new_ry);
	#define ZMOVE_TOP ((PicBase*)0xffff00ff) /* 最前面へ */
	#define ZMOVE_BOTTOM ((PicBase*)0xffff0fff) /* 最背面へ */
	void ZMove(PicBase* back); // back の前に移動(back と自分は同じ親を持つこと)

	void SetSurface(Surface* new_surface, int x, int y, int attribute = 0);
	void SetSurface(const char* new_surface, int x, int y);
	void SetSurfacePos(int x, int y);
	int SurfacePosX(void);
	int SurfacePosY(void);
	void SetSurfaceRect(const Rect& r);
	void SetSurfaceAlpha(const unsigned char* alpha, const Rect& rect);
	void SetSurfaceAlphaFile(const char* file);
	void SetSurfaceColorKey(int r, int g, int b);
	void SetSurfaceAttribute(int attribute);
	void SetSurfaceFreeFlag(bool flag=true);
	void SetClipArea(const Rect& r);

	void hide(void);
	void show_all(void);
	void show(void);

	int PosX(void) const { return rel_pos.lx;}
	int PosY(void) const { return rel_pos.ty;}
	int Width(void) const { return rel_pos.width();}
	int Height(void) const { return rel_pos.height();}
	int DistanceRoot(void) const { return distance_root; }
	bool IsHidden(void) { return is_hidden_now;}
	bool IsParent(PicBase* pic);

	std::vector<PicAnm*> anm;
	void ClearAnm(void);
};

class PicContainer : public PicBase {
	friend class PicBase;

	void BlitBack(iterator z, Rect rpos); // z より後ろの領域を描画
	void BlitFront(iterator z, Rect rpos); // z を含め、zより前の領域を描画
	void BlitChildren(Rect rpos);
	void BlitSelf(Rect rpos);
	void BlitSelf(void) {
		is_cached = true;
		BlitSelf(Rect(0, 0, rel_pos.width(), rel_pos.height()));
	}
public:
	List children;
private:

	void set_showflag(void);
	void set_nowhiddenflag(bool is_hide);
public:
	PicContainer(const Rect& rel_pos, PicContainer* parent, int attr);
	~PicContainer();
	PicBase* create_leaf(const Rect& rel_pos, int attr);
	PicContainer* create_node(const Rect& rel_pos, int attr);
	PicRoot& Root(void) { return *root;}
	void RMove(int add_x, int add_y);
};

typedef enum { NO_MASK, ALPHA_MASK, COLOR_MASK} MaskType;
struct PicRoot {
	class PicContainer* root;
private:
	class FileToSurface* ftosurface;
	struct UpdateItem {
		PicBase* pic;
		Rect rpos;
		Rect apos;
		static bool less(const UpdateItem&, const UpdateItem&);
		UpdateItem(PicBase* p, const Rect& _rpos, const Rect& _apos) : pic(p), rpos(_rpos), apos(_apos) {}
	};
	std::vector<UpdateItem> update_rects;

	friend class FileToSurface;
	void DeleteSurfaceImpl(Surface* s) const;
public:
	void Update(PicBase* pic, const Rect& rpos, const Rect& apos);
	void DeleteUpdatePic(PicBase* pic);
	void ExecUpdate(void);
	void SetWindowCaption(const char* caption);

	// Surface 操作
	Surface* NewSurfaceFromRGBAData(int w, int h, char* data, MaskType with_mask) const; // data は malloc されたものであること(SDLの内部仕様)
	Surface* NewSurface(int w, int h, MaskType with_mask) const;
	Surface* NewSurface(const char* filename, MaskType with_mask = ALPHA_MASK);
	Surface* RotZoomSurface(Surface* from, double zoom, double rotate_angle);
	void DeleteSurface(Surface* s);
	void BlitSurface(Surface* src, const Rect& src_rpos, const unsigned char* alpha, const Rect& alpha_r,  Surface* dest, const Rect& dest_rpos, int attribute) const;
	void BlitSurface(Surface* src, const Rect& src_rpos, Surface* dest, const Rect& dest_rpos) const {
		BlitSurface(src, src_rpos, 0, Rect(0,0), dest, dest_rpos, 0);
	}
	static bool with_mask(Surface* src);

	Surface* surface;
	Surface* hw_surface;
	int width, height;
	PicRoot(void);
	~PicRoot();
	PicBase* create_leaf(const Rect& apos, int attr) {
		return root->create_leaf(apos, attr);
	}
	PicContainer* create_node(const Rect& apos, int attr) {
		return root->create_node(apos, attr);
	}
};

class PicWidget {
	PicBase* pic; /* 本来継承するべきだが、遅延初期化したいので instance */
public:
	PicWidget(void);
	virtual ~PicWidget();
	void SetPic(PicBase* new_pic);
	PicBase* Pic(void);
	PicContainer* PicNode(void);
	virtual void activate(void);
	virtual void deactivate(void);
	virtual void SetRegion(const Rect& apos);
	void show(void);
	void hide(void);
	void show_all(void);
};

#endif /* PICTURE */
