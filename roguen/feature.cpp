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

featurei* featurei::getlocked() const {
	auto alternative = activateto;
	for(auto& e : bsdata<featurei>()) {
		if(!e.isvisible())
			continue;
		if(e.activate_item) {
			if(e.activateto == this)
				return &e;
			if(alternative && e.activateto == alternative)
				return &e;
		}
	}
	return 0;
}