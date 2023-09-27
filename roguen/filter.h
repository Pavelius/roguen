#include "collection.h"
#include "nameable.h"

#pragma once

struct filteri : nameable {
	typedef void(*fnaction)(fnvisible proc, int counter);
	fnvisible	proc;
	fnaction	action;
};