#include "point.h"

#pragma once

typedef short unsigned indext;
const int mps = 96;
const indext Blocked = 0xFFFF;

enum direction_s : unsigned char {
	North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest
};
enum mapf_s : unsigned char {
	Explored, Visible, Searched, Blooded, Iced, Webbed,
};
enum tile_s : unsigned char {
	NoTile, WoodenFloor, Grass, GrassCorupted, Lava, Water
};
enum feature_s : unsigned char {
	NoFeature,
	Tree,
};
inline indext m2i(point v) { return v.x + v.y * mps; }
inline point i2m(indext v) { return point{(short)(v % mps), (short)(v / mps)}; }
struct areamap {
	tile_s			tiles[mps * mps];
	feature_s		features[mps * mps];
	unsigned char	random[mps * mps];
	unsigned char	flags[mps * mps];
	void			clear();
	static point	correct(point v);
	bool			is(indext i, mapf_s v) const { return (flags[i] & (1 << v)) != 0; }
	bool			isfreenw(indext i) const { return true; }
	void			set(indext i, mapf_s v) { flags[i] |= (1 << v); }
	void			set(indext i, tile_s v);
	void			set(indext i, tile_s v, short w, short h);
	void			remove(indext i, mapf_s v) { flags[i] &= ~(1 << v); }
};
struct framerange {
	unsigned char	start;
	unsigned char	count;
	int				get(int s) { return start + s % count; }
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
};
struct featurei {
	const char*		id;
	framerange		features;
};
indext				to(indext i, direction_s v);