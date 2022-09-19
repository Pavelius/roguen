#include "variant.h"

#pragma once

struct speech {
	const char*		id;
	variants		condition;
	const char*		name;
	void			clear() { memset(this, 0, sizeof(*this)); }
	static void		read(const char* url);
};
struct speecha : adat<speech*> {
	const char*		getrandom() const;
	void			select(const char* id);
};