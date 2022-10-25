#include "nameable.h"
#include "variant.h"

#pragma once

struct randomizeri : nameable {
	variants	chance;
	variant		random() const;
};
variant single(variant v);