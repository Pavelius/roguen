#include "anymap.h"

#pragma once

struct worldi : anymap<unsigned char, 32> {
	void	clear();
	void	generate(point start, unsigned char v);
};
