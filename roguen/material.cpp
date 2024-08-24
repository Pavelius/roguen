#include "crt.h"
#include "material.h"

BSDATA(materiali) = {
	{"Wood"},
	{"Metal"},
	{"Paper"},
	{"Stone"},
	{"Leather"},
	{"Glass"},
	{"Ivory"},
	{"Organic"},
};
assert_enum(materiali, Organic)