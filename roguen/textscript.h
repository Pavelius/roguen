#include "stringbuilder.h"

#pragma once

struct textscript {
	const char*		id;
	fnprint			proc;
	static void		initialize();
};
extern const char* last_name;
extern bool	last_female;
