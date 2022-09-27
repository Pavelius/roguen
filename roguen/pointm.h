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
	explicit constexpr operator bool() const { return u != 0xFFFF; }
	constexpr operator unsigned short() const { return u; }
	static unsigned		mps;
	unsigned short		get() const { return y * mps + x; }
	pointm				to(direction_s v) const;
};
