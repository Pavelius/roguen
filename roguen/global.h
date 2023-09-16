#include "nameable.h"
#include "variant.h"

#pragma once

struct globali : nameable {
	int			minimum, maximum, current;
	variants	effect, fail;
	int			get() const { return current; }
	void		set(int v);
};
extern globali* last_global;
