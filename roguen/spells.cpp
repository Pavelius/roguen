#include "main.h"

BSDATA(spelli) = {
	{"CureWounds", AllyClose, Instant},
	{"Gate", You, Instant},
	{"Light", You, Hour1PL},
	{"ManaRegeneration", You, Hour1PL},
	{"Regeneration", You, Hour1PL},
	{"Sleep", You, Minute20},
	{"Teleport", You, Instant},
	{"Web", EnemyOrAllyNear, Instant},
};
assert_enum(spelli, Web)

bool creature::apply(spell_s v, int level, bool run) {
	char temp[260]; stringbuilder sb(temp);
	auto& ei = bsdata<spelli>::elements[v];
	switch(v) {
	case CureWounds:
		if(is(NoWounded))
			return false;
		if(run)
			heal(ei.getcount(level));
		break;
	}
	if(ei.duration) {
		auto count = bsdata<durationi>::elements[ei.duration].get(level);
		sb.add("%1 %2i %-Minutes", ei.getname(), count);
		fixvalue(temp, 2);
		apply(v, count);
	}
	return true;
}

int	spelli::getcount(int level) const {
	return level + count.roll();
}