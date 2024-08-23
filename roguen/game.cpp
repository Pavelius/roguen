#include "areapiece.h"
#include "boost.h"
#include "greatneed.h"
#include "global.h"
#include "game.h"
#include "fraction.h"
#include "pushvalue.h"
#include "script.h"
#include "trigger.h"
#include "triggern.h"
#include "creature.h"

namespace draw {
bool			isnext();
}

static char		console_text[4096];
stringbuilder	console(console_text);
gamei			game;
point			start_village = {128, 128};

void prepare_need();
void update_need();

void gamei::clear() {
	memset(this, 0, sizeof(*this));
}

static void all(fnevent proc) {
	auto push_player = player;
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid()) {
			player = &e;
			proc();
		}
	}
	player = push_player;
}

static void allnext(fnevent proc) {
	auto push_player = player;
	for(auto& e : bsdata<creature>()) {
		if(draw::isnext())
			break;
		if(e.isvalid()) {
			player = &e;
			proc();
		}
	}
	player = push_player;
}

static bool istriggertype(const void* object, int param) {
	auto p = (trigger*)object;
	return p->type == param;
}

static bool iscreature(const void* object) {
	auto p = (trigger*)object;
	if(p->p1.iskind<variant>()) {
		auto pv = bsdata<varianti>::elements + p->p1.value;
		return pv == varianti::getsource("Creature");
	}
	return p->p1.iskind<creature>();
}

static bool isfeature(const void* object) {
	auto p = (trigger*)object;
	if(p->p1.iskind<varianti>()) {
		auto pv = bsdata<varianti>::elements + p->p1.value;
		return pv == varianti::getsource("Feature");
	}
	return p->p1.iskind<featurei>();
}

static collection<trigger>& get_triggers(triggern type, fnvisible proc) {
	static collection<trigger> source;
	source.select();
	source.match(istriggertype, type, true);
	source.match(proc, true);
	return source;
}

static void all_creatures(triggern type) {
	auto push_player = player;
	auto& source = get_triggers(type, iscreature);
	for(auto& e : bsdata<creature>()) {
		if(!e.isvalid())
			continue;
		for(auto p : source) {
			if(!p->match(&e, {}))
				continue;
			player = &e;
			script_run(p->effect);
		}
	}
	player = push_player;
}

static void all_features(triggern type) {
	point m;
	pushvalue push_point(last_rect);
	auto& source = get_triggers(type, isfeature);
	for(m.y = 0; m.y < area->mps; m.y++) {
		for(m.x = 0; m.x < area->mps; m.x++) {
			if(area->features[m] == 0)
				continue;
			auto v1 = bsdata<featurei>::elements + (int)area->features[m];
			for(auto p : source) {
				if(!p->match(v1, {}))
					continue;
				last_rect = m.rectangle();
				script_run(p->effect);
			}
		}
	}
}

static bool isfreelt(point m) {
	if(area->is(m, Darkened))
		return false;
	return area->isfree(m);
}

bool isfreeltsv(point m) {
	area->setflag(m, Visible);
	area->setflag(m, Explored);
	if(area->is(m, Darkened))
		return false;
	return area->isfree(m);
}

static void update_los() {
	point m;
	for(m.y = 0; m.y < area->mps; m.y++)
		for(m.x = 0; m.x < area->mps; m.x++) {
			area->remove(m, Visible);
			if(area->is(m, Darkened))
				area->remove(m, Explored);
		}
	auto human = game.getowner();
	if(human) {
		auto i = human->getposition();
		if(area->isvalid(i))
			area->setlos(i, human->getlos(), isfreeltsv);
	}
}

static void decoy_food() {
	for(auto& e : area->items) {
		if(!e)
			continue;
		auto rotting = e.geti().rotting;
		if(!rotting)
			continue;
		if(d100() < rotting)
			e.damage();
	}
}

