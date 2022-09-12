#include "main.h"

BSDATA(featurei) = {
	{"NoFeature"},
	{"Tree", {7, 4}, {11, 3}},
	{"FootMud", {14, 1}, {}, 5},
	{"FootHill", {15, 1}, {}, 5},
};
assert_enum(featurei, FootHill)