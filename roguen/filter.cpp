#include "areapiece.h"
#include "creature.h"
#include "filter.h"
#include "indexa.h"
#include "itema.h"
#include "pushvalue.h"
#include "site.h"
#include "script.h"

static variant last_variant;

static void clear_all_collections() {
	rooms.clear();
	targets.clear();
	indecies.clear();
	items.clear();
}

static void read_next_variant() {
	last_variant = *script_begin++;
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
		auto& source = bsdata<listi>::elements[last_variant.value].elements;
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
	return p->iswounded();
}

static bool filter_unaware(const void* object) {
	auto p = (creature*)object;
	return p->isunaware();
}

static bool filter_close(const void* object) {
	auto p = (creature*)object;
	return (area->getrange(p->getposition(), last_index) <= 1);
}

static bool filter_neutral(const void* object) {
	auto p = (creature*)object;
	return !p->is(Ally) && !p->is(Enemy);
}

static bool filter_feature(const void* object) {
	auto p = (creature*)object;
	return area->features[p->getposition()] != 0;
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

static bool filter_blessed(const void* object) {
	auto p = (item*)object;
	return p->is(Blessed);
}

static bool filter_identified(const void* object) {
	auto p = (item*)object;
	return p->isidentified();
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
	last_variant = push;
}

static void filter_items(fnvisible proc, int counter) {
	items.match(proc, counter >= 0);
}

static void select_allies(fnvisible proc, int counter) {
	targets = creatures;
	if(player->is(Ally))
		targets.match(Enemy, false);
	else if(player->is(Enemy))
		targets.match(Ally, false);
}

static void select_creatures(fnvisible proc, int counter) {
	clear_all_collections();
	targets = creatures;
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

static void select_custom_creatures(fnvisible proc, int counter) {
	clear_all_collections();
	auto keep = counter >= 0;
	for(auto p : creatures) {
		if(proc(p) != keep)
			continue;
		targets.add(p);
	}
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
	{"FilterBlessed", filter_blessed, filter_items},
	{"FilterClose", filter_close, match_targets},
	{"FilterDamaged", filter_damaged, filter_items},
	{"FilterCursed", filter_cursed, filter_items},
	{"FilterExplored", filter_explored_room, match_rooms},
	{"FilterIdentified", filter_identified, filter_items},
	{"FilterFeature", filter_feature, match_targets},
	{"FilterNotable", filter_notable, match_rooms},
	{"FilterThisRoom", filter_this_room, match_rooms},
	{"FilterUnaware", filter_unaware, match_targets},
	{"FilterWounded", filter_wounded, match_targets},
	{"SelectAllies", 0, select_allies},
	{"SelectCreatures", 0, select_creatures},
	{"SelectEnemies", 0, select_enemies},
	{"SelectFeatures", 0, select_features},
	{"SelectNeutralCreatures", filter_neutral, select_custom_creatures},
	{"SelectRooms", 0, select_rooms},
	{"SelectYou", 0, select_you},
	{"SelectYourItems", 0, select_your_items},
	{"SelectYourRoom", 0, select_your_room},
};
BSDATAF(filteri)