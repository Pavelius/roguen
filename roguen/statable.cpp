#include "main.h"

static int ability_bonus[] = {
	-4, -4, -4, -3, -2, -2, -1, -1, -1, 0,
	0, 0, 0, 1, 1, 1, 2, 2, 3, 3,
	4, 4, 5, 5, 6, 6
};
static int ability_bonush[] = {
	-3, -3, -3, -2, -1, -1, -1, -1, -1, 0,
	0, 0, 0, 1, 1, 1, 1, 1, 2
};

void statable::create() {
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		abilities[i] = 8 + rand() % 5;
	abilities[LineOfSight] += 3;
}

int	statable::getbonus(ability_s v) const {
	auto i = abilities[v];
	return maptbl(ability_bonus, i);
}