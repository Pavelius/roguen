#include "ability.h"

#pragma once

struct skillable : statable {
	unsigned	skill_recall[Religion - Herbalism + 1];
	bool		canuse(ability_s v) const;
};
