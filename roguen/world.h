#include "anymap.h"
#include "pointm.h"

#pragma once

struct worldi : anymap<unsigned char, 32> {
	void	clear();
	void	generate(pointm start, unsigned char v);
};
