#include "point.h"

#pragma once

template<class T, int MPS>
class anymap {
	T data[MPS * MPS];
public:
	T* begin() { return data; }
	T* end() { return data + MPS * MPS; }
	constexpr T& operator[](point m) { return data[m.y * MPS + m.x]; }
	constexpr T operator[](point m) const { return data[m.y * MPS + m.x]; }
	constexpr bool have(const void* p) const { return p >= data && p < data + MPS * MPS; }
	constexpr point indexof(const void* p) const { return have(p) ? point{(short)(((T*)p - (T*)data) % MPS), (short)(((T*)p - (T*)data) / MPS)} : point{-1000, -1000}; }
};