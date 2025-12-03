#pragma once

#include "collection.h"
#include "nameable.h"

typedef void(*fnevent)();

struct querryi : nameable {
	fnevent	proc;
};
extern collectiona records;

bool querry_allow(const void* object);
