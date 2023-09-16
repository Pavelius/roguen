#include "crt.h"
#include "modifier.h"
#include "script.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"Permanent"},
};
assert_enum(modifieri, Permanent)

modifiers modifier;

template<> void ftscript<modifieri>(int value, int counter) {
	modifier = (modifiers)value;
}