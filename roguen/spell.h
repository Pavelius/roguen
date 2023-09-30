#include "collection.h"
#include "diceprogress.h"
#include "duration.h"
#include "variant.h"

#pragma once

struct spelli : nameable {
	int				mana;
	duration_s		duration;
	diceprogress	count;
	variants		targets, effect, summon;
	bool			apply(int level, int targets_count, bool interactive, bool silent) const;
	int				getcount(int level) const { return count.roll(level); }
	bool			ishostile() const;
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
extern spella allowed_spells;