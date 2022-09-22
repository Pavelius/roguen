#include "main.h"

const char* getnameshort(ability_s id) {
	return getnm(bsdata<abilityi>::elements[id].id);
}

static void addv(stringbuilder& sb, const dice& value) {
	if(value.max!=value.min)
		sb.adds("%1i-%2i", value.min, value.max);
	else
		sb.adds("%1i", value.min);
}

static void addf(stringbuilder& sb, ability_s i, int value, int value_maximum = 0) {
	switch(i) {
	case Hits: case Mana:
		sb.addn("[~%1]\t%2i/%3i", getnameshort(i), value, value_maximum);
		break;
	default:
		sb.addn("[~%1]\t%2i", getnameshort(i), value);
		break;
	}
	
}

void creature::getinfo(stringbuilder& sb) const {
	sb.addn("Кастор");
	sb.addn("Эльф мужчина");
	sb.addn("---");
	sb.addn("$Tab -20");
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		addf(sb, i, abilities[i]);
	sb.addn("---");
	sb.addn("$Tab -40");
	addf(sb, Hits, abilities[Hits], abilities[HitsMaximum]);
	addf(sb, Mana, abilities[Mana], abilities[ManaMaximum]);
	addf(sb, Money, getmoney());
	sb.addn("---");
	sb.addn("[~%1]\t%2i", getnm("Rounds"), game.getminutes());
}

void item::getinfo(stringbuilder& sb, bool need_name) const {
	auto& ei = geti();
	if(need_name)
		sb.adds(getname());
	if(ei.bonus)
		sb.adds("%+1i", ei.bonus);
	if(ei.weapon.damage) {
		sb.adds("%-Damage");
		addv(sb, ei.weapon.damage);
	}
}