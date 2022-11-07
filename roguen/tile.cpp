#include "areamap.h"
#include "main.h"

BSDATA(tilei) = {
	{"NoTile"},
	{"WoodenFloor", {130, 104, 86}, {0, 1}, {}},
	{"Cave", {89, 97, 101}, {9, 3}, {14, 3}, 1},
	{"DungeonFloor", {89, 97, 101}, {12, 8}, {14, 3}, 3},
	{"Grass", {60, 176, 67}, {1, 4}, {0, 8}, 0},
	{"GrassCorupted", {66, 63, 48}, {5, 4}, {8, 6}, 2},
	{"Rock", {96, 71, 52}, {44, 1}},
	{"Sand", {}, {35, 4}},
	{"Snow", {}, {31, 4}},
	{"Lava", {}, {28, 3}, {}},
	{"Water", {}, {40, 4}},
	{"DarkWater", {22, 68, 59}, {45, 8}},
	{"DeepWater", {29, 80, 120}, {53, 8}},
	{"WallCave", {}, {}, {}, -1, Cave, {0, 2}},
	{"WallBuilding", {}, {}, {}, -1, WoodenFloor, {13, 1}},
	{"WallDungeon", {}, {}, {}, -1, DungeonFloor, {25, 3}},
	{"WallFire", {}, {}, {}, -1, Lava, {39, 5}},
	{"WallIce", {}, {}, {}, -1, Lava, {55, 3}},
};
assert_enum(tilei, WallIce)