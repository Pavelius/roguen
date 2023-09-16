#include "areapiece.h"
#include "site.h"

void areaheadi::clear() {
	memset(this, 0, sizeof(*this));
}

variants sitei::getloot() const {
	if(area && area->level)
		return loot.left(4 + (iabs(area->level) - 1));
	return loot;
}