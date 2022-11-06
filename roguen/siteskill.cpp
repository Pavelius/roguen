#include "main.h"

bool siteskilli::isvalid(const void* object) {
	auto p = (siteskilli*)object;
	auto rm = player->getroom();
	auto su = skilluse::find(p, bsid(player), bsid(rm));
	if(!p->retry) {
		if(su)
			return false;
	} else {
		auto next_time = su->stamp + bsdata<durationi>::get(p->retry).get(1);
		if(su && game.getminutes() < next_time)
			return false;
	}
	return p->site == last_site
		&& player->get(p->skill) != 0;
}