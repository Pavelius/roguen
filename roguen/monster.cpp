#include "monster.h"

monsteri* monsteri::ally() const {
	if(minions)
		return minions->param();
	return 0;
}