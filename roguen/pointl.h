#pragma once

struct pointl {
	int					x, y;
	constexpr bool		operator!=(const pointl v) const { return v.x != x || v.y != y; }
	constexpr bool		operator==(const pointl v) const { return v.x == x && v.y == y; }
	constexpr pointl operator+(const pointl& v) const { return {x + v.x, y + v.y}; }
	constexpr pointl operator-(const pointl& v) const { return {x - v.x, y - v.y}; }
};