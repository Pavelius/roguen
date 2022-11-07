#include "variant.h"

#pragma once

struct advancement {
	variant		type;
	char		level;
	variants	elements;
};
