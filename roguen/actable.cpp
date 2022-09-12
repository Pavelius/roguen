#include "stringact.h"
#include "main.h"

void actable::actv(stringbuilder& sb, const char* format, const char* format_param) {
	stringact sa(sb, getname(), getgender());
	sa.addv(format, format_param);
	sb = sa;
}

gender_s actable::getgender() const {
	if(bsdata<creature>::have(this))
		return static_cast<const creature*>(this)->gender;
	return Male;
}