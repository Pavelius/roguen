#include "point.h"

bool point::in(const point p1, const point p2, const point p3) const {
	int a = (p1.x - x) * (p2.y - p1.y) - (p2.x - p1.x) * (p1.y - y);
	int b = (p2.x - x) * (p3.y - p2.y) - (p3.x - p2.x) * (p2.y - y);
	int c = (p3.x - x) * (p1.y - p3.y) - (p1.x - p3.x) * (p3.y - y);
	return (a >= 0 && b >= 0 && c >= 0)
		|| (a < 0 && b < 0 && c < 0);
}

point center(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = rc.x1 + rc.width() / 2;
	short y = rc.y1 + rc.height() / 2;
	return {x, y};
}