#include "stringact.h"
#include "main.h"

void nameable::actv(stringbuilder& sb, const char* format, const char* format_param) {
	stringact sa(sb, getname(), getgender());
	sa.addv(format, format_param);
	sb = sa;
}

gender_s nameable::getgender() const {
	if(bsdata<creature>::have(this))
		return static_cast<const creature*>(this)->gender;
	return Male;
}