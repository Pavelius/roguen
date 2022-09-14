#include "stringbuilder.h"

#pragma once

struct stringact : stringbuilder {
	const char*	name;
	bool		female;
	stringact(const stringbuilder& v, const char* name, bool female) : stringbuilder(v), name(name), female(female) {}
	void		addidentifier(const char* identifier) override;
};
