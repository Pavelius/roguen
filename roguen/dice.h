#pragma once

struct dice {
	char			c, d, b, m;
	int				roll() const;
	explicit operator bool() const { return c || b; }
};