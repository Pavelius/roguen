#include "nameable.h"
#include "variant.h"

#pragma once

enum triggern : unsigned char;

struct triggerni : nameable {
};
struct trigger : nameable {
	triggern type;
	variants effect;
};

void fire_trigger(triggern type);
