#pragma once

#include "nameable.h"
#include "variant.h"

int	random_total(const variants& elements);

variant random_value(const variants& elements, int bonus, int range);
inline variant random_value(const variants& elements) { return random_value(elements, 0, random_total(elements)); }
variant single(variant v);

struct randomizeri : nameable {
	variants	chance;
	bool		is(variant v) const { for(auto e : chance) if(e.issame(v)) return true; return false; }
	variant		random() const { return random_value(chance); }
};
