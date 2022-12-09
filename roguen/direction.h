#include "point.h"

#pragma once

enum direction_s : unsigned char {
	North, East, South, West,
	NorthEast, SouthEast, SouthWest, NorthWest,
	Center
};
struct directioni {
	const char*	id;
};
extern point all_directions[8];
direction_s	round(direction_s d, direction_s v);
point		to(point, direction_s d);
