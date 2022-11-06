#include "bsreq.h"
#include "crt.h"
#include "duration.h"

BSMETA(durationi) = {
	BSREQ(id),
	BSREQ(value), BSREQ(per_level),
	{}};
BSDATA(durationi) = {
	{"Instant"},
	{"Minute10", 10},
	{"Minute10PL", 0, 10},
	{"Minute20", 20},
	{"Minute20PL", 0, 20},
	{"Minute30", 30},
	{"Hour1", 60},
	{"Hour2", 60 * 2},
	{"Hour4", 60 * 4},
	{"Hour1PL", 0, 60},
	{"Day1", 60 * 24},
};
assert_enum(durationi, Day1)