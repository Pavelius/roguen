#include "main.h"

const char* getnameshort(ability_s id) {
	return getnm(bsdata<abilityi>::elements[id].id);
}

static void addf(stringbuilder& sb, ability_s i, int value) {
	switch(i) {
	case Money:
		sb.addn("$%1i", value);
		break;
	default:
		sb.addn("[~%1]\t%2i", getnameshort(i), value);
		break;
	}
	
}

void creature::getinfo(stringbuilder& sb) const {
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		addf(sb, i, abilities[i]);
	addf(sb, Money, getmoney());
}