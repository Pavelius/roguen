#include "areamap.h"
#include "boost.h"
#include "greatneed.h"
#include "game.h"
#include "script.h"
#include "site.h"
#include "triggern.h"
#include "main.h"

areamap			area;
areaheadi		areahead;
gamei			game;
static char		console_text[4096];
stringbuilder	console(console_text);
point			gamei::start_village = {128, 128};

void update_need();

static int getbase(const char* id) {
	auto p = varianti::getsource(id);
	if(!p)
		return -1;
	return p - bsdata<varianti>::elements;
}

static void all(funct<creature>::command proc) {
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid())
			(e.*proc)();
	}
}

static void allnext(funct<creature>::command proc) {
	if(draw::isnext())
		return;
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid())
			(e.*proc)();
		if(draw::isnext())
			break;
	}
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
	auto& source = get_triggers(type, iscreature);
	for(auto& e : bsdata<creature>()) {
		if(!e.isvalid())
			continue;
		auto v1 = &e;
		for(auto p : source) {
			if(!p->match(v1, {}))
				continue;
			e.apply(p->effect);
		}
	}
}

static void all_features(triggern type) {
	point m;
	pushvalue push_point(last_rect);
	auto& source = get_triggers(type, isfeature);
	for(m.y = 0; m.y < area.mps; m.y++) {
		for(m.x = 0; m.x < area.mps; m.x++) {
			if(area.features[m] == 0)
				continue;
			auto v1 = bsdata<featurei>::elements + (int)area.features[m];
			for(auto p : source) {
				if(!p->match(v1, {}))
					continue;
				last_rect = m.rectangle();
				script::run(p->effect);
			}
		}
	}
}

static bool isfreelt(point m) {
	if(area.is(m, Darkened))
		return false;
	return area.isfree(m);
}

bool isfreeltsv(point m) {
	area.setflag(m, Visible);
	area.setflag(m, Explored);
	if(area.is(m, Darkened))
		return false;
	return area.isfree(m);
}

static void update_los() {
	point m;
	for(m.y = 0; m.y < area.mps; m.y++)
		for(m.x = 0; m.x < area.mps; m.x++) {
			area.remove(m, Visible);
			if(area.is(m, Darkened))
				area.remove(m, Explored);
		}
	auto human = game.getowner();
	if(human) {
		auto i = human->getposition();
		if(area.isvalid(i))
			area.setlos(i, human->getlos(), isfreeltsv);
	}
}

static void decoy_food() {
	for(auto& e : bsdata<itemground>()) {
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
	for(m.y = 0; m.y < area.mps; m.y++)
		for(m.x = 0; m.x < area.mps; m.x++) {
			auto f = area.features[m];
			if(f==0)
				continue;
			auto p = bsdata<featurei>::elements + (int)f;
			if(!p->autoactivated())
				continue;
			if(d100() < p->chance_auto_activate)
				area.setfeature(m, bsid(p->getactivate()));
		}
}

static bool checkalive() {
	if(!player || !(*player))
		return false;
	return true;
}

static void remove_flags(areaf v, int chance) {
	point m;
	for(m.y = 0; m.y < area.mps; m.y++) {
		for(m.x = 0; m.x < area.mps; m.x++) {
			if(!area.is(m, v))
				continue;
			if(d100() >= chance)
				continue;
			area.remove(m, v);
		}
	}
}

int gamei::getrange(point m1, point m2) {
	auto dx = iabs(m1.x - m2.x);
	auto dy = iabs(m1.y - m2.y);
	return (dx > dy) ? dx : dy;
}

void gamei::passminute() {
	minutes++;
	boosti::updateall(getminutes());
	all(&creature::everyminute);
	while(restore_half_turn < minutes) {
		all(&creature::every5minutes);
		restore_half_turn += 5;
		remove_flags(Iced, 20);
	}
	while(restore_turn < minutes) {
		all(&creature::every10minutes);
		restore_turn += 10;
	}
	while(restore_hour < minutes) {
		all(&creature::every1hour);
		restore_hour = (restore_hour / 60 + 1) * 60 + rand() % 60;
		update_need();
	}
	while(restore_day_part < minutes) {
		decoy_food();
		restore_day_part = (restore_day_part / (60 * 4) + 1) * (60 * 4) + rand() % (60 * 4);
	}
	while(restore_day < minutes) {
		auto_activate_features();
		restore_day = (restore_day / (60 * 24) + 1) * (60 * 24) + rand() % (60 * 24);
	}
	while(restore_several_days < minutes) {
		all_creatures(EverySeveralDaysForP1);
		all_features(EverySeveralDaysForP1);
		trigger::fire(EverySeveralDays);
		restore_several_days += xrand(60 * 24 * 2, 60 * 24 * 6);
	}
}

static void skip_long_time() {
	auto human = game.getowner();
	if(!human)
		return;
	while(human->isunaware()) {
		allnext(&creature::makemovelong);
		game.passminute();
	}
}

void gamei::playminute() {
	const int moves_per_minute = 6 * 4;
	bool need_continue = true;
	while(need_continue) {
		need_continue = true;
		for(auto i = 0; i < moves_per_minute; i++) {
			update_los();
			allnext(&creature::makemove);
			if(!game.getowner() || draw::isnext()) {
				need_continue = false;
				break;
			}
		}
		passminute();
		skip_long_time();
	}
}

void gamei::play() {
	while(checkalive() && !draw::isnext())
		game.playminute();
}

void gamei::endgame() {
	dialog_message(getdescription("LoseGame"));
	next(game.mainmenu);
}

void gamei::mainmenu() {
	writelog();
}

int gamei::getcount(variant v, int minimal) {
	auto count = v.counter;
	if(count < 0 && d100() >= -count)
		return -1;
	if(count < minimal)
		count = minimal;
	return count;
}

int gamei::getpositivecount(variant v) {
	if(v.counter < 1)
		return 1;
	return v.counter;
}

void gamei::setup_globals() {
	for(auto& e : bsdata<globali>())
		globals[bsid(&e)] = e.current;
}

void gamei::setup_rumors(int count) {
	point pt;
	quest::add(KillBossQuest, start_village);
	const int r = 6;
	for(auto i = 0; i < count; i++) {
		pt.x = xrand(start_village.x - r, start_village.x + r);
		pt.y = xrand(start_village.x - r, start_village.x + r);
		quest::add(KillBossQuest, pt);
	}
}

void gamei::clear() {
	memset(this, 0, sizeof(*this));
}

void prepare_need();

void gamei::randomworld() {
	clear();
	setup_globals();
	setup_rumors(xrand(4, 7));
	prepare_need();
}

void gamei::set(const globali& e, int v) {
	if(v < e.minimum)
		v = e.minimum;
	if(v > e.maximum)
		v = e.maximum;
	if(get(e) == v)
		return;
	globals[bsid(&e)] = v;
	if(v == e.minimum)
		script::run(e.fail);
	else if(v == e.maximum)
		script::run(e.effect);
}

const char* gamei::timeleft(unsigned end_stamp) const {
	auto stamp = getminutes();
	if(end_stamp <= stamp)
		return getnm("FewTime");
	auto value = (end_stamp - stamp) / (24 * 60);
	if(value > 0)
		return str("%1i %-2", value, stringbuilder::getbycount("Day", value));
	value = (end_stamp - stamp) / 60;
	if(value > 0)
		return str("%1i %-2", value, stringbuilder::getbycount("Hour", value));
	value = end_stamp - stamp;
	return str("%1i %-2", value, stringbuilder::getbycount("Minute", value));
}