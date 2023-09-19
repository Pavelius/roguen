#include "point.h"
#include "variant.h"

#pragma once

struct skilluse {
	variant				ability;
	point				position;
	unsigned short		player_id;
	unsigned			stamp;
	static skilluse*	add(variant v, point position, short unsigned player_id, unsigned stamp);
	static skilluse*	find(variant v, point position, short unsigned player_id);
};
