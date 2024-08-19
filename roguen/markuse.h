#pragma once
#include "point.h"
#include "variant.h"

struct markuse {
	variant			ability;
	point			position;
	unsigned short	player_id;
	unsigned		stamp;
};

markuse* markuse_add(variant v, point position, short unsigned player_id, unsigned stamp);
markuse* markuse_find(variant v, point position, short unsigned player_id);

bool markused(variant v, point position, short unsigned player_id);

