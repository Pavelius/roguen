#include "areapiece.h"
#include "creature.h"
#include "filter.h"
#include "indexa.h"
#include "itema.h"
#include "pushvalue.h"
#include "site.h"
#include "script.h"

static variants	last_list;

static void read_same_list() {
	last_list.clear();
	if(!script_begin)
		return;
	auto type = script_begin->type;
	auto start = bsdata<variant>::source.indexof(script_begin);
	if(start == -1)
		return;
	while(script_begin < script_end && script_begin->type == type)
		script_begin++;
	auto end = bsdata<variant>::source.indexof(script_begin);
	last_list.start = start;
	last_list.count = end - start;
}

static bool match_list_value(int value) {
	for(auto v : last_list) {
		if(v.iskind<listi>()) {
			pushvalue push(last_list, bsdata<listi>::elements[v.value].elements);
			if(match_list_value(value))
				return true;
		} else if(v.iskind<randomizeri>()) {
			pushvalue push(last_list, bsdata<randomizeri>::elements[v.value].chance);
			if(match_list_value(value))
				return true;
		} else if(v.value == value)
			return true;
	}
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

static bool filter_wounded(const void* object) {
	auto p = (creature*)object;
	auto n = p->abilities[Hits];
	return n > 0 && n < p->basic.abilities[Hits];
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

static void filter_targets(fnvisible proc, int counter) {
	targets.collection<creature>::match(proc, counter >= 0);
}

static void filter_next_indecies(fnvisible proc, int counter) {
	read_same_list();
	if(last_list)
		indecies.match(match_list_feature, true);
}

static void filter_next_rooms(fnvisible proc, int counter) {
	read_same_list();
	if(last_list)
		rooms.match(match_list_room, true);
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
	targets = creatures;
}

static void select_enemies(fnvisible proc, int counter) {
	targets = enemies;
}

static void select_you(fnvisible proc, int counter) {
	targets.clear();
	targets.add(player);
}

static void select_your_items(fnvisible proc, int counter) {
	items.clear();
	items.select(player);
}

static void select_features(fnvisible proc, int counter) {
	indecies.clear();
	indecies.select(player->getposition(), counter ? counter : 1);
}

static void select_custom_creatures(fnvisible proc, int counter) {
	targets.clear();
	auto keep = counter >= 0;
	for(auto p : creatures) {
		if(proc(p) != keep)
			continue;
		targets.add(p);
	}
}

static void select_rooms(fnvisible proc, int counter) {
	rooms.clear();
	rooms.collectiona::select(area->rooms);
}

BSDATA(filteri) = {
	{"FilterBlessed", filter_blessed, filter_items},
	{"FilterClose", filter_close, filter_targets},
	{"FilterCursed", filter_cursed, filter_items},
	{"FilterIdentified", filter_identified, filter_items},
	{"FilterNextFeatures", 0, filter_next_indecies},
	{"FilterNextRooms", 0, filter_next_rooms},
	{"FilterUnaware", filter_unaware, filter_targets},
	{"FilterWounded", filter_wounded, filter_targets},
	{"SelectAllies", 0, select_allies},
	{"SelectCreatures", 0, select_creatures},
	{"SelectEnemies", 0, select_enemies},
	{"SelectFeatures", 0, select_features},
	{"SelectNeutralCreatures", filter_neutral, select_custom_creatures},
	{"SelectRooms", 0, select_rooms},
	{"SelectYou", 0, select_you},
	{"SelectYourItems", 0, select_your_items},
};
BSDATAF(filteri)