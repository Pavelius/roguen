#include "nameable.h"
#include "variant.h"

#pragma once

struct randomizeri : nameable {
	variants	chance;
	static variant random(const variants& elements);
	variant		random() const { return random(chance); }
	static int	total(const variants& elements);
};
variant single(variant v);