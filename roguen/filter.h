#include "collection.h"
#include "nameable.h"

#pragma once

struct filteri : nameable {
	fnvisible		proc;
	collectiona*	source;
};