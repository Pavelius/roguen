#include "craft.h"
#include "script.h"

craftn last_craft;

BSDATA(crafti) = {
	{"AlchemyCraft"},
	{"BlacksmithCraft"},
};
assert_enum(crafti, BlacksmithCraft)

template<> void ftscript<crafti>(int index, int bonus) {
	last_craft = (craftn)index;
}