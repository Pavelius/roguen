#pragma once

#include "nameable.h"

typedef void(*fnevent)();
typedef bool(*fncondition)();
typedef bool(*fnvisible)(const void* drawobject);

struct filteri : nameable {
	fnvisible proc;
};
struct querryi : nameable {
	fnevent	proc;
	fncondition condition;
};
extern bool querry_fail;

bool querry_allow(const void* drawobject);
bool querry_allow_all(const void* drawobject);
void querry_filter();
bool querry_nobody();
bool querry_select();
