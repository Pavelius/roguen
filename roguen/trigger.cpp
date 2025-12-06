#include "bsreq.h"
#include "pushvalue.h"
#include "trigger.h"
#include "script.h"

BSMETA(triggerni) = {
	BSREQ(id),
	{}};
BSMETA(trigger) = {
	BSREQ(id),
	BSENM(type, triggerni),
	BSREQ(effect),
	{}};

void fire_trigger(triggern type) {
	for(auto& e : bsdata<trigger>()) {
		if(e.type == type)
			script_run(e.effect);
	}
}