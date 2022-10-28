#include "main.h"

BSDATA(spelli) = {
	{"Gate"},
	{"Light", You},
	{"Sleep"},
	{"Teleport"},
	{"Web"},
};
assert_enum(spelli, Web)

bool creature::apply(spell_s v, int level, bool run) {
	apply(v, level * 60 * 2);
	return true;
}

