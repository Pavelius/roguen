#include "nameable.h"

#pragma once

enum duration_s : unsigned char {
	Instant,
	Minute10, Minute20, Minute30,
	Hour1, Hour2, Hour4, Hour1PL,
	Day1,
};

struct durationi : nameable {
	int			value, per_level;
	int			get(int level) const { return value + level * per_level; }
};
