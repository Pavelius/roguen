#include "main.h"

static int ability_bonus[] = {
	-5, -4, -4, -3, -2, -2, -1, -1, -1, 0,
	0, 0, 0, 1, 1, 1, 2, 2, 3, 3,
	4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10
};
static int attack_bonus[3][20] = {
	{-1, 0, 0, 0, 2, 2, 2, 5, 5, 5, 7, 7, 7, 9, 9, 9, 11, 11, 11, 13},
	{-1, 0, 0, 0, 0, 2, 2, 2, 2, 5, 5, 5, 5, 7, 7, 7, 7, 9, 9, 9},
	{-1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 7, 7, 7, 7},
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