#include "point.h"

#pragma once

enum direction_s : unsigned char;

union pointm {
	short unsigned		u;
	struct {
		unsigned char	x, y;
	};
	constexpr pointm() : u(0xFFFF) {}
	constexpr pointm(int x, int y) : x(x), y(y) {}
	constexpr pointm(unsigned short u) : u(u) {}
	constexpr pointm(const point& v) : x((unsigned char)v.x), y((unsigned char)v.y) {}
	explicit constexpr operator bool() const { return u != 0xFFFF; }
	constexpr operator point() const { return {x, y}; }
	//constexpr operator unsigned short() const { return u; }
	size_t				maximum() const { return x * y; }
	pointm				to(direction_s v, unsigned char mps) const;
};