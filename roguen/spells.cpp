#include "boost.h"
#include "main.h"

static void match_creatures(const spelli& ei, int level) {
	auto ps = targets.begin();
	for(auto p : targets) {
		if(!p->isallow(ei, level))
			continue;
		*ps++ = p;
	}
	targets.count = ps - targets.begin();
}

bool spelli::isallowmana(const void* object) {
	auto p = (spelli*)object;
	return player->get(Mana) >= p->mana;
}

bool spelli::isallowuse(const void* object) {
	auto p = (spelli*)object;
	return p->isready(player->get(*p));
}

bool spelli::iscombat(const void* object) {
	auto p = (spelli*)object;
	if(p->is(Enemies))
		return true;
	if(p->summon)
		return true;
	return false;
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
	for(auto i = 0; i< (sizeof(p->spells) / sizeof(p->spells[0])); i++) {
		if(!p->spells[i])
			continue;
		*ps++ = bsdata<spelli>::elements + i;
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