#include "main.h"

BSDATA(triggeri) = {
	{"WhenCreatureP1EnterSiteP2"},
	{"WhenCreatureP1Dead"},
};
assert_enum(triggeri, WhenCreatureP1Dead)

void trigger::apply(variants source) const {
	pushvalue push_stop(stop_script, false);
	for(auto v : source) {
		if(stop_script)
			break;
		apply(v);
	}
}

void trigger::apply(variant v) const {
	if(v.iskind<speech>()) {
		if(!game.testcount(v))
			return;
		player->speech(bsdata<speech>::elements[v.value].id);
	} else
		player->apply(v);
}

void trigger::fire(trigger_s type, variant p1, variant p2) {
	for(auto& e : bsdata<trigger>()) {
		if(e.type == type
			&& (!e.p1 || e.p1 == p1)
			&& (!e.p2 || e.p2 == p2))
			e.apply(e.effect);
	}
}