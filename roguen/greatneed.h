#include "nameable.h"
#include "variant.h"

#pragma once

enum needn : unsigned char {
	NeedAccepted
};
struct needni : nameable {
};
struct greatneedi : nameable {
	variants			targets;
	variants			need;
	variant				special;
	variants			fail, success;
	char				level;
	unsigned			flags;
};
struct greatneed {
	short unsigned		type;
	unsigned			deadline;
	variant				owner;
	int					random;
	unsigned			flags;
	int					score; // percent of completion
	explicit operator bool() const { return deadline != 0; }
	static greatneed*	add(const greatneedi* type, variant owner, unsigned deadline);
	void				clear() { zclear(this); }
	static greatneed*	find(variant owner);
	const greatneedi&	geti() const { return bsdata<greatneedi>::elements[type]; }
	bool				is(needn v) const { return (flags & (1 << v)) != 0; }
	void				set(needn v) { flags |= (1 << v); }
	void				remove(needn v) { flags &= ~(1 << v); }
};