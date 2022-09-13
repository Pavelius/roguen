#include "main.h"

creaturea	creatures, enemies;
creature*	last_enemy;

static void choose_creature(int bonus) {
}

BSDATA(script) = {
	{"ChooseCreature", choose_creature},
};
BSDATAF(script)