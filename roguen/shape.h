#include "direction.h"
#include "point.h"
#include "pointm.h"

#pragma once

struct shapei {
	const char*		id;
	const char*		content;
	point			origin;
	pointm			size;
	pointm			points[10];
	pointm			find(char sym) const;
	size_t			maximum() const { return size.x * size.y; }
	int				m2i(pointm v) const { return size.x * v.y + v.x; }
	pointm			i2m(size_t v) const { return pointm(v % size.x, v / size.x); }
	pointm			translate(pointm s, pointm m, direction_s d) const;
	static void		read(const char* url);
};