#include "ability.h"
#include "areapiece.h"
#include "bsreq.h"
#include "creature.h"
#include "duration.h"
#include "feature.h"
#include "game.h"
#include "indexa.h"
#include "script.h"
#include "skilluse.h"
#include "siteskill.h"

extern indexa indecies;
extern point last_index;
siteskilla	last_actions;
siteskilli* last_action;

bool allow_targets(const variants& conditions);

void siteskilli::fixuse() const {
	auto rm = player->getroom();
	if(rm)
		skilluse::add(this, rm->center(), bsid(player), game.getminutes());
}

bool siteskilli::isusable() const {
	if((player->get(skill) + bonus) < 0)
		return false;
	if(!player->canuse(skill))
		return false;
	auto rm = player->getroom();
	if(rm) {
		auto su = skilluse::find(this, rm->center(), bsid(player));
		if(su) {
			if(!retry)
				return false;
			else {
				auto next_time = su->stamp + bsdata<durationi>::elements[retry].get(0);
				if(game.getminutes() < next_time)
					return false;
			}
		}
	}
	return allow_targets(effect);
}