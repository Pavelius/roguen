#include "bsreq.h"
#include "crt.h"
#include "duration.h"
#include "nameable.h"

struct durationi : nameable {
};

BSDATA(durationi) = {
	{"Instant"},
	{"Minutes"},
	{"Hours"},
	{"Days"},
};
assert_enum(durationi, Days)

BSMETA(durationi) = {
	BSREQ(id),
	{}};

int get_duration(duration_s v, int level) {
	switch(v) {
	case Minutes: return 1 + level;
	case Hours: return (1 + level) * 60;
	case Days: return (1 + level) * 24 * 60;
	default: return 0;
	}
}