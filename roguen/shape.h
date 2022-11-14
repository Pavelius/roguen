#include "point.h"

#pragma once

enum direction_s : unsigned char;

struct shapei {
	const char*		id;
	const char*		content;
	point			origin;
	point			size;
	point			points[10];
	char operator[](point m) const { return content[m.y * size.x + m.x]; }
	rect			bounding(point c, direction_s d) const;
	point			center(point c) const { return c + origin; }
	point			find(char sym) const;
	size_t			maximum() const { return size.x * size.y; }
	point			translate(point s, point m, direction_s d) const;
	static void		read(const char* url);
};