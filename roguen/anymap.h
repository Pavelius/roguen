#include "point.h"

#pragma once

enum direction_s : unsigned char;

template<class T, int MPS>
struct anymap {
	T data[MPS][MPS];
	T* begin() { return &data[0][0]; }
	T* end() { return &data[MPS - 1][MPS - 1] + 1; }
	constexpr T& operator[](point m) { return data[m.y][m.x]; }
	constexpr T operator[](point m) const { return data[m.y][m.x]; }
};