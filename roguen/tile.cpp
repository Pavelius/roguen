#include "areamap.h"
#include "bsreq.h"
#include "main.h"

BSDATA(tilei) = {
	{"NoTile"},
	{"WoodenFloor", {}, {0, 1}, {}},
	{"Cave", {}, {9, 3}, {}, 1},
	{"DungeonFloor", {}, {12, 8}, {}, 3},
	{"Grass", {60, 176, 67}, {1, 4}, {0, 8}, 0},
	{"GrassCorupted", {}, {5, 4}, {}, 2},
	{"Lava", {}, {28, 3}, {}},
	{"Water", {}},
	{"WallCave", {}, {}, {}, -1, Cave, {0, 2}},
	{"WallBuilding", {}, {}, {}, -1, WoodenFloor, {13, 1}},
	{"WallDungeon", {}, {}, {}, -1, DungeonFloor, {25, 3}},
	{"WallFire", {}, {}, {}, -1, Lava, {39, 5}},
	{"WallIce", {}, {}, {}, -1, Lava, {55, 3}},
};
assert_enum(tilei, WallIce)