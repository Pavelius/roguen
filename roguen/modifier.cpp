#include "crt.h"
#include "modifier.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"OneOf"},
};
assert_enum(modifieri, OneOf)

modifiers modifier;