#include "variant.h"

#pragma once

struct boosti {
	variant		parent;
	variant		effect;
	unsigned	stamp;
	constexpr explicit operator bool() const { return parent.operator bool(); }
	void		clear() { memset(this, 0, sizeof(*this)); }
};

void add_boost(variant parent, variant effect, unsigned stop_time);
void update_all_boost(unsigned current_stamp);
void remove_boost(variant parent);

