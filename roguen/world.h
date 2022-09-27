#include "pointm.h"

#pragma once

struct worldi {
	typedef unsigned char mpt;
	static const int	mps = 16;
	mpt					data[mps][mps];
	void				clear();
	mpt					get(pointm i) const { return data[i.y][i.x]; }
	void				generate(pointm start, mpt v);
	void				set(pointm i, mpt v) { if(i) data[i.y][i.x] = v; }
	static pointm		to(pointm i, direction_s d) { return i.to(d, mps); }
};
