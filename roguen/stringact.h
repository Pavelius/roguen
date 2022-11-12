#include "stringbuilder.h"

#pragma once

struct stringact : stringbuilder {
	const char*		name;
	bool			female;
	void*			object;
	stringact(const stringbuilder& v, const char* name, bool female, void* object = 0) : stringbuilder(v), name(name), female(female), object(object) {}
	void			addidentifier(const char* identifier) override;
};
struct textscript {
	const char*		id;
	fnprint			proc;
};
