#include "ability.h"
#include "areapiece.h"
#include "bsreq.h"
#include "creature.h"
#include "duration.h"
#include "feature.h"
#include "game.h"
#include "hotkey.h"
#include "indexa.h"
#include "script.h"
#include "skilluse.h"
#include "siteskill.h"

extern indexa indecies;
extern point last_index;
siteskilla last_actions;
siteskilli* last_action;

bool allow_targets(const variants& conditions);

void siteskilli::fixuse() const {
	auto rm = player->getroom();
	if(rm)
		skilluse_add(this, rm->center(), bsid(player), game.getminutes());
}

static bool have_item(variant v) {
	if(v.iskind<itemi>())
		return player->haveitem(bsdata<itemi>::elements + v.value);
	return false;
}

void siteskilli::usetool() {
	player->useitem((itemi*)tool);
}

bool siteskilli::ishotkeypresent() const {
	if(!key)
		return false;
	for(auto p : last_actions) {
		if(p->key == key)
			return true;
	}
	return false;
}

bool siteskilli::isusable() const {
	if((player->get(skill) + bonus) < 0)
		return false;
	if(tool) {
		if(!have_item(tool))
			return false;
	}
	if(ishotkeypresent())
		return false;
	auto rm = player->getroom();
	if(rm) {
		auto su = skilluse_find(this, rm->center(), bsid(player));
		if(su) {
			if(!retry)
				return false;
			else {
				auto next_time = su->stamp + get_duration(retry);
				if(game.getminutes() < next_time)
					return false;
			}
		}
	}
	return allow_targets(effect);
}

void site_skills_initialize() {
	for(auto& e : bsdata<siteskilli>()) {
		if(e.keyid)
			e.key = hotkey_parse(e.keyid);
	}
}