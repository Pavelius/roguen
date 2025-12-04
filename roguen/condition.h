#pragma once

#include "nameable.h"

typedef bool(*fncondition)();

struct conditioni : nameable {
	fncondition proc;
};

bool script_pass_condition(int value, int bonus);
