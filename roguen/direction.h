#include "point.h"

#pragma once

enum directionn : unsigned char {
	North, East, South, West,
	NorthEast, SouthEast, SouthWest, NorthWest,
	Center
};
struct directioni {
	const char*	id;
};
extern point all_directions[8];
directionn	round(directionn d, directionn v);
point		to(point, directionn d);
