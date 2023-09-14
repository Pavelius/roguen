#include "areapiece.h"

void areapiece::clear() {
	memset(this, 0, sizeof(*this));
	rooms.size = sizeof(roomi);
	items.size = sizeof(itemground);
}