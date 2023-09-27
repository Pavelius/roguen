#include "areapiece.h"
#include "creature.h"
#include "filter.h"
#include "indexa.h"
#include "itema.h"
#include "site.h"
#include "script.h"

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

static bool filter_cursed(const void* object) {
	auto p = (item*)object;
	return p->iscursed();
}

static bool filter_blessed(const void* object) {
	auto p = (item*)object;
	return p->is(Blessed);
}

static void filter_targets(fnvisible proc, int counter) {
	targets.collection<creature>::match(proc, counter >= 0);
}

static void filter_items(fnvisible proc, int counter) {
	items.match(proc, counter >= 0);
}

static void select_creatures(fnvisible proc, int counter) {
	targets = creatures;
}

static void select_enemies(fnvisible proc, int counter) {
	targets = enemies;
}

static void select_your_items(fnvisible proc, int counter) {
	items.clear();
	items.select(player);
}

static void select_features(fnvisible proc, int counter) {
	indecies.clear();
	indecies.select(player->getposition(), 5);
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
	{"FilterUnaware", filter_unaware, filter_targets},
	{"FilterWounded", filter_wounded, filter_targets},
	{"SelectCreatures", 0, select_creatures},
	{"SelectEnemies", 0, select_enemies},
	{"SelectFeatures", 0, select_features},
	{"SelectNeutralCreatures", filter_neutral, select_custom_creatures},
	{"SelectRooms", 0, select_rooms},
	{"SelectYourItems", 0, select_your_items},
};
BSDATAF(filteri)