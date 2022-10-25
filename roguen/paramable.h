#include "nameable.h"
#include "variant.h"

#pragma once

struct paramable {
	variant			v1, v2;
	const char*		getname() const;
};