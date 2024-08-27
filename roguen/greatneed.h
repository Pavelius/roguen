#include "dice.h"
#include "nameable.h"
#include "variant.h"

#pragma once

enum needn : unsigned char {
	NeedAccepted, NeedSpecialApplied, NeedCompleted, NeedFinished,
	NeedFail, NeedSuccess,
};
struct needni : nameable {
};
struct greatneedi : nameable {
	variants			targets;
	variants			need;
	variant				special;
	variants			fail, success;
	dice				days;
	unsigned			coins;
	unsigned			flags;
	const char*			get(const char* suffix) const { return getnme(str("%1%2", id, suffix)); }
	const char*			getcompleted() const { return get("Completed"); }
	const char*			getfail() const { return get("Fail"); }
	const char*			getpartial() const { return get("Partial"); }
	const char*			getsuccess() const { return get("Success"); }
};
struct greatneed {
	short unsigned		type;
	unsigned			deadline;
	variant				owner;
	unsigned			flags;
	int					score; // percent of completion
	explicit operator bool() const { return deadline != 0; }
	void				clear();
	static greatneed*	find(variant owner);
	static greatneed*	find(variant owner, needn f);
	const greatneedi&	geti() const { return bsdata<greatneedi>::elements[type]; }
	bool				is(needn v) const { return (flags & (1 << v)) != 0; }
	void				remove(needn v) { flags &= ~(1 << v); }
	void				set(needn v) { flags |= (1 << v); }
};
extern greatneed* last_need;

void add_greatneed(const greatneedi* type, variant owner, unsigned deadline);
void check_need_objects(int bonus);
void shrink_greatneed();
bool speech_need();