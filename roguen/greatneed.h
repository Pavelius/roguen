#include "nameable.h"
#include "variant.h"

#pragma once

struct greatneedi : nameable {
	variants			need, conditions;
	variant				special;
	variants			fail, success;
};
struct greatneed {
	short unsigned		type;
	unsigned			deadline;
	variant				owner;
	int					random;
	char				score; // 0-20. Depend on game difficult or by player charisma.
	explicit operator bool() const { return deadline != 0; }
	static greatneed*	add(const greatneedi* type, variant owner, unsigned deadline);
	void				clear() { zclear(this); }
	static greatneed*	find(variant owner);
	const greatneedi&	geti() const { return bsdata<greatneedi>::elements[type]; }
};