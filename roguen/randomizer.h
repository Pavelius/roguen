#include "nameable.h"
#include "variant.h"

#pragma once

struct randomizeri : nameable {
	variants	chance;
	bool		is(variant v) const { for(auto e : chance) if(e.issame(v)) return true; return false; }
	static variant param(const variants& elements, int bonus, int range);
	static variant param(const variants& elements) { return param(elements, 0, total(elements)); }
	variant		param() const { return param(chance); }
	static int	total(const variants& elements);
};
variant single(variant v);