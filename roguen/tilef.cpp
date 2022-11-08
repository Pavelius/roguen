#include "areamap.h"
#include "bsreq.h"

BSDATA(tilefi) = {
	{"Impassable"},
	{"CanSwim"},
	{"AllowActivate"},
	{"DangerousFeature"},
	{"BetweenWalls"},
	{"Woods"},
};
assert_enum(tilefi, Woods)