static void auto_activate_features() {
	point m;
	for(m.y = 0; m.y < area->mps; m.y++)
		for(m.x = 0; m.x < area->mps; m.x++) {
			auto f = area->features[m];
			if(f == 0)
				continue;
			auto p = bsdata<featurei>::elements + (int)f;
			if(!p->autoactivated())
				continue;
			if(d100() < p->chance_auto_activate)
				area->setfeature(m, bsid(p->getactivate()));
		}
}

static bool checkalive() {
	if(!player || !(*player))
		return false;
	return true;
}

static void remove_flags(areaf v, int chance) {
	point m;
	for(m.y = 0; m.y < area->mps; m.y++) {
		for(m.x = 0; m.x < area->mps; m.x++) {
			if(!area->is(m, v))
				continue;
			if(d100() >= chance)
				continue;
			area->remove(m, v);
		}
	}
}

void pass_minute() {
	game.minutes++;
	update_all_boost(game.getminutes());
	all(creature_every_minute);
	while(game.restore_half_turn < game.minutes) {
		all(creature_every_5_minutes);
		game.restore_half_turn += 5;
		remove_flags(Iced, 20);
	}
	while(game.restore_turn < game.minutes) {
		all(creature_every_10_minutes);
		game.restore_turn += 10;
	}
	while(game.restore_hour < game.minutes) {
		game.restore_hour = (game.restore_hour / 60 + 1) * 60 + rand() % 60;
		update_need();
		update_relations();
	}
	while(game.restore_day_part < game.minutes) {
		all(creature_every_4_hours);
		decoy_food();
		game.restore_day_part = (game.restore_day_part / (60 * 4) + 1) * (60 * 4) + rand() % (60 * 4);
	}
	while(game.restore_day < game.minutes) {
		auto_activate_features();
		game.restore_day = (game.restore_day / (60 * 24) + 1) * (60 * 24) + rand() % (60 * 24);
	}
	while(game.restore_several_days < game.minutes) {
		all_creatures(EverySeveralDaysForP1);
		all_features(EverySeveralDaysForP1);
		fire_trigger(EverySeveralDays);
		game.restore_several_days += xrand(60 * 24 * 2, 60 * 24 * 6);
	}
}

static void skip_long_time() {
	auto human = game.getowner();
	if(!human)
		return;
	while(human->isunaware()) {
		allnext(make_move_long);
		pass_minute();
	}
}

static void play_minute() {
	const int moves_per_minute = 10 * 4;
	bool need_continue = true;
	while(need_continue) {
		need_continue = true;
		for(auto i = 0; i < moves_per_minute; i++) {
			update_los();
			allnext(make_move);
			if(!game.getowner() || draw::isnext()) {
				need_continue = false;
				break;
			}
		}
		pass_minute();
		skip_long_time();
	}
}

void play_game() {
	while(checkalive() && !draw::isnext())
		play_minute();
}

void end_game() {
	dialog_message(getdescription("LoseGame"));
	next_phase(main_menu);
}

void main_menu() {
	save_log();
}

int get_positive_count(variant v) {
	if(v.counter < 1)
		return 1;
	return v.counter;
}

static void initialize_globals() {
	for(auto& e : bsdata<globali>())
		game.globals[bsid(&e)] = e.current;
}

static void initialize_rumors(int count) {
	point pt;
	quest::add(KillBossQuest, start_village);
	const int r = 6;
	for(auto i = 0; i < count; i++) {
		pt.x = xrand(start_village.x - r, start_village.x + r);
		pt.y = xrand(start_village.x - r, start_village.x + r);
		quest::add(KillBossQuest, pt);
	}
}

static void random_world() {
	game.clear();
	initialize_globals();
	initialize_rumors(xrand(4, 7));
	prepare_need();
}

void new_game() {
	random_world();
	enter_area(start_village, 0, bsdata<featurei>::find("StairsDown"), NorthEast);
}