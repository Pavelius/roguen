#include "gender.h"
#include "variant.h"

#pragma once

struct nameable {
	variant		kind; // Race or monster
	gender_s	gender;
	void		actv(stringbuilder& sb, const char* format, const char* format_param);
	const char*	getname() const { return kind.getname(); }
};
