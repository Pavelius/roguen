#include "main.h"
#include "resource.h"

BSDATA(resource) = {
	{"monsters"},
	{"floor"},
	{"decals"},
	{"features"},
	{"pc_body"},
	{"pc_arms"},
	{"pc_accessories"},
};
assert_enum(resource, (int)res::PCAccessories)