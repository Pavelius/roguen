#include "point.h"

#pragma once

template<class T, int MPS>
class anymap {
	T data[MPS][MPS];
public:
	T* begin() { return &data[0][0]; }
	T* end() { return &data[MPS - 1][MPS - 1] + 1; }
	constexpr T& operator[](point m) { return data[m.y][m.x]; }
	constexpr T operator[](point m) const { return data[m.y][m.x]; }
};