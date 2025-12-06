#pragma once

#include "nameable.h"

typedef void(*fnevent)();
typedef bool(*fncondition)();
typedef bool(*fnvisible)(const void* object);

struct filteri : nameable {
	fnvisible proc;
};
struct querryi : nameable {
	fnevent	proc;
	fncondition condition;
};
extern bool querry_fail;

bool querry_allow(const void* object);
bool querry_allow_all(const void* object);
void querry_filter();
bool querry_nobody();
bool querry_select();
