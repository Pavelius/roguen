#include "nameable.h"

#pragma once

typedef bool (*fnifscript)(int bonus);
struct ifscripti : nameable {
	fnifscript			proc;
};