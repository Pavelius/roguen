#include "crt.h"
#include "magic.h"

BSDATA(magici) = {
	{"Mundane"},
	{"Blessed"},
	{"Cursed"},
	{"Artifact"},
};
assert_enum(magici, Artifact)