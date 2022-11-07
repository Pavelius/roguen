#include "point.h"

#pragma once

struct geoposition {
	point		position;
	short		level;
	constexpr bool operator==(const geoposition& e) const { return e.position == position && e.level == level; }
	constexpr bool operator!=(const geoposition& e) const { return e.position != position || e.level != level; }
	bool		isoutdoor() const { return level == 0; }
};
