#include "bsreq.h"
#include "crt.h"
#include "feature.h"

NOBSDATA(color)
NOBSDATA(framerange)

BSMETA(featurei) = {
	BSREQ(id),
	BSREQ(features), BSREQ(overlay),
	BSREQ(priority),
	BSFLG(flags, tilefi),
	BSREQ(minimap),
	BSREQ(leadto), BSREQ(activateto),
	BSREQ(lead),
	BSREQ(chance_auto_activate),
	{}};
BSDATAC(featurei, 250);

featurei* featurei::gethidden() const {
	for(auto& e : bsdata<featurei>()) {
		if(!e.isvisible() && e.activateto == this)
			return &e;
	}
	return 0;
}