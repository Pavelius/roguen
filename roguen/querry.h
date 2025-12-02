#pragma once

#include "collection.h"
#include "nameable.h"

typedef void(*fnevent)();

struct querryi : nameable {
	fnevent	proc;
};
extern collectiona records;

template<class T> bool fnfilter(const void* object, int value);

bool querry_allow(const void* object);
