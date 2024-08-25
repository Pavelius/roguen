#include "adat.h"
#include "variant.h"

#pragma once

struct variantc : adat<variant, 256> {
	variant		picktop();
	void		select(array& source);
	void		shuffle();
};
