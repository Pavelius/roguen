#include "ability.h"
#include "bsreq.h"
#include "duration.h"
#include "skilluse.h"
#include "siteskill.h"
#include "main.h"

BSMETA(siteskilli) = {
	BSREQ(id),
	BSENM(skill, abilityi),
	BSREQ(site),
	BSENM(retry, durationi),
	BSREQ(bonus),
	BSREQ(effect), BSREQ(fail),
	{}};
BSDATAC(siteskilli, 256);

bool siteskilli::isvalid(const void* object) {
	auto p = (siteskilli*)object;
	auto rm = player->getroom();
	if(!rm)
		return false;
	auto su = skilluse::find(p, rm->center(), bsid(player));
	if(!p->retry) {
		if(su)
			return false;
	} else {
		auto next_time = su->stamp + bsdata<durationi>::get(p->retry).get(1);
		if(su && game.getminutes() < next_time)
			return false;
	}
	return p->site == last_site && player->get(p->skill) != 0;
}