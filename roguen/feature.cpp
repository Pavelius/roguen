#include "main.h"

BSDATA(featurei) = {
	{"NoFeature"},
	{"Tree", {7, 4}, {11, 3}, 10, FG(Impassable) | FG(Woods)},
	{"TreePalm", {101, 6}, {}, 10, FG(Impassable) | FG(Woods)},
	{"DeadTree", {45, 4}, {49, 3}, 10, FG(Impassable) | FG(Woods)},
	{"ThornBush", {98, 3}, {49, 3}, 10, FG(Impassable) | FG(Woods)},
	{"FootMud", {14, 1}, {}, 5},
	{"FootHill", {15, 1}, {}, 5},
	{"Grave", {16, 2}},
	{"Statue", {43, 2}, {}, 10, FG(Impassable)},
	{"HiveHole", {18, 1}, {}, 5},
	{"Hive", {19, 1}},
	{"Hole", {20, 1}},
	{"Plant", {21, 3}, {}, 10},
	{"Herbs", {24, 3}, {}, 6},
	{"AltarGood", {56, 1}, {}, 6},
	{"AltarNeutral", {57, 1}, {}, 6},
	{"AltarEvil", {58, 1}, {}, 6},
	{"Trap", {27, 12}, {}, 4},
	{"Door", {39, 1}, {}, 7, FG(BetweenWalls) | FG(ImpassableNonActive) | FG(AllowActivate)},
	{"StairsUp", {52, 1}, {}, 4},
	{"StairsDown", {53, 1}, {}, 4},
	{"GatePortal", {54, 2}, {}, 4, FG(AllowActivate)},
};
assert_enum(featurei, GatePortal)