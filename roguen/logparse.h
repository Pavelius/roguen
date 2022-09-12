#include "log.h"
#include "variant.h"

#pragma once

namespace log {
struct valuei {
	const char*	text;
	long		number;
	void*		data;
	variant		identifier;
	void clear() { memset(this, 0, sizeof(*this)); }
	const char* read(const char* p, stringbuilder& sb);
};
bool checksym(const char* p, char sym);
const char* getstring(stringbuilder& sb);
const char* readbon(const char* p, int& bonus);
const char* readidn(const char* p, stringbuilder& result);
const char* readname(const char* p, stringbuilder& result);
const char*	readstr(const char* p, stringbuilder& result);
const char*	readval(const char* p, stringbuilder& sb, variant& result);
}