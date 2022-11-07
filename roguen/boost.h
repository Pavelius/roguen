#include "variant.h"

#pragma once

struct boosti {
	variant			parent;
	variant			effect;
	unsigned		stamp;
	constexpr explicit operator bool() const { return parent.operator bool(); }
	static boosti*	add(variant parent, variant effect, unsigned stop_time);
	void			clear() { memset(this, 0, sizeof(*this)); }
	static boosti*	find(variant parent, variant effect);
	static void		remove(variant parent);
	static void		updateall(unsigned current_stamp);
};
