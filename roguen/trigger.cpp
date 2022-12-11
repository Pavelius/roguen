#include "bsreq.h"
#include "pushvalue.h"
#include "trigger.h"
#include "script.h"

BSMETA(triggerni) = {
	BSREQ(id),
	{}};
BSMETA(trigger) = {
	BSENM(type, triggerni),
	BSREQ(p1),
	BSREQ(p2),
	BSREQ(effect),
	{}};
trigger* last_trigger;

void trigger::fire(triggern type, variant p1, variant p2) {
	for(auto& e : bsdata<trigger>()) {
		if(e.type == type
			&& (!e.p1 || e.p1 == p1)
			&& (!e.p2 || e.p2 == p2)) {
			pushvalue last_push(last_trigger, &e);
			script::run(e.effect);
		}
	}
}