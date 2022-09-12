#include "areamap.h"
#include "bsreq.h"

BSDATA(tilei) = {
	{"NoTile"},
	{"WoodenFloor", {0, 1}, {}},
	{"Grass", {1, 4}, {0, 8}},
	{"GrassCorupted", {5, 4}},
	{"Lava"},
	{"Water"},
};
assert_enum(tilei, Water)