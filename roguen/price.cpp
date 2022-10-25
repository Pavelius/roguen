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
	{"VeryHighCost", 170},
	{"ExpenciveCost", 200},
	{"SuperExpenciveCost", 300},
};
BSDATAF(pricei)