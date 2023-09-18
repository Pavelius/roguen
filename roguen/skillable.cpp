#include "game.h"
#include "skillable.h"

bool skillable::canuse(ability_s v) const {
	if(!abilities[v])
		return false;
	if(v >= Herbalism && v <= FirstAid)
		return skill_recall[v - Herbalism] < game.getminutes();
	return false;
}