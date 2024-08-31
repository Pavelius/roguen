#include "collection.h"
#include "feat.h"
#include "variant.h"

#pragma once

struct spelli : nameable {
	int				mana;
	featable		feats;
	variants		use;
	variant			summon;
	bool			adventure;
	spelli*			counterspell;
	int				getmana() const;
};
struct spellable {
	constexpr static const int maximum = 64;
	unsigned		known_spells[maximum / 32];
	int				getspell(const spelli& e) const { return known_spell(&e - bsdata<spelli>::elements) ? 1 : 0; }
	bool			known_spell(int v) const { return (known_spells[v / 32] & ((unsigned)1 << (v % 32))) != 0; }
	void			learn_spell(int v) { known_spells[v / 32] |= 1 << (v % 32); }
	void			forget_spell(int v) { known_spells[v / 32] &= ~(1 << (v % 32)); }
};
struct spella : collection<spelli> {
	spelli*			choose(const char* title, const char* cancel, const spellable* context) const;
	void			select(const spellable* p);
};
extern const spelli* last_spell;
extern spella allowed_spells;