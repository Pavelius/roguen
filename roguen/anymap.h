#include "point.h"

#pragma once

enum direction_s : unsigned char;

template<class T, int MPS>
struct anymap {
	static const auto mps = MPS;
	T data[MPS][MPS];
	T* begin() { return &data[0][0]; }
	T* end() { return &data[MPS - 1][MPS - 1] + 1; }
	static constexpr bool isvalid(point m) { return ((unsigned short)m.x) < ((unsigned short)MPS) && ((unsigned short)m.y) < ((unsigned short)MPS); }
	constexpr T& operator[](point m) { return data[m.y][m.x]; }
	constexpr T operator[](point m) const { return data[m.y][m.x]; }
};