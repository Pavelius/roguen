#include "main.h"

BSDATA(featurei) = {
	{"NoFeature"},
	{"Tree", {7, 4}, {11, 3}, 10, FG(Impassable) | FG(Woods)},
	{"FootMud", {14, 1}, {}, 5},
	{"FootHill", {15, 1}, {}, 5},
	{"Grave", {16, 2}},
	{"Statue", {43, 2}},
	{"HiveHole", {18, 1}, {}, 5},
	{"Hive", {19, 1}},
	{"Hole", {20, 1}},
	{"Plant", {21, 3}, {}, 10},
	{"Herbs", {24, 3}, {}, 6},
	{"Trap", {27, 12}, {}, 4},
	{"Door", {39, 1}, {}, 7, FG(BetweenWalls) | FG(ImpassableNonActive) | FG(AllowActivate)},
};
assert_enum(featurei, Door)