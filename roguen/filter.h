#pragma once

#include "collection.h"
#include "nameable.h"

struct filteri : nameable {
	typedef void(*fnaction)(fnvisible proc, int counter);
	fnvisible	proc;
	fnaction	action;
};
struct querryi : nameable {
	fnevent	proc, prepare;
};