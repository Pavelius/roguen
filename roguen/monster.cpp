#include "monster.h"

monsteri* monsteri::ally() const {
	if(minions)
		return minions->random();
	return 0;
}

bool monsteri::islower(ability_s v, int value) const {
	for(auto v : use) {
		if(v.iskind<abilityi>() && v.value <= value)
			return true;
	}
	return false;
}

bool is_boss(const void* p) {
	return ((monsteri*)p)->minions != 0;
}