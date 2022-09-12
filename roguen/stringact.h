#include "gender.h"
#include "stringbuilder.h"

#pragma once

struct stringact : stringbuilder {
	const char*	name;
	gender_s	gender;
	stringact(const stringbuilder& v, const char* name, gender_s gender) : stringbuilder(v), name(name), gender(gender) {}
	void		addidentifier(const char* identifier) override;
};
