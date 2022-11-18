#include "nameable.h"

#pragma once

enum magic_s : unsigned char {
	Mudane, Blessed, Cursed, Artifact,
};
struct magici : nameable {
	int		bonus;
};
