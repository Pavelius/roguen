#include "boost.h"
#include "calendar.h"
#include "creature.h"
#include "draw.h"
#include "functor.h"
#include "pushvalue.h"
#include "trigger.h"
#include "triggern.h"
#include "script.h"

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
	for(m.y = 0; m.y < area->mps; m.y++) {
		for(m.x = 0; m.x < area->mps; m.x++) {
			if(area->features[m] == 0)
				continue;
			auto v1 = bsdata<featurei>::elements + (int)area->features[m];
			for(auto p : source) {
				if(!p->match(v1, {}))
					continue;
				last_rect = m.rectangle();
				script::run(p->effect);
			}
		}
	}
}

static collection<trigger>& get_triggers(triggern type, fnvisible proc) {
	static collection<trigger> source;
	source.select();
	source.match(istriggertype, type, true);
	source.match(proc, true);
	return source;
}

void calendari::passminute() {
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