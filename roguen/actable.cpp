#include "stringact.h"
#include "main.h"

void actable::actv(stringbuilder& sb, const char* format, const char* format_param, bool female, char separator) {
	stringact sa(sb, getname(), female);
	sa.addsep(separator);
	sa.addv(format, format_param);
	sb = sa;
}