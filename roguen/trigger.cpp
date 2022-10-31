#include "main.h"

BSDATA(triggeri) = {
	{"WhenCreatureEnterP1"},
};
assert_enum(triggeri, WhenEnterSiteP1)

void trigger::apply(variants source) const {
	for(auto v : source)
		apply(v);
}

void trigger::apply(variant v) const {
	if(v.iskind<speech>()) {
		if(!game.testcount(v))
			return;
		player->speech(bsdata<speech>::elements[v.value].id);
	}
}

void trigger::fire(trigger_s type, variant p1, variant p2) {
	for(auto& e : bsdata<trigger>()) {
		if(e.type == type && e.p1 == p1 && e.p2 == p2)
			e.apply(e.effect);
	}
}