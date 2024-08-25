#include "areaf.h"
#include "point.h"

#pragma once

struct indexa : adat<point, 1024> {
	typedef bool(*fnallowposition)(point v);
	void	match(fnallowposition proc, bool keep);
	void	match(fnvisible proc, bool keep);
	void	match(areaf v, bool keep);
	void	select(point m, int range);
	void	select(fnallowposition allow, bool keep, int offset);
	void	shuffle();
	void	sort(point m);
};
extern indexa indecies;