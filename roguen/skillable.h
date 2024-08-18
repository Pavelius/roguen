#include "ability.h"

#pragma once

struct skillable : statable {
	unsigned	skill_recall[LastSkill - FirstSkill + 1];
	bool		canuse(ability_s v) const;
	void		setrecall(ability_s v, unsigned rounds);
};
