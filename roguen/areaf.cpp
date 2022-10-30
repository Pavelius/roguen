#include "areamap.h"
#include "bsreq.h"

BSDATA(areafi) = {
	{"Explored"},
	{"Visible"},
	{"Activated"},
	{"Hidden"},
	{"Darkened"},
	{"Blooded", {4, 3}},
	{"Iced", {3, 1}},
	{"Webbed", {0, 3}},
};
assert_enum(areafi, Webbed)