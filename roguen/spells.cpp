#include "main.h"

BSDATA(spelli) = {
	{"CureWounds"},
	{"Gate"},
	{"Light"},
	{"ManaRegeneration", 0, FG(You), Hour1PL},
	{"Regeneration"},
	{"Sleep", 5, FG(You), Minute20},
	{"Teleport", 30, FG(You), Instant},
	{"Web", 5, FG(Enemies) | FG(Ranged), Instant},
};
assert_enum(spelli, Web)

void creature::apply(spell_s v, int level) {
	auto& ei = bsdata<spelli>::elements[v];
	switch(v) {
	case CureWounds:
		heal(ei.getcount(level));
		break;
	case Web:
		area.set(getposition(), Webbed);
		break;
	}
	if(ei.duration) {
		auto count = bsdata<durationi>::elements[ei.duration].get(level);
		fixvalue(str("%1 %2i %-Minutes", ei.getname(), count), 2);
		addeffect(v, count);
	}
}

bool creature::isallow(const variants& source) const {
	for(auto v : source) {
		if(isallow(v))
			return true;
	}
	return false;
}

bool creature::isallow(spell_s v, int level) const {
	auto& ei = bsdata<spelli>::elements[v];
	if(ei.conditions) {
		if(!isallow(ei.conditions))
			return false;
	}
	if(ei.duration) {
		unsigned next_stamp = game.getminutes() + bsdata<durationi>::elements[ei.duration].get(level);
		auto p = boosti::find(this, v);
		if(p && p->stamp > next_stamp)
			return false;
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
	sb.add("[%1i] %-Level", player->spells[i]);
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
		{"Mana", 80, object_mana},
		{}};
	pushvalue push_width(window_width, 300);
	pushvalue push_columns(current_columns, columns);
	return collection<spelli>::choose(title, cancel, false);
}