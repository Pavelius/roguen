#pragma once

#include "nameable.h"

typedef bool(*fncondition)();

struct conditioni : nameable {
	fncondition proc;
};
