#include "boost.h"
#include "listcolumn.h"
#include "pushvalue.h"
#include "spell.h"

BSDATAC(spelli, spellable::maximum)

static const spellable* current_spellable;
const spelli* last_spell;

static const char* object_mana(const void* object, stringbuilder& sb) {
	auto i = bsdata<spelli>::source.indexof(object);
	sb.add("[%1i] %-Mana", ((spelli*)object)->getmana());
	return sb.begin();
}

int	spelli::getmana() const {
	return mana;
}

spelli*	spella::choose(const char* title, const char* cancel, const spellable* context) const {
	static listcolumn columns[] = {
		{"Mana", 80, object_mana},
		{}};
	pushvalue push_context(current_spellable, context);
	pushvalue push_columns(current_columns, columns);
	return (spelli*)collectiona::choose(spelli::getname, title, cancel, false);
}

void spella::select(const spellable* p) {
	auto ps = data;
	for(auto i = 0; i < 64; i++) {
		if(!p->known_spell(i))
			continue;
		*ps++ = bsdata<spelli>::elements + i;
	}
	count = ps - data;
}