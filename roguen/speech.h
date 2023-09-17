#include "variant.h"

#pragma once

struct speech {
	const char*		id;
	const char*		name;
	variants		condition;
	void			clear() { memset(this, 0, sizeof(*this)); }
	static void		read(const char* url);
};
struct speecha : adat<speech*> {
	const char*		random() const;
	void			select(const char* id);
};