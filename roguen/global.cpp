#include "archive.h"
#include "global.h"
#include "script.h"

globali* last_global;
extern int last_value;

template<> void archive::set<globali>(globali& e) {
	set(e.current);
}

template<> void fiscript<globali>(int value, int counter) {
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
		script_run(fail);
	else if(v == maximum)
		script_run(effect);
}