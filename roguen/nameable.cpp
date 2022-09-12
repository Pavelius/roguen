#include "stringact.h"
#include "nameable.h"

void nameable::actv(stringbuilder& sb, const char* format, const char* format_param) {
	stringact sa(sb, getname(), gender);
	sa.addv(format, format_param);
	sb = sa;
}