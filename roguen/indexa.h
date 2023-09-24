#include "areaf.h"
#include "crt.h"
#include "point.h"

#pragma once

struct indexa : adat<point, 1024> {
	typedef bool(*fnallow)(point v);
	void	match(fnvisible proc, bool keep);
	void	match(areaf v, bool keep);
	void	select(point m, int range);
	void	select(fnallow allow, bool keep, int offset);
	void	shuffle();
	void	sort(point m);
};
