#include "monster.h"

monsteri* monsteri::ally() const {
	if(minions)
		return minions->random();
	return 0;
}