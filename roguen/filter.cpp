#include "areapiece.h"
#include "creature.h"
#include "filter.h"
#include "indexa.h"
#include "itema.h"
#include "math.h"
#include "markuse.h"
#include "pushvalue.h"
#include "site.h"
#include "siteskill.h"
#include "script.h"

static variant last_variant;

static void clear_all_collections() {
	rooms.clear();
	targets.clear();
	indecies.clear();
	items.clear();
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
		return p->is((feat_s)v.value);
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

template<> void ftscript<filteri>(int value, int counter) {
	auto& ei = bsdata<filteri>::elements[value];
	ei.action(ei.proc, counter);
}
template<> bool fttest<filteri>(int value, int counter) {
	ftscript<filteri>(value, counter);
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

static bool filter_close(const void* object) {
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

static bool filter_human(const void* object) {
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
	auto p = (creature*)object;
	auto v = p->get(Wits);
	return v == 3 && v == 4;
}

static void match_targets(fnvisible proc, int counter) {
	targets.collection<creature>::match(proc, counter >= 0);
}

static void match_rooms(fnvisible proc, int counter) {
	rooms.match(proc, counter >= 0);
}

static void filter_next(fnvisible proc, int counter) {
	auto push = last_variant;
	last_variant = *script_begin++;
	if(indecies)
		indecies.match(match_list_feature, counter >= 0);
	else if(rooms)
		rooms.match(match_list_room, counter >= 0);
	else if(items)
		items.match(match_item_variant, counter >= 0);
	last_variant = push;
}

static void filter_items(fnvisible proc, int counter) {
	items.match(proc, counter >= 0);
}

static void select_allies(fnvisible proc, int counter) {
	targets = creatures;
	targets.collectiona::match(is_ally, counter >= 0);
}

static void select_creatures(fnvisible proc, int counter) {
	clear_all_collections();
	targets = creatures;
}

static void select_not_enemies(fnvisible proc, int counter) {
	clear_all_collections();
	targets = creatures;
	targets.match(is_enemy, false);
}

static void select_not_ally(fnvisible proc, int counter) {
	clear_all_collections();
	targets = creatures;
	targets.match(is_ally, false);
}

static void select_enemies(fnvisible proc, int counter) {
	clear_all_collections();
	targets = enemies;
}

static void select_you(fnvisible proc, int counter) {
	clear_all_collections();
	targets.add(player);
}

static void select_your_items(fnvisible proc, int counter) {
	clear_all_collections();
	items.select(player);
}

static void select_features(fnvisible proc, int counter) {
	clear_all_collections();
	indecies.select(player->getposition(), counter ? counter : 1);
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

static void select_rooms(fnvisible proc, int counter) {
	clear_all_collections();
	rooms.collectiona::select(area->rooms);
}

static void select_your_room(fnvisible proc, int counter) {
	clear_all_collections();
	if(player) {
		auto p = player->getroom();
		if(p)
			rooms.add(p);
	}
}

BSDATA(filteri) = {
	{"Filter", 0, filter_next},
	{"FilterAnimal", filter_animal, match_targets},
	{"FilterBlessed", filter_blessed, filter_items},
	{"FilterCharmed", filter_charmed, match_targets},
	{"FilterClose", filter_close, match_targets},
	{"FilterCursed", filter_cursed, filter_items},
	{"FilterDamaged", filter_damaged, filter_items},
	{"FilterExplored", filter_explored_room, match_rooms},
	{"FilterFeature", filter_feature, match_targets},
	{"FilterHuman", filter_human, match_targets},
	{"FilterIdentified", filter_identified, filter_items},
	{"FilterMindless", filter_mindless, match_targets},
	{"FilterNotable", filter_notable, match_rooms},
	{"FilterRoomMarked", filter_room_marked, match_rooms},
	{"FilterThisRoom", filter_this_room, match_rooms},
	{"FilterUnaware", filter_unaware, match_targets},
	{"FilterUndead", filter_undead, match_targets},
	{"FilterWounded", filter_wounded, match_targets},
	{"SelectAllies", 0, select_allies},
	{"SelectCreatures", 0, select_creatures},
	{"SelectEnemies", 0, select_enemies},
	{"SelectFeatures", 0, select_features},
	{"SelectNextFeatures", 0, select_next_features},
	{"SelectNotEnemies", 0, select_not_enemies},
	{"SelectRooms", 0, select_rooms},
	{"SelectWalls", 0, select_walls},
	{"SelectWallsMines", 0, select_walls_mines},
	{"SelectYou", 0, select_you},
	{"SelectYourItems", 0, select_your_items},
	{"SelectYourRoom", 0, select_your_room},
};
BSDATAF(filteri)