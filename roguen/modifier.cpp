#include "crt.h"
#include "modifier.h"
#include "script.h"

BSDATA(modifieri) = {
	{"NoModifier"},
	{"InPlayerBackpack"},
};
assert_enum(modifieri, InPlayerBackpack)

modifiers modifier;

template<> void ftscript<modifieri>(int value, int counter) {
	modifier = (modifiers)value;
}
template<> bool fttest<modifieri>(int value, int counter) {
	ftscript<modifieri>(value, counter);
	return true;
}