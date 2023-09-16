#include "global.h"
#include "script.h"

BSDATAD(globali);

globali*	last_global;
extern int	last_value;

template<> void ftscript<globali>(int value, int counter) {
	last_global = bsdata<globali>::elements + value;
	last_value = last_global->current;
	last_global->set(last_value + counter);
}

void globali::set(int v) {
	if(v < minimum)
		v = minimum;
	if(v > maximum)
		v = maximum;
	if(current == v)
		return;
	current = v;
	if(v == minimum)
		script::run(fail);
	else if(v == maximum)
		script::run(effect);
}