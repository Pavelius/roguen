#include "crt.h"

#pragma once

class dataset : public adat<short unsigned> {
public:
	void select(array& source);
	void select(array& source, fnvisible proc);
};
