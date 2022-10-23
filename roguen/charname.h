#include "variant.h"

#pragma once

struct charname {
	variant			conditions[3];
	const char*		name;
	bool			match(const slice<variant>& source) const;
	bool			match(variant v) const;
	static const char* getname(unsigned short v);
	static short unsigned random(const slice<variant>& source);
	static void		read(const char* url);
	static unsigned select(short unsigned* pb, short unsigned* pe, const slice<variant>& source);
};