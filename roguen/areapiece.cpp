#include "areapiece.h"

areapiece* area;

void areapiece::clear() {
	memset(this, 0, sizeof(*this));
	rooms.element_size = sizeof(roomi);
	items.element_size = sizeof(itemground);
}