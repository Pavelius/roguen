#include "point.h"

#pragma once

enum direction_s : unsigned char;

template<class T, int MPS>
struct anymap {
	static const auto mps = MPS;
	T				data[MPS][MPS];
	constexpr bool	isvalid(point m) const { return ((unsigned short)m.x) < ((unsigned short)MPS) && ((unsigned short)m.y) < ((unsigned short)MPS); }
	constexpr T& operator[](point m) { return data[m.y][m.x]; }
	constexpr T operator[](point m) const { return data[m.y][m.x]; }
};