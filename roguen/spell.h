#include "collection.h"
#include "diceprogress.h"
#include "feat.h"
#include "variant.h"

#pragma once

struct spelli : nameable {
	int				mana;
	diceprogress	count;
	featable		feats;
	variants		targets, effect;
	variants		use, summon;
	int				getcount() const { return count.roll(0); }
	int				getmana() const;
	bool			ishostile() const;
};
struct spellable {
	constexpr static const int maximum = 64;
	unsigned		known_spells[maximum / 32];
	int				getspell(const spelli& e) const { return known_spell(&e - bsdata<spelli>::elements) ? 1 : 0; }
	bool			known_spell(int v) const { return (known_spells[v / 32] & ~(1 << (v % 32))) != 0; }
	void			learn_spell(int v) { known_spells[v / 32] |= 1 << (v % 32); }
};
struct spella : collection<spelli> {
	spelli*			choose(const char* title, const char* cancel, const spellable* context) const;
	void			select(const spellable* p);
};
extern spella allowed_spells;