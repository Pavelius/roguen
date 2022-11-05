#include "nameable.h"
#include "variant.h"

#pragma once

enum trigger_s : unsigned char;

struct triggeri : nameable {
};
struct trigger {
	trigger_s		type;
	variant			p1, p2;
	variants		effect;
	static void		fire(trigger_s t, variant p1 = {}, variant p2 = {});
};
extern trigger*		last_trigger;
