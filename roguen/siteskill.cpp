#include "ability.h"
#include "areapiece.h"
#include "bsreq.h"
#include "creature.h"
#include "feature.h"
#include "game.h"
#include "hotkey.h"
#include "indexa.h"
#include "pushvalue.h"
#include "script.h"
#include "siteskill.h"

extern indexa indecies;
extern point last_index;
siteskilla last_actions;
siteskilli* last_action;

bool allow_targets(const variants& conditions);

static bool use_item(variant v, bool run) {
	if(v.iskind<itemi>())
		return player->useitem(bsdata<itemi>::elements + v.value, run);
	return false;
}

void siteskilli::usetool() {
	use_item((itemi*)tool, true);
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
	auto ishuman = player->ishuman();
	if(!player->basic.abilities[skill])
		return false;
	if((player->get(skill) + bonus) < 0)
		return false;
	if(tool) {
		if(!use_item(tool, false))
			return false;
	}
	if(ishotkeypresent())
		return false;
	pushvalue push(last_action, const_cast<siteskilli*>(this));
	return allow_targets(effect);
}

void check_site_skills(int bonus) {
	for(auto& e : bsdata<siteskilli>()) {
		if(e.keyid)
			e.key = hotkey_parse(e.keyid);
	}
}