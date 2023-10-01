#include "boost.h"
#include "listcolumn.h"
#include "pushvalue.h"
#include "spell.h"

BSDATAC(spelli, spellable::maximum)

static const spellable* current_spellable;

static const char* object_level(const void* object, stringbuilder& sb) {
	auto i = bsdata<spelli>::source.indexof(object);
	sb.add("[%1i] %-Level", current_spellable->spells[i]);
	return sb.begin();
}

static const char* object_mana(const void* object, stringbuilder& sb) {
	auto i = bsdata<spelli>::source.indexof(object);
	sb.add("[%1i] %-Mana", ((spelli*)object)->mana);
	return sb.begin();
}

bool spelli::ishostile() const {
	return feats.is(Enemy);
}

spelli*	spella::choose(const char* title, const char* cancel, const spellable* context) const {
	static listcolumn columns[] = {
		{"Level", 60, object_level},
		{"Mana", 80, object_mana},
		{}};
	pushvalue push_context(current_spellable, context);
	pushvalue push_columns(current_columns, columns);
	return collection<spelli>::choose(title, cancel, false);
}

void spella::select(const spellable* p) {
	auto ps = data;
	for(auto i = 0; i < (sizeof(p->spells) / sizeof(p->spells[0])); i++) {
		if(!p->spells[i])
			continue;
		*ps++ = bsdata<spelli>::elements + i;
	}
	count = ps - data;
}