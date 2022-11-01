#include "listcolumn.h"

listcolumn* current_columns;

int listcolumn::totalwidth() const {
	auto result = 0;
	for(auto p = this; *p; p++)
		result += p->width;
	return result;
}