#include "nameable.h"
#include "variant.h"

#pragma once

enum needn : unsigned char {
	NeedKnown,
};

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
	unsigned			flags;
	char				score; // 0-5 depend on game difficult or by player charisma.
	explicit operator bool() const { return deadline != 0; }
	static greatneed*	add(const greatneedi* type, variant owner, unsigned deadline);
	void				clear() { zclear(this); }
	static greatneed*	find(variant owner);
	const greatneedi&	geti() const { return bsdata<greatneedi>::elements[type]; }
	bool				is(needn v) const { return (flags & (1 << v)) != 0; }
	void				set(needn v) { flags |= (1 << v); }
	void				remove(needn v) { flags &= ~(1 << v); }
};