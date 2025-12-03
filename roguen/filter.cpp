#include "areapiece.h"
#include "creature.h"
#include "filter.h"
#include "indexa.h"
#include "itema.h"
#include "math.h"
#include "markuse.h"
#include "pushvalue.h"
#include "querry.h"
#include "race.h"
#include "site.h"
#include "siteskill.h"
#include "script.h"
#include "variant.h"

static variant last_variant;

static void clear_all_collections() {
	rooms.clear();
	targets.clear();
	indecies.clear();
	records.clear();
}

static bool match_list_value(int value) {
	if(last_variant.iskind<listi>()) {
		pushvalue push(last_variant);
		auto& source = bsdata<listi>::elements[last_variant.value].elements;
		for(auto v : source) {
			last_variant = v;
			if(match_list_value(value))
				return true;
		}
	} else if(last_variant.iskind<randomizeri>()) {
		pushvalue push(last_variant);
		auto& source = bsdata<randomizeri>::elements[last_variant.value].chance;
		for(auto v : source) {
			last_variant = v;
			if(match_list_value(value))
				return true;
		}
	} else if(last_variant.value == value)
		return true;
	return false;
}

static bool match_list_feature(point m) {
	return match_list_value(area->features[m]);
}

static bool match_list_room(const void* object) {
	return match_list_value(&((roomi*)object)->geti() - bsdata<sitei>::elements);
}

static bool match_item_variant(const item* p, variant v) {
	if(v.iskind<feati>())
		return p->is((featn)v.value);
	else if(v.iskind<itemi>())
		return p->is(bsdata<itemi>::elements + v.value);
	else if(last_variant.iskind<listi>()) {
		for(auto e : bsdata<listi>::elements[v.value].elements) {
			if(match_item_variant(p, e))
				return true;
		}
	} else if(last_variant.iskind<randomizeri>()) {
		for(auto e : bsdata<randomizeri>::elements[v.value].chance) {
			if(match_item_variant(p, e))
				return true;
		}
	}
	return false;
}

static bool match_item_variant(const void* object) {
	return match_item_variant((item*)object, last_variant);
}

static bool match_wall(point m) {
	return bsdata<tilei>::elements[area->tiles[m]].iswall();
}

static bool match_wall_mines(point m) {
	auto& ei = bsdata<tilei>::elements[area->tiles[m]];
	return ei.iswall() && ei.is(Mines);
}

template<> void fiscript<filteri>(int value, int counter) {
	auto& ei = bsdata<filteri>::elements[value];
	ei.action(ei.proc, counter);
}
template<> bool fitest<filteri>(int value, int counter) {
	fiscript<filteri>(value, counter);
	return true;
}

static bool filter_wounded(const void* object) {
	auto p = (creature*)object;
	auto n = p->abilities[Hits];
	return n > 0 && n < p->basic.abilities[Hits];
}

static bool filter_damaged(const void* object) {
	auto p = (item*)object;
	return p->isdamaged();
}

static bool filter_unaware(const void* object) {
	auto p = (creature*)object;
	return p->isunaware();
}

static bool if_close(const void* object) {
	auto p = (creature*)object;
	return (area->getrange(p->getposition(), last_index) <= 1);
}

static bool filter_feature(const void* object) {
	auto p = (creature*)object;
	return area->features[p->getposition()] != 0;
}

static bool filter_room_marked(const void* object) {
	auto p = (roomi*)object;
	return markused(last_action, center(p->rc), bsid(player));
}

static bool filter_notable(const void* object) {
	auto p = (roomi*)object;
	return p->is(Notable);
}

static bool filter_explored_room(const void* object) {
	auto p = (roomi*)object;
	return area->is(p->center(), Explored);
}

static bool filter_this_room(const void* object) {
	return player->getroom() == (roomi*)object;
}

static bool filter_cursed(const void* object) {
	auto p = (item*)object;
	return p->iscursed();
}

static bool if_human(const void* object) {
	auto p = (creature*)object;
	return p->ishuman();
}

static bool filter_charmed(const void* object) {
	auto p = (creature*)object;
	return p->getcharmer() != 0;
}

static bool filter_mindless(const void* object) {
	auto p = (creature*)object;
	return p->get(Wits) <= 5;
}

static bool filter_blessed(const void* object) {
	auto p = (item*)object;
	return p->is(Blessed);
}

static bool filter_identified(const void* object) {
	auto p = (item*)object;
	return p->isidentified();
}

static bool filter_undead(const void* object) {
	auto p = (creature*)object;
	return p->is(Undead);
}

static bool filter_animal(const void* object) {
	if(bsdata<creature>::have(object)) {
		auto p = (creature*)object;
		auto v = p->get(Wits);
		return v == 3 || v == 4;
	}
	return false;
}

static void match_targets(fnvisible proc, int counter) {
	targets.match(proc, counter >= 0);
}

static void match_rooms(fnvisible proc, int counter) {
	rooms.match(proc, counter >= 0);
}

static void select_allies() {
	records = creatures;
	querry_filter();
}

static void select_creatures() {
	records = creatures;
	querry_filter();
}

