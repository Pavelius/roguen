#include "rect.h"

#pragma once

struct point {
	short			x, y;
	constexpr explicit operator bool() const { return x || y; }
	constexpr bool	operator!=(const point pt) const { return pt.x != x || pt.y != y; }
	constexpr bool	operator==(const point pt) const { return pt.x == x && pt.y == y; }
	constexpr point	operator-() const { return{(short)-x, (short)-y}; }
	constexpr point	operator-(const point pt) const { return{(short)(x - pt.x), (short)(y - pt.y)}; }
	constexpr point	operator+(const point pt) const { return{(short)(x + pt.x), (short)(y + pt.y)}; }
	constexpr void	clear() { x = y = 0; }
	constexpr bool	in(const rect& rc) const { return x >= rc.x1 && x <= rc.x2 && y >= rc.y1 && y <= rc.y2; }
	bool			in(const point p1, const point p2, const point p3) const;
	constexpr rect	rectangle() const { return {x, y, x, y}; }
	constexpr point to(short dx, short dy) const { return {(short)(x + dx), (short)(y + dy)}; }
	int				range(point m2) const;
};
point center(const rect& rc);