#pragma once
#include "point.h"
#include "variant.h"

struct skilluse {
	variant			ability;
	point			position;
	unsigned short	player_id;
	unsigned		stamp;
};

skilluse* skilluse_add(variant v, point position, short unsigned player_id, unsigned stamp);
skilluse* skilluse_find(variant v, point position, short unsigned player_id);

