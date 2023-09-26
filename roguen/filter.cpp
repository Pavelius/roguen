#include "creature.h"
#include "filter.h"
#include "itema.h"

static bool filter_wounded(const void* object) {
	auto p = (creature*)object;
	auto n = p->abilities[Hits];
	return n > 0 && n < p->basic.abilities[Hits];
}

static bool filter_unaware(const void* object) {
	auto p = (creature*)object;
	return p->isunaware();
}

static bool filter_cursed(const void* object) {
	auto p = (item*)object;
	return p->iscursed();
}

static bool filter_blessed(const void* object) {
	auto p = (item*)object;
	return p->is(Blessed);
}

BSDATA(filteri) = {
	{"FilterBlessed", filter_blessed, &items},
	{"FilterCursed", filter_cursed, &items},
	{"FilterUnaware", filter_unaware, &targets},
	{"FilterWounded", filter_wounded, &targets},
};
BSDATAF(filteri)