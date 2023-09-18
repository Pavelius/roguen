#include "crt.h"
#include "modifier.h"
#include "script.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"Permanent"},
	{"InPlayerBackpack"},
};
assert_enum(modifieri, InPlayerBackpack)

modifiers modifier;

template<> void ftscript<modifieri>(int value, int counter) {
	modifier = (modifiers)value;
}