#pragma once

#include "collection.h"
#include "nameable.h"

typedef void(*fnevent)();
typedef bool(*fncondition)();

struct querryi : nameable {
	fnevent	proc;
	fncondition condition;
};
extern collectiona records;

bool querry_allow(const void* object);
bool querry_allow_all(const void* object);
void querry_filter();
bool querry_nobody();
bool querry_select();
