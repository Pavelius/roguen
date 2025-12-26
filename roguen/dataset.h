#pragma once

#include "adat.h"
#include "array.h"

typedef bool(*fnvisible)(const void* drawobject); // Callback function of checking some functionality of `object`

struct dataset : public adat<short unsigned> {
	void select(array& source);
	void select(array& source, fnvisible proc);
};
