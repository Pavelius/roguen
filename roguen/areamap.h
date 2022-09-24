#include "point.h"

#pragma once

typedef short unsigned indext;
const int mps = 64;
const indext Blocked = 0xFFFF;
const indext NotCalculatedMovement = 0xFFF0;

enum direction_s : unsigned char {
	North, East, South, West,
	NorthEast, SouthEast, SouthWest, NorthWest
};
enum mapf_s : unsigned char {
	Explored, Visible, Activated, Searched, Blooded, Iced, Webbed,
};
enum tile_s : unsigned char {
	NoTile, WoodenFloor, Cave, DungeonFloor, Grass, GrassCorupted, Lava, Water,
	WallCave, WallBuilding, WallDungeon, WallFire, WallIce,
};
enum feature_s : unsigned char {
	NoFeature,
	Tree, FootMud, FootHill, Grave,
	HiveHole, Hive, Hole, Plant, Herbs,
	Trap, Door,
};
enum areaf : unsigned char {
	Impassable, ImpassableNonActive, AllowActivate, BetweenWalls, Woods,
};
inline indext m2i(point v) { return v.x + v.y * mps; }
inline indext m2i(int x, int y) { return x + y * mps; }
inline point i2m(indext v) { return point{(short)(v % mps), (short)(v / mps)}; }
struct areamap {
	tile_s			tiles[mps * mps];
	feature_s		features[mps * mps];
	unsigned char	random[mps * mps];
	unsigned char	flags[mps * mps];
	static void		addwave(indext i);
	void			blockfeatures() const;
	static void		blockrange(int range);
	void			blockwalls() const;
	static void		blockzero();
	void			clear();
	static void		clearpath();
	static direction_s getdirection(point s, point d);
	feature_s		getfeature(indext i) const;
	unsigned char	getfow(indext i) const;
	int				getindex(indext i, tile_s e) const;
	static indext	getnext(indext start, indext goal);
	static unsigned getpath(indext start, indext goal, indext* result, unsigned maximum);
	static indext	getpoint(const rect& rc, direction_s dir);
	static int		getrange(indext start, indext target);
	static indext	getwave();
	void			horz(int x1, int y1, int x2, tile_s tile);
	bool			is(indext i, mapf_s v) const { return (flags[i] & (1 << v)) != 0; }
	bool			isb(indext i, mapf_s v) const { return i == Blocked || (flags[i] & (1 << v)) != 0; }
	bool			isfree(indext i) const;
	bool			isfreelt(indext i) const;
	bool			iswall(indext i, direction_s d) const;
	bool			linelos(int x0, int y0, int x1, int y1) const;
	bool			linelossv(int x0, int y0, int x1, int y1);
	void			set(indext i, mapf_s v) { flags[i] |= (1 << v); }
	void			set(indext i, tile_s v);
	void			set(indext i, feature_s v);
	void			set(rect rc, tile_s v);
	void			set(rect rc, tile_s v, int random_count);
	void			set(rect rc, feature_s v, int random_count);
	void			set(feature_s v, int bonus);
	void			setlos(indext index, int radius);
	void			remove(indext i, mapf_s v) { flags[i] &= ~(1 << v); }
	void			removechance(mapf_s v, int chance);
	static void		makewave(indext start_index);
	static void		makewavex();
	void			vert(int x1, int y1, int y2, tile_s tile);
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
	framerange		floor;
	framerange		decals;
	int				borders = -1;
	tile_s			tile;
	framerange		walls;
	bool			iswall() const { return tile != NoTile; }
};
struct featurei {
	const char*		id;
	framerange		features, overlay;
	unsigned char	priority = 10;
	unsigned		flags = 0;
	void			paint(int random) const;
	bool			is(areaf v) const { return (flags & (1 << v)) != 0; }
};
indext				to(indext d, direction_s v);
direction_s			round(direction_s i, direction_s v);