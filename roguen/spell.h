#include "collection.h"
#include "diceprogress.h"
#include "duration.h"
#include "variant.h"

#pragma once

struct spelli : nameable {
	int				mana;
	unsigned char	goal;
	unsigned		target;
	duration_s		duration;
	diceprogress	count;
	variants		conditions, effect, summon;
	int				getcount(int level) const { return count.roll(level); }
	bool			is(int v) const { return (target & FG(v)) != 0; }
};
struct spellable {
	constexpr static const int maximum = 64;
	char			spells[maximum];
	int				getspell(const spelli& e) const { return spells[&e - bsdata<spelli>::elements]; }
	void			setspell(const spelli& e, int v) { spells[&e - bsdata<spelli>::elements] = v; }
};
struct spella : collection<spelli> {
	spelli*			choose(const char* title, const char* cancel, const spellable* context) const;
	void			select(const spellable* p);
};