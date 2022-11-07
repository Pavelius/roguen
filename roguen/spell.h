#include "collection.h"
#include "dice.h"
#include "duration.h"
#include "variant.h"

#pragma once

struct spellable {
	constexpr static const int maximum = 64;
	char			spells[maximum];
};
struct spelli : nameable {
	int				mana;
	unsigned		target;
	duration_s		duration;
	dice			count;
	variants		conditions, effect, summon;
	int				getcount(int level) const;
	bool			is(int v) const { return (target & FG(v)) != 0; }
};
struct spella : collection<spelli> {
	spelli*			choose(const char* title, const char* cancel, const spellable* context) const;
	void			select(const spellable* p);
};