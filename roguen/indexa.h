#include "crt.h"
#include "point.h"

#pragma once

struct indexa : adat<point> {
	void	match(fnvisible proc, bool keep);
	void	select(point m, int range);
	void	shuffle();
	void	sort(point m);
};
