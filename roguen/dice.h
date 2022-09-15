#pragma once

struct dice {
	char		min, max;
	void operator+=(int v) { min += v; max += v; }
	void		correct();
	int			roll() const;
};