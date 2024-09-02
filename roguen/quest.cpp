#include "bsreq.h"
#include "collection.h"
#include "quest.h"
#include "rand.h"
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

quest* last_quest;

static monsteri* random_boss() {
	collection<monsteri> source;
	source.select(is_boss);
	return source.random();
}

void quest::clear() {
	memset(this, 0, sizeof(*this));
}

void add_quest(questn type, point position, variant modifier, variant level, variant reward) {
	last_quest = find_quest(position);
	if(last_quest)
		return;
	if(!modifier || !level)
		return;
	auto boss = random_boss();
	if(!boss)
		return;
	last_quest = bsdata<quest>::addz();
	last_quest->clear();
	last_quest->type = type;
	last_quest->position = position;
	last_quest->modifier = modifier;
	last_quest->level = level;
	last_quest->reward = reward;
	last_quest->problem = boss;
	last_quest->rumor = xrand(20, 70);
}

void add_quest(questn type, point position) {
	add_quest(type, position,
		single("RandomDungeonModifier"),
		single("RandomDungeonType"),
		single("RandomDungeonTreasure"));
}

quest* find_quest(point v) {
	for(auto& e : bsdata<quest>()) {
		if(e.position == v)
			return &e;
	}
	return 0;
}