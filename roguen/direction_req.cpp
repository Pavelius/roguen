#include "bsreq.h"
#include "direction.h"

BSMETA(directioni) = {
	BSREQ(id)
};
BSDATA(directioni) = {
	{"North"},
	{"East"},
	{"South"},
	{"West"},
	{"NorthEast"},
	{"SouthEast"},
	{"SouthWest"},
	{"NorthWest"},
};
assert_enum(directioni, NorthWest)
