#pragma once

template<int count>
class datamap {
	constexpr static unsigned perbyte = 4;
	unsigned char	data[(count + perbyte - 1) / perbyte];
public:
	int				get(short unsigned i) const { return (data[i / perbyte] >> ((i % perbyte) * 2)) & 0x03; }
	void			set(short unsigned i, int v) { data[i / perbyte] |= (v & 0x03) << ((i % perbyte) * 2); }
};
