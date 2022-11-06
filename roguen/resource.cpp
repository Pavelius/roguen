#include "main.h"
#include "resource.h"

BSDATA(resource) = {
	{"monsters"},
	{"borders"},
	{"floor"},
	{"walls"},
	{"decals"},
	{"features"},
	{"shadows"},
	{"items"},
	{"attack"},
	{"conditions"},
	{"splash"},
	{"fow"},
	{"los"},
	{"missiles"},
	{"pc_body"},
	{"pc_arms"},
	{"pc_accessories"},
};
assert_enum(resource, (int)res::PCAccessories)