#include "bsreq.h"
#include "condition.h"
#include "script.h"

BSMETA(conditioni) = {
	BSREQ(id),
	{}};

template<> bool fitest<conditioni>(int value, int bonus) {
	return bsdata<conditioni>::elements[value].proc() == (bonus >= 0);
}
template<> void fiscript<conditioni>(int value, int bonus) {
	if(!fitest<conditioni>(value, bonus))
		script_stop();
}