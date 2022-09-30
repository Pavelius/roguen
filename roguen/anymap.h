#include "point.h"

#pragma once

enum direction_s : unsigned char;

template<class T, int MPS>
struct anymap {
	static const auto mps = MPS;
	T				data[MPS][MPS];
	constexpr bool	isvalid(point m) const { return ((unsigned short)m.x) < ((unsigned short)MPS); }
	constexpr T		get(point m) const { return data[m.y][m.x]; }
	constexpr void	set(point m, T v) { if(isvalid(m)) data[m.y][m.x] = v; }
};
point to(point m, direction_s v, unsigned char mps);