#include "color.h"
#include "framerange.h"

#pragma once

enum tile_s : unsigned char { NoTile };
enum tilef : unsigned char { Impassable, CanSwim, DangerousFeature, BetweenWalls, Woods };

struct tilefi {
	const char*		id;
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
