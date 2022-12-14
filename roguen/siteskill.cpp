#include "ability.h"
#include "areamap.h"
#include "bsreq.h"
#include "condition.h"
#include "creature.h"
#include "duration.h"
#include "feature.h"
#include "game.h"
#include "indexa.h"
#include "script.h"
#include "skilluse.h"
#include "siteskill.h"

extern areamap area;
extern indexa indecies;
extern point last_index;

bool choose_targets(unsigned flags, const variants& effects);

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
	return choose_targets(p->target, p->effect);
}

void siteskilli::apply() const {
	applying(effect);
}