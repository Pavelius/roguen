#include "stringact.h"
#include "main.h"

void actable::actv(stringbuilder& sb, const char* format, const char* format_param) {
	stringact sa(sb, getname(), getgender());
	sa.addv(format, format_param);
	sb = sa;
}