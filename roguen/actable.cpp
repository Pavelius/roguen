#include "stringact.h"
#include "main.h"

void actable::actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female, char separator) const {
	if(!name)
		name = getname();
	stringact sa(sb, getname(), female);
	sa.addsep(separator);
	sa.addv(format, format_param);
	sb = sa;
}

void actable::sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const {
	if(!name)
		name = getname();
	stringact sa(sb, getname(), female);
	sa.addn("[%1] \"", name);
	sa.addv(format, format_param);
	sa.add("\"");
	sb = sa;
}