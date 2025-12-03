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

static void clear_all_collections() {
	rooms.clear();
	targets.clear();
	indecies.clear();
	records.clear();
}

static bool match_wall(point m) {
	return bsdata<tilei>::elements[area->tiles[m]].iswall();
}

static bool match_wall_mines(point m) {
	auto& ei = bsdata<tilei>::elements[area->tiles[m]];
	return ei.iswall() && ei.is(Mines);
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
}

static void select_creatures() {
	records = creatures;
}

static void select_not_enemies() {
	records = creatures;
	records.match(is_enemy, false);
}

static void select_not_ally() {
	records = creatures;
	records.match(is_ally, false);
}

static void select_enemies() {
	records = enemies;
}

static void select_you() {
	records.clear();
	records.add(player);
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
}

static void select_features() {
	indecies.select(player->getposition(), true);
}

static void select_walls() {
	indecies.select(match_wall, true, player->getposition(), 1);
}

static void select_walls_mines() {
	indecies.select(match_wall_mines, true, player->getposition(), 1);
}

static void select_rooms() {
	records.select(area->rooms);
}

static void select_your_room() {
	records.clear();
	if(player) {
		auto p = player->getroom();
		if(p)
			records.add(p);
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

static bool querry_filter_player() {
	return querry_allow(player);
}

BSDATA(querryi) = {
	{"GroupPosition", group_position, querry_nobody},
	{"IfPlayer", script_empty, querry_filter_player},
	{"SelectAllies", select_allies, querry_select},
	{"SelectCreatures", select_creatures, querry_select},
	{"SelectEnemies", select_enemies, querry_select},
	{"SelectFeatures", select_features, querry_select},
	{"SelectNotEnemies", select_not_enemies, querry_select},
	{"SelectRooms", select_rooms, querry_select},
	{"SelectYou", select_you, querry_select},
	{"SelectYourItems", select_your_items, querry_select},
	{"SelectYourRoom", select_your_room, querry_select},
	{"SelectWalls", select_walls, querry_select},
	{"SelectWallsMines", select_walls_mines, querry_select},
};
BSDATAF(querryi)
BSDATA(filteri) = {
	{"FilterAnimal", filter_animal},
	{"FilterBlessed", filter_blessed},
	{"FilterCharmed", filter_charmed},
	{"FilterCursed", filter_cursed},
	{"FilterDamaged", filter_damaged},
	{"FilterExplored", filter_explored_room},
	{"FilterFeature", filter_feature},
	{"FilterIdentified", filter_identified},
	{"FilterMindless", filter_mindless},
	{"FilterNotable", filter_notable},
	{"FilterRoomMarked", filter_room_marked},
	{"FilterThisRoom", filter_this_room},
	{"FilterUnaware", filter_unaware},
	{"FilterUndead", filter_undead},
	{"FilterWounded", filter_wounded},
	{"IfClose", if_close},
	{"IfHuman", if_human},
};
BSDATAF(filteri)