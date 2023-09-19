#include "bsreq.h"
#include "price.h"

BSMETA(pricei) = {
	BSREQ(id),
	BSREQ(percent),
	{}};
BSDATA(pricei) = {
	{"NoCost"},
	{"CheapCost", 80},
	{"LowCost", 90},
	{"NormalCost", 100},
	{"HighCost", 130},
	{"VeryHighCost", 200},
	{"ExpenciveCost", 300},
	{"SuperExpenciveCost", 400},
};
BSDATAF(pricei)