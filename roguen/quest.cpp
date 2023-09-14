#include "bsreq.h"
#include "collection.h"
#include "quest.h"
#include "monster.h"

BSDATA(questni) = {
	{"KillBossQuest"},
	{"RescueQuest"},
};
assert_enum(questni, RescueQuest)

BSMETA(questni) = {
	{"id"},
	{}};
BSMETA(quest) = {
	{"object"},
	{"problem"},
	{"twist"},
	{"treasure"},
	{}};
BSDATAC(quest, 256)

static monsteri* random_boss() {
	collection<monsteri> source;
	source.select(monsteri::isboss);
	return source.random();
}

quest* quest::add(questn type, point position, variant modifier, variant level, variant reward) {
	auto p = find(position);
	if(p)
		return p;
	if(!modifier || !level)
		return 0;
	auto boss = random_boss();
	if(!boss)
		return 0;
	p = bsdata<quest>::addz();
	p->clear();
	p->type = type;
	p->position = position;
	p->modifier = modifier;
	p->level = level;
	p->reward = reward;
	p->problem = boss;
	p->rumor = xrand(20, 70);
	return p;
}

quest* quest::add(questn type, point position) {
	return add(type, position,
		single("RandomDungeonModifier"),
		single("RandomDungeonType"),
		single("RandomDungeonTreasure"));
}

quest* quest::find(point v) {
	for(auto& e : bsdata<quest>()) {
		if(e.position == v)
			return &e;
	}
	return 0;
}