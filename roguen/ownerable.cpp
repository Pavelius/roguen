#include "bsdata.h"
#include "ownerable.h"

creature* ownerable::getowner() const {
	return bsdata<creature>::ptr(owner_id);
}

void ownerable::setowner(const creature* v) {
	bsset(owner_id, v);
}