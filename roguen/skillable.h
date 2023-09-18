#include "ability.h"

#pragma once

struct skillable : statable {
	unsigned	skill_recall[FirstAid - Herbalism + 1];
	bool		canuse(ability_s v) const;
};
