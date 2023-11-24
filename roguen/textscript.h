#include "stringbuilder.h"

#pragma once

struct textscript {
	const char*		id;
	fnprint			proc;
};
extern const char* last_name;
extern bool	last_female;

void string_initialize();
