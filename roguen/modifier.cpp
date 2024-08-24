#include "crt.h"
#include "modifier.h"
#include "script.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"InPlayerBackpack"},
	{"InPosition"},
};
assert_enum(modifieri, InPosition)