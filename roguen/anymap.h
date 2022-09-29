#include "pointm.h"

#pragma once

template<class T, int MPS>
struct anymap {
	typedef short unsigned indext;
	static const auto mps = MPS;
	T				data[MPS][MPS];
	constexpr T		get(pointm i) const { return data[i.y][i.x]; }
	constexpr static pointm	i2m(indext i) { return (i == 0xFFFF) ? pointm() : pointm(i % MPS, i / MPS); }
	constexpr static indext m2i(pointm m) { return m ? m.y * MPS + m.x : 0xFFFF; }
	constexpr void	set(pointm i, T v) { if(i) data[i.y][i.x] = v; }
	constexpr static pointm to(pointm i, direction_s d) { return i.to(d, MPS); }
};