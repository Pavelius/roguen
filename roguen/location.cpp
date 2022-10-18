#include "main.h"

void location::clear() {
	memset(this, 0, sizeof(*this));
}

void location::settile(const char* id) {
	stringbuilder sb(tile);
	sb.add(id);
}