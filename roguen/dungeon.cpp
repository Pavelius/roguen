#include "main.h"

static monsteri* random_boss() {
	collection<monsteri> source;
	source.select(monsteri::isboss);
	return source.random();
}

dungeon* dungeon::add(point position, sitei* modifier, sitei* type, variant reward) {
	if(!modifier || !type)
		return 0;
	auto boss = random_boss();
	if(!boss)
		return 0;
	auto p = bsdata<dungeon>::addz();
	p->clear();
	p->position = position;
	p->modifier = modifier;
	p->entrance = bsdata<sitei>::find("DefaultDungeonEntrance");
	p->reward = reward;
	p->level = type;
	p->final_level = type;
	p->guardian = boss;
	p->rumor = xrand(20, 70);
	return p;
}

dungeon* dungeon::add(point position) {
	return add(position,
		single("RandomDungeonModifier"),
		single("RandomDungeonType"),
		single("RandomDungeonTreasure"));
}

dungeon* dungeon::find(point v) {
	for(auto& e : bsdata<dungeon>()) {
		if(e.position == v)
			return &e;
	}
	return 0;
}