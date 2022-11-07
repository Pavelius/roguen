#include "nameable.h"

#pragma once

enum class res;

struct visualeffect : nameable {
	res				resid;
	short unsigned	frame, feats;
	unsigned char	priority = 15;
	int				dy;
	void			paint(unsigned char random) const;
};
