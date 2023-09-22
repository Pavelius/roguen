#include "nameable.h"

#pragma once

enum magic_s : unsigned char {
	Mundane, Blessed, Cursed, Artifact,
};
struct magici : nameable {
	int			multiplier;
};
