#include "nameable.h"
#include "variant.h"

#pragma once

struct listi : nameable {
	variants elements;
	bool is(variant v) const { for(auto e : elements) if(e == v) return true; return false; }
};
