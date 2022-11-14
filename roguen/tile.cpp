#include "bsreq.h"
#include "tile.h"

NOBSDATA(color)
NOBSDATA(framerange)

BSMETA(tilefi) = {
	BSREQ(id),
	{}};
BSMETA(tilei) = {
	BSREQ(id),
	BSREQ(minimap),
	BSREQ(floor), BSREQ(decals), BSREQ(walls),
	BSREQ(borders),
	BSENM(tile, tilei),
	BSFLG(flags, tilefi),
	{}};

BSDATA(tilefi) = {
	{"Impassable"},
	{"CanSwim"},
	{"DangerousFeature"},
	{"BetweenWalls"},
	{"Woods"},
};
assert_enum(tilefi, Woods)