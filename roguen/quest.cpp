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
BSDATAC(quest, 256)

static monsteri* random_boss() {
	collection<monsteri> source;
	source.select(monsteri::isboss);
	return source.random();
}

quest* quest::add(point position, variant modifier, variant type, variant reward) {
	auto p = find(position);
	if(p)
		return p;
	if(!modifier || !type)
		return 0;
	auto boss = random_boss();
	if(!boss)
		return 0;
	p = bsdata<quest>::addz();
	p->clear();
	p->position = position;
	p->modifier = modifier;
	//p->entrance = bsdata<sitei>::find(word(type->id, "Entrance"));
	p->level = type;
	//p->final_level = bsdata<locationi>::find(word(type->id, "Final"));
	p->reward = reward;
	p->problem = boss;
	p->rumor = xrand(20, 70);
	return p;
}

quest* quest::find(point v) {
	for(auto& e : bsdata<quest>()) {
		if(e.position == v)
			return &e;
	}
	return 0;
}