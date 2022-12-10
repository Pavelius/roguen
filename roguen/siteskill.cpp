#include "ability.h"
#include "areamap.h"
#include "bsreq.h"
#include "creature.h"
#include "duration.h"
#include "feature.h"
#include "indexa.h"
#include "script.h"
#include "skilluse.h"
#include "siteskill.h"

BSMETA(siteskilli) = {
	BSREQ(id),
	BSENM(skill, abilityi),
	BSENM(retry, durationi),
	BSREQ(bonus),
	BSREQ(effect), BSREQ(fail),
	{}};
BSDATAC(siteskilli, 256);

extern areamap area;
extern indexa indecies;
extern point last_index;

static bool testing(variant v) {
	if(v.iskind<featurei>()) {
		if(v.counter < 0)
			return area.features[last_index] == v.value;
		else if(v.counter > 0)
			return area.features[last_index] != v.value;
	}
	return true;
}

static bool testing(const variants& source) {
	for(auto v : source) {
		if(!testing(v))
			return false;
	}
	return true;
}

static void applying(variant v) {
	if(v.iskind<featurei>()) {
		if(v.counter < 0)
			area.features[last_index] = 0;
		else if(v.counter > 0)
			area.features[last_index] = (unsigned char)v.value;
	} else if(v.iskind<script>())
		bsdata<script>::elements[v.value].proc(v.counter);
}

static void applying(const variants& source) {
	for(auto v : source)
		applying(v);
}

bool siteskilli::isvalid(const void* object) {
	auto p = (siteskilli*)object;
	if(!player->get(p->skill))
		return false;
	auto rm = player->getroom();
	if(rm) {
		auto su = skilluse::find(p, player->getposition(), bsid(player));
		if(!p->retry) {
			if(su)
				return false;
		} else {
			auto next_time = su->stamp + bsdata<durationi>::get(p->retry).get(1);
			if(su && game.getminutes() < next_time)
				return false;
		}
	}
	return testing(p->effect);
}

void siteskilli::apply() const {
	applying(effect);
}