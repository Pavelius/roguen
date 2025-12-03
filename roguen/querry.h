#pragma once

#include "collection.h"
#include "nameable.h"

typedef void(*fnevent)();
typedef bool(*fnvisible)();

struct querryi : nameable {
	fnevent	proc;
	fnvisible condition;
};
extern collectiona records;

bool querry_allow(const void* object);
bool querry_allow_all(const void* object);
void querry_filter();
bool querry_nobody();
bool querry_select();
