#include "areamap.h"
#include "bsreq.h"

BSDATA(tilei) = {
	{"NoTile"},
	{"WoodenFloor", {0, 1}, {}},
	{"Cave", {9, 3}, {}, 1},
	{"Grass", {1, 4}, {0, 8}, 0},
	{"GrassCorupted", {5, 4}, {}, 2},
	{"Lava"},
	{"Water"},
	{"WallCave", {}, {}, -1, Cave, {0, 2}},
};
assert_enum(tilei, WallCave)