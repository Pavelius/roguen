#include "anymap.h"
#include "color.h"
#include "crt.h"
#include "point.h"

#pragma once

enum direction_s : unsigned char;
enum tile_s : unsigned char { NoTile };
enum class featuren : unsigned char { No };
enum areaf : unsigned char { Explored, Visible, Hidden, Darkened, Blooded, Iced, Webbed };
enum tilef : unsigned char { Impassable, CanSwim, DangerousFeature, BetweenWalls, Woods };
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
	framerange		floor, decals;
	int				borders;
	tile_s			tile;
	framerange		walls;
	unsigned		flags;
	bool			is(tilef v) const { return (flags & (1 << v)) != 0; }
	bool			iswall() const { return tile != NoTile; }
};
struct tilefi {
	const char*		id;
};
struct featurei {
	const char*		id;
	framerange		features, overlay;
	unsigned char	priority;
	unsigned		flags;
	color			minimap;
	featurei		*leadto, *activateto;
	char			lead;
	operator featuren() const { return (featuren)(this - bsdata<featurei>::elements); }
	bool			is(tilef v) const { return (flags & (1 << v)) != 0; }
	bool			isvisible() const { return features.count != 0; }
	featurei*		getactivate() const { return activateto; }
	featurei*		gethidden() const;
	featurei*		getlead() const { return leadto; }
	void			paint(int random) const;
};
struct areamap : anymap<tile_s, 64> {
	typedef bool (*fntest)(point m);
	anymap<featuren, mps> features;
	anymap<unsigned char, mps> random;
	anymap<unsigned char, mps> feats;
	void			blockfeatures() const;
	static void		blockrange(int range);
	void			blocktiles(tile_s v) const;
	void			blockwalls() const;
	static void		blockzero();
	static point	bordered(direction_s d);
	void			change(tile_s t1, tile_s t2);
	void			clear();
	static void		clearpath();
	point			find(featuren v) const;
	static direction_s getdirection(point s, point d);
	static point	get(int x, int y) { return {(short)x, (short)y}; }
	static point	get(const rect& rc);
	static rect		get(const rect& rc, point offset, point minimum, point maximum);
	const featurei&	getfeature(point m) const;
	unsigned char	getfow(point m) const;
	static point	getfree(point m, short maximum, fntest test);
	int				getindex(point m, tile_s e) const;
	unsigned char	getlos(point m) const;
	direction_s		getmost(const rect& rc) const;
	static point	getnext(point start, point goal);
	static unsigned getpath(point start, point goal, point* result, unsigned maximum);
	static point	getpoint(const rect& rc, direction_s dir);
	static point	getpoint(const rect& rc, const rect& bound, direction_s dir);
	static int		getrange(point start, point target);
	void			horz(int x1, int y1, int x2, tile_s tile);
	bool			is(point m, areaf v) const { return (feats[m] & (1 << v)) != 0; }
	bool			is(point m, tile_s v) const { return isvalid(m) && (*this)[m] == v; }
	bool			isb(point m, areaf v) const { return !isvalid(m) || (feats[m] & (1 << v)) != 0; }
	bool			isfree(point m) const;
	bool			iswall(point m) const;
	bool			iswall(point m, direction_s d) const;
	bool			linelos(int x0, int y0, int x1, int y1, fntest test) const;
	bool			linelossv(int x0, int y0, int x1, int y1, fntest test);
	static void		makewave(point start_index);
	static void		makewavex();
	static int		randomcount(const rect& rc, int v);
	void			set(point m, areaf v) { if(isvalid(m)) feats[m] |= (1 << v); }
	void			set(point m, tile_s v);
	void			set(point m, featuren v);
	void			set(point m, const featurei& v);
	void			set(rect rc, tile_s v);
	void			set(rect rc, featuren v);
	void			set(rect rc, areaf v);
	void			set(rect rc, featuren v, int random_count);
	void			set(rect rc, areaf v, int random_count);
	void			set(rect rc, tile_s v, int random_count);
	void			set(featuren v, int bonus);
	void			setactivate(point m);
	static void		setblock(point m, unsigned short v);
	void			setlos(point m, int radius, fntest test);
	void			setreveal(point m, tile_s floor);
	void			remove(point m, areaf v) { feats[m] &= ~(1 << v); }
	void			removechance(areaf v, int chance);
	void			vert(int x1, int y1, int y2, tile_s tile);
};