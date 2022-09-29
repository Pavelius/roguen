#include "pointm.h"

#pragma once

template<class T, int MPS>
struct anymap {
	typedef short unsigned indext;
	static const int mps = MPS;
	T				data[MPS][MPS];
	constexpr const T& operator[](const pointm v) const { return data[v.y][v.x]; }
	static constexpr pointm	i2m(indext i) { return (i == 0xFFFF) ? pointm() : pointm(i % MPS, i / MPS); }
	static constexpr indext m2i(pointm m) { return m ? m.y * MPS + m.x : 0xFFFF; }
};
