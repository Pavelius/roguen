#include "main.h"

BSDATA(spelli) = {
	{"CureWounds", 10, AllyClose, Instant},
	{"Gate", 40, You, Instant},
	{"Light", 2, You, Hour1PL},
	{"ManaRegeneration", 0, You, Hour1PL},
	{"Regeneration", 15, You, Hour1PL},
	{"Sleep", 5, You, Minute20},
	{"Teleport", 30, You, Instant},
	{"Web", 5, EnemyOrAllyNear, Instant},
};
assert_enum(spelli, Web)

bool creature::apply(spell_s v, int level, bool run) {
	char temp[260]; stringbuilder sb(temp);
	auto& ei = bsdata<spelli>::elements[v];
	switch(v) {
	case CureWounds:
		if(is(NoWounded))
			return false;
		if(run)
			heal(ei.getcount(level));
		break;
	}
	if(ei.duration) {
		auto count = bsdata<durationi>::elements[ei.duration].get(level);
		sb.add("%1 %2i %-Minutes", ei.getname(), count);
		fixvalue(temp, 2);
		apply(v, count);
	}
	return true;
}

int	spelli::getcount(int level) const {
	return level + count.roll();
}

void spella::select(const spellable* p) {
	auto ps = data;
	for(auto i = (spell_s)0; i <= LastSpell; i = (spell_s)(i + 1)) {
		if(!p->spells[i])
			continue;
		*ps++ = bsdata<spelli>::source.ptr(i);
	}
	count = ps - data;
}

static const char* object_level(const void* object, stringbuilder& sb) {
	auto i = bsdata<spelli>::source.indexof(object);
	sb.add("%1i %-Level", player->spells[i]);
	return sb.begin();
}

static const char* object_mana(const void* object, stringbuilder& sb) {
	auto i = bsdata<spelli>::source.indexof(object);
	sb.add("[%1i] %-Mana", ((spelli*)object)->mana);
	return sb.begin();
}

spelli*	spella::choose(const char* title, const char* cancel) const {
	static listcolumn columns[] = {
		{"Level", 60, object_level},
		{"Mana", 60, object_mana},
		{}};
	pushvalue push_width(window_width, 300);
	pushvalue push_columns(current_columns, columns);
	pushvalue push_count(answers::column_count, 1);
	return collection<spelli>::choose(title, cancel, false);
}