#include "anymap.h"
#include "color.h"
#include "point.h"

#pragma once

enum direction_s : unsigned char;
enum tile_s : unsigned char;
enum feature_s : unsigned char;
enum mapf_s : unsigned char {
	Explored, Visible, Activated, Searched, Blooded, Iced, Webbed,
};
enum areaf : unsigned char {
	Impassable, ImpassableNonActive, AllowActivate, BetweenWalls, Woods,
};
struct framerange {
	unsigned char	start;
	unsigned char	count;
	int				get(int s) const { return start + s % count; }
	explicit constexpr operator bool() const { return count != 0; }
};
struct areafi {
	const char*		id;
	framerange		features;
};
struct tilei {
	const char*		id;
	color			minimap;
	framerange		floor;
	framerange		decals;
	int				borders = -1;
	tile_s			tile;
	framerange		walls;
	bool			iswall() const { return tile != (tile_s)0; }
};
struct featurei {
	const char*		id;
	framerange		features, overlay;
	unsigned char	priority = 10;
	unsigned		flags = 0;
	void			paint(int random) const;
	bool			is(areaf v) const { return (flags & (1 << v)) != 0; }
};
struct areamap : anymap<tile_s, 64> {
	anymap<feature_s, mps> features;
	anymap<unsigned char, mps> random;
	anymap<unsigned char, mps> flags;
	void			blockfeatures() const;
	static void		blockrange(int range);
	void			blockwalls() const;
	static void		blockzero();
	void			clear();
	static void		clearpath();
	static direction_s getdirection(point s, point d);
	feature_s		getfeature(point m) const;
	unsigned char	getfow(point m) const;
	int				getindex(point m, tile_s e) const;
	point			getfree(point m, short maximum) const;
	static point	getnext(point start, point goal);
	static unsigned getpath(point start, point goal, point* result, unsigned maximum);
	static point	getpoint(const rect& rc, direction_s dir);
	static int		getrange(point start, point target);
	void			horz(int x1, int y1, int x2, tile_s tile);
	bool			is(point m, mapf_s v) const { return (flags[m] & (1 << v)) != 0; }
	bool			isb(point m, mapf_s v) const { return !isvalid(m) || (flags[m] & (1 << v)) != 0; }
	bool			isfree(point m) const;
	bool			isfreelt(point m) const;
	bool			iswall(point m) const;
	bool			iswall(point m, direction_s d) const;
	bool			linelos(int x0, int y0, int x1, int y1) const;
	bool			linelossv(int x0, int y0, int x1, int y1);
	void			set(point m, mapf_s v) { if(isvalid(m)) flags[m] |= (1 << v); }
	void			set(point m, tile_s v);
	void			set(point m, feature_s v);
	void			set(rect rc, tile_s v);
	void			set(rect rc, feature_s v, int random_count);
	void			set(rect rc, mapf_s v, int random_count);
	void			set(rect rc, tile_s v, int random_count);
	void			set(feature_s v, int bonus);
	void			setlos(point m, int radius);
	void			remove(point m, mapf_s v) { flags[m] &= ~(1 << v); }
	void			removechance(mapf_s v, int chance);
	static void		makewave(point start_index);
	static void		makewavex();
	void			vert(int x1, int y1, int y2, tile_s tile);
};