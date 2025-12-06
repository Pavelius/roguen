#include "anymap.h"
#include "feature.h"

#pragma once

enum areaf : unsigned char;
enum directionn : unsigned char;

typedef bool(*fnallowposition)(point v);

struct areamap {
	static const auto mps = 64;
	typedef anymap<unsigned char, mps> arrayt;
	typedef void(areamap::*fnset)(point m, int v);
	typedef bool (*fitest)(point m);
	arrayt			tiles;
	arrayt			features;
	arrayt			param;
	arrayt			feats;
	void			blockfeatures() const;
	static void		blockrange(int range);
	void			blocktiles(int v) const;
	void			blockwalls() const;
	static void		blockzero();
	static point	bordered(directionn d);
	void			change(int t1, int t2);
	void			clear();
	static void		clearpath();
	point			findfeature(unsigned char v) const;
	static directionn getdirection(point s, point d);
	static point	get(const rect& rc);
	static rect		get(const rect& rc, point offset, point minimum, point maximum);
	const featurei&	getfeature(point m) const { return bsdata<featurei>::elements[isvalid(m) ? features[m] : 0]; }
	static point	getfree(point m, short maximum, fitest test);
	int				getindex(point m, int e) const;
	directionn		getmost(const rect& rc) const;
	static point	getnext(point start, point goal);
	static point	getnextgreater(point start);
	static unsigned getpath(point start, point goal, point* result, unsigned maximum);
	static point	getpoint(const rect& rc, directionn dir);
	static point	getpoint(const rect& rc, const rect& bound, directionn dir);
	static int		getrange(point start, point target);
	void			horz(int x1, int y1, int x2, fnset proc, int v);
	bool			is(point m, areaf v) const { return (feats[m] & (1 << v)) != 0; }
	bool			isb(point m, areaf v) const { return !isvalid(m) || (feats[m] & (1 << v)) != 0; }
	bool			istile(point m, int v) const { return isvalid(m) && tiles[m] == v; }
	bool			isfree(point m) const;
	static bool		isvalid(point m) { return ((unsigned short)m.x) < ((unsigned short)mps) && ((unsigned short)m.y) < ((unsigned short)mps); }
	bool			iswall(point m) const { return bsdata<tilei>::elements[tiles[m]].iswall(); }
	bool			iswall(point m, directionn d) const;
	bool			linelos(int x0, int y0, int x1, int y1, fitest test) const;
	static void		makewave(point start_index);
	static void		makewavex();
	unsigned char*	ptr(point m) { return isvalid(m) ? &tiles[m] : 0; }
	void			remove(point m, int v) { feats[m] &= ~(1 << v); }
	void			set(rect rc, fnset proc, int v);
	void			set(rect rc, fnset proc, int v, int random_count);
	void			setactivate(point m);
	static void		setblock(point m, unsigned short v);
	void			setfeature(point m, int v) { if(isvalid(m)) features[m] = (unsigned char)v; }
	void			setflag(point m, int v) { if(isvalid(m)) feats[m] |= (1 << v); }
	void			setlos(point m, int radius, fitest test);
	void			settile(point m, int v);
	void			setreveal(point m, int floor);
	void			vert(int x1, int y1, int y2, fnset proc, int v);
};
extern point		last_index;
extern unsigned char last_map_object;
point				randomr(const rect& rc);