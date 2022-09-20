#pragma once

struct dice {
	char		min, max;
	constexpr operator bool() const { return min != 0; }
	void operator+=(int v) { min += v; max += v; }
	void		correct();
	int			roll() const;
};