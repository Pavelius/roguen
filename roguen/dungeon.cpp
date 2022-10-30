#include "main.h"

static monsteri* random_boss() {
	collection<monsteri> source;
	source.select(monsteri::isboss);
	return source.random();
}

static const char* word(const char* prefix, const char* suffix) {
	char temp[64]; stringbuilder sb(temp);
	sb.add(prefix);
	sb.add(suffix);
	return szdup(temp);
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
	p->entrance = bsdata<sitei>::find(word(type->id, "Entrance"));
	p->level = type;
	p->final_level = bsdata<sitei>::find(word(type->id, "Final"));
	p->reward = reward;
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