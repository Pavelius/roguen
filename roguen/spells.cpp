#include "main.h"

BSDATA(spelli) = {
	{"Gate", You, Instant},
	{"Light", You, Hour1PL},
	{"Sleep", You, Minute20},
	{"Teleport", You, Instant},
	{"Web", EnemyOrAllyNear, Instant},
};
assert_enum(spelli, Web)

bool creature::apply(spell_s v, int level, bool run) {
	auto& ei = bsdata<spelli>::elements[v];
	if(ei.duration)
		apply(v, bsdata<durationi>::elements[ei.duration].get(level));
	return true;
}

