#include "bsreq.h"
#include "crt.h"
#include "feature.h"

NOBSDATA(color)
NOBSDATA(framerange)

BSMETA(featurei) = {
	BSREQ(id),
	BSREQ(features), BSREQ(overlay),
	BSREQ(priority),
	BSREQ(movedifficult),
	BSFLG(flags, tilefi),
	BSREQ(minimap),
	BSREQ(activate_item),
	BSREQ(random_count),
	BSREQ(leadto), BSREQ(activateto), BSREQ(activate_script),
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

featurei* featurei::getlocked() const {
	for(auto& e : bsdata<featurei>()) {
		if(e.activateto == this && e.isvisible()) {
			if(e.activate_item)
				return &e;
		}
	}
	return 0;
}