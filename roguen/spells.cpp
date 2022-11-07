#include "main.h"

BSDATA(spelli) = {
	{"CureWounds"},
	{"Entaglement"},
	{"Gate"},
	{"Light"},
	{"Sleep"},
	{"SummonUndead"},
	{"Teleport"},
	{"Web"},
};
assert_enum(spelli, Web)

static void match_creatures(const spelli& ei, int level) {
	auto ps = targets.begin();
	for(auto p : targets) {
		if(!ei.isallow(p, level))
			continue;
		*ps++ = p;
	}
	targets.count = ps - targets.begin();
}

bool spelli::iscombat(const void* object) {
	auto p = (spelli*)object;
	if(p->is(Enemies))
		return true;
	if(p->summon)
		return true;
	return false;
}

bool spelli::isallow(const creature* target, int level) const {
	if(conditions) {
		if(!target->isallow(conditions))
			return false;
	}
	if(duration) {
		//if(target->is((spell_s)bsid(this)))
		//	return false;
	}
	return true;
}

bool spelli::isallowmana(const void* object) {
	auto p = (spelli*)object;
	return player->get(Mana) >= p->mana;
}

bool spelli::isallowuse(const void* object) {
	auto p = (spelli*)object;
	return p->isready(player->get((spell_s)bsid(p)));
}

int	spelli::getcount(int level) const {
	return level + count.roll();
}

bool spelli::isready(int level) const {
	choose_targets(target);
	unsigned target_count = 1;
	if(is(Multitarget))
		target_count += level;
	if(targets.count > target_count)
		targets.count = target_count;
	match_creatures(*this, level);
	return targets.getcount() != 0 || summon.size() != 0;
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