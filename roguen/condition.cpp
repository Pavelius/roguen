#include "bsreq.h"
#include "condition.h"
#include "script.h"

BSMETA(conditioni) = {
	BSREQ(id),
	{}};

bool script_pass_condition(int value, int bonus) {
	return bsdata<conditioni>::elements[value].proc() == (bonus >= 0);
}

template<> void fiscript<conditioni>(int value, int bonus) {
	if(!script_pass_condition(value, bonus))
		script_stop();
}