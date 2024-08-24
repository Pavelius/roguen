#include "bsreq.h"
#include "global.h"

BSMETA(globali) = {
	BSREQ(id),
	BSREQ(minimum), BSREQ(maximum), BSREQ(current),
	BSREQ(effect), BSREQ(fail),
	{}};
BSDATAC(globali, 128);