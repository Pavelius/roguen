#include "game.h"
#include "skillable.h"

bool skillable::canuse(ability_s v) const {
	if(!abilities[v])
		return false;
	if(v >= FirstSkill && v <= LastSkill)
		return skill_recall[v - FirstSkill] < game.getminutes();
	return true;
}