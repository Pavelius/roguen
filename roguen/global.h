#include "nameable.h"
#include "variant.h"

#pragma once

struct globali : nameable {
	int			minimum, maximum, current;
	variants	effect, fail;
};
