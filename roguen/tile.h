#pragma once

#include "color.h"
#include "framerange.h"

enum tilef : unsigned char {
	Impassable, CanSwim, DangerousFeature, BetweenWalls, Undeground,
	StuckFeature, TrappedFeature, Woods, Mines, Container,
};

struct tilefi {
	const char*		id;
};
struct tilei {
	const char*		id;
	color			minimap;
	framerange		floor, decals;
	int				borders;
	unsigned char	tile;
	framerange		walls;
	unsigned		flags;
	bool			is(tilef v) const { return (flags & (1 << v)) != 0; }
	bool			iswall() const { return tile != 0; }
};
