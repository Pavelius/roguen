#include "anymap.h"

#pragma once

struct worldi : anymap<unsigned char, 32> {
	void	clear();
	void	generate(point start, unsigned char v);
	static bool	isvalid(point m) { return ((unsigned short)m.x) < ((unsigned short)32) && ((unsigned short)m.y) < ((unsigned short)32); }
};
