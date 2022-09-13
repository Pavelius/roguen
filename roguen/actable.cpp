#include "stringact.h"
#include "main.h"

void actable::actv(stringbuilder& sb, const char* format, const char* format_param, char separator) {
	stringact sa(sb, getname(), getgender());
	sa.addsep(separator);
	sa.addv(format, format_param);
	sb = sa;
}