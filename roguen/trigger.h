#include "nameable.h"
#include "variant.h"

#pragma once

enum triggern : unsigned char;

struct triggerni : nameable {
};
struct trigger {
	triggern	type;
	variant		p1, p2;
	variants	effect;
	bool		match(variant v1, variant v2) const { return (!p1 || p1 == v1) && (!p2 || p2 == v2); }
};

void fire_trigger(triggern type, variant p1 = {}, variant p2 = {});
