#include "bsreq.h"
#include "diceprogress.h"

BSMETA(diceprogress) = {
	BSREQ(min),
	BSREQ(max),
	BSREQ(multiplier),
	BSREQ(divider),
	BSREQ(bound),
	{}};

int diceprogress::roll(int level) const {
	if(bound && level > bound)
		level = bound;
	int v = 0;
	if(multiplier)
		v += level * multiplier;
	if(divider)
		v = v / divider;
	return v + dice::roll();
}