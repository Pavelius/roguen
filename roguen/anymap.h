#include "point.h"

#pragma once

enum direction_s : unsigned char;

template<class T, int MPS>
struct anymap {
	typedef short unsigned indext;
	static const auto mps = MPS;
	T				data[MPS][MPS];
	constexpr T		get(point i) const { return data[i.y][i.x]; }
	constexpr static point	i2m(indext i) { return (i == 0xFFFF) ? point{-1000, -1000} : point{i % MPS, i / MPS}; }
	constexpr static indext m2i(point m) { return m ? m.y * MPS + m.x : 0xFFFF; }
	constexpr void	set(point m, T v) { if(m.x >= 0) data[m.y][m.x] = v; }
};
point to(point m, direction_s v, unsigned char mps);