#include "modifier.h"
#include "script.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"InPlayerBackpack"},
	{"InPosition"},
	{"InRoom"},
};
assert_enum(modifieri, InRoom)