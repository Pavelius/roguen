#include "main.h"

void statable::create() {
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		abilities[i] = 8 + rand() % 5;
	abilities[LineOfSight] += 4;
}