static void select_not_enemies() {
	records = creatures;
	records.match(is_enemy, false);
	querry_filter();
}

static void select_not_ally() {
	records = creatures;
	records.match(is_ally, false);
	querry_filter();
}

static void select_enemies() {
	records = enemies;
	querry_filter();
}

static void select_you() {
	records.clear();
	records.add(player);
	querry_filter();
}

static void select_items(collectiona& records, creature* p) {
	auto pb = records.data;
	auto pe = records.endof();
	if(!p)
		return;
	for(auto& e : p->wears) {
		if(!e)
			continue;
		if(pb < pe)
			*pb++ = &e;
	}
	records.count = pb - records.data;
}

static void select_your_items() {
	select_items(records, player);
	querry_filter();
}

static void select_features() {
	indecies.select(player->getposition(), true);
	querry_filter();
}

static void select_walls(fnvisible proc, int counter) {
	clear_all_collections();
	indecies.select(match_wall, true, player->getposition(), imax(1, counter));
}

static void select_walls_mines(fnvisible proc, int counter) {
	clear_all_collections();
	indecies.select(match_wall_mines, true, player->getposition(), imax(1, counter));
}

static void select_next_features(fnvisible proc, int counter) {
	clear_all_collections();
	auto push = last_variant;
	last_variant = next_script();
	indecies.select(match_list_feature, counter >= 0, 0);
	last_variant = push;
}

static void select_rooms() {
	records.select(area->rooms);
}

static void select_your_room(fnvisible proc, int counter) {
	clear_all_collections();
	if(player) {
		auto p = player->getroom();
		if(p)
			rooms.add(p);
	}
}

template<> bool fnfilter<tilei>(const void* object, int param) {
	if(bsdata<creature>::have(object))
		return area->tiles[((creature*)object)->getposition()] == param;
	return false;
}

template<> bool fnfilter<spelli>(const void* object, int param) {
	if(bsdata<creature>::have(object))
		return ((creature*)object)->known_spell(param);
	return false;
}

template<> bool fnfilter<abilityi>(const void* object, int param) {
	if(bsdata<creature>::have(object))
		return ((creature*)object)->get(last_ability) >= param;
	return false;
}

template<> bool fnfilter<feati>(const void* object, int param) {
	if(bsdata<creature>::have(object))
		return ((creature*)object)->is((featn)param);
	return false;
}

template<> bool fnfilter<itemi>(const void* object, int param) {
	if(bsdata<creature>::have(object))
		return ((creature*)object)->haveitem(param);
	if(bsdata<itemi>::have(object))
		return ((item*)object)->istype(param);
	return false;
}

template<> bool fnfilter<featurei>(const void* object, int param) {
	if(haveposition(object)) {
		auto m = *((point*)object);
		return area->features[m] == param;
	}
	return false;
}

template<> bool fnfilter<racei>(const void* object, int param) {
	variant v = object;
	if(bsdata<creature>::have(object)) {
		variant v = ((creature*)object)->getkind();
		if(v.iskind<racei>())
			return v.value == param;
	}
	return false;
}

static void* group_position(const void* object) {
	if(bsdata<creature>::have(object))
		return &area->tiles[((creature*)object)->getposition()];
	else if(haveitem(object))
		return &area->tiles[((itemground*)object)->position];
	return 0;
}

static void group_position() {
	records.group(group_position);
}

static void if_player() {
	querry_allow_all(player);
}

BSDATA(querryi) = {
	{"GroupPosition", group_position},
	{"IfPlayer", if_player},
	{"SelectAllies", select_allies},
	{"SelectCreatures", select_creatures},
	{"SelectEnemies", select_enemies},
	{"SelectFeatures", select_features},
	{"SelectNotEnemies", select_not_enemies},
	{"SelectRooms", select_rooms},
	{"SelectYou", select_you},
	{"SelectYourItems", select_your_items},
};
BSDATAF(querryi)
BSDATA(filteri) = {
	{"FilterAnimal", filter_animal, match_targets},
	{"FilterBlessed", filter_blessed},
	{"FilterCharmed", filter_charmed, match_targets},
	{"FilterCursed", filter_cursed},
	{"FilterDamaged", filter_damaged},
	{"FilterExplored", filter_explored_room, match_rooms},
	{"FilterFeature", filter_feature, match_targets},
	{"FilterIdentified", filter_identified},
	{"FilterMindless", filter_mindless, match_targets},
	{"FilterNotable", filter_notable, match_rooms},
	{"FilterRoomMarked", filter_room_marked, match_rooms},
	{"FilterThisRoom", filter_this_room, match_rooms},
	{"FilterUnaware", filter_unaware, match_targets},
	{"FilterUndead", filter_undead, match_targets},
	{"FilterWounded", filter_wounded, match_targets},
	{"IfClose", if_close, match_targets},
	{"IfHuman", if_human, match_targets},
	{"SelectNextFeatures", 0, select_next_features},
	{"SelectWalls", 0, select_walls},
	{"SelectWallsMines", 0, select_walls_mines},
	{"SelectYourRoom", 0, select_your_room},
};
BSDATAF(filteri)