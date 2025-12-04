#pragma once

#include "nameable.h"

typedef bool(*fnvisible)(const void* object);

struct filteri : nameable {
	fnvisible proc;
};