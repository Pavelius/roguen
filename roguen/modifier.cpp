#include "crt.h"
#include "modifier.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"Permanent"},
};
assert_enum(modifieri, Permanent)

modifiers modifier;