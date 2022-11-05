#include "bsreq.h"
#include "pushvalue.h"
#include "trigger.h"
#include "script.h"

trigger* last_trigger;

void trigger::fire(trigger_s type, variant p1, variant p2) {
	for(auto& e : bsdata<trigger>()) {
		if(e.type == type
			&& (!e.p1 || e.p1 == p1)
			&& (!e.p2 || e.p2 == p2)) {
			pushvalue last_push(last_trigger, &e);
			runscript(e.effect);
		}
	}
}