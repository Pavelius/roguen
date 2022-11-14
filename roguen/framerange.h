#pragma once

struct framerange {
	unsigned char	start;
	unsigned char	count;
	int				get(int s) const { return start + s % count; }
	explicit constexpr operator bool() const { return count != 0; }
};
