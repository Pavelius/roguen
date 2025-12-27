#include "ability.h"
#include "bsdata.h"

abilityn last_ability;

BSDATA(abilityi) = {
	{"LineOfSight", 0},
	{"Strenght", 0, 3},
	{"Dexterity", 0, 3},
	{"Wits", 0, 3}, // 1 - Insects, 2 - Snakes, 3 - HerdAnimals, 4 - PredatorAnimals,
	{"WeaponSkill", 0, 4},
	{"BalisticSkill", 0, 4},
	{"Dodge", 0, 4, Dexterity},
	{"DamageMelee", -120},
	{"DamageRanged", -120},
	{"DamageThrown", -120},
	{"Armor", -120},
	{"Block", 0},
	{"BlockRanged", 0},
	{"EnemyAttacks", 0},
	{"ChanceFailSpell", 0},
	{"Alertness", 0, 5, Wits, 3},
	{"Gemcutting", 0, 5, Dexterity},
	{"Alchemy", 0, 5, Wits},
	{"Thievery", 0, 5, Dexterity},
	{"Literacy", 0, 5, Wits, 3},
	{"Metallurgy", 0, 5, Wits, 3},
	{"Mining", 0, 5, Strenght, 3},
	{"Stealth", 0, 5, Dexterity, 3},
	{"Survival", 0, 5, Wits, 3},
	{"Haggling", 0, 5, Wits, 3},
	{"History", 0, 5, Wits},
	{"Religion", 0, 5, Wits},
	{"Woodcutting", 0, 5, Dexterity, 3},
	{"CarryCapacity", 0},
	{"Level", 0},
	{"Hits", 0},
	{"Mana", 0},
	{"Faith", 0},
	{"Mood", -120},
	{"Reputation", -120},
	{"Poison", -120},
	{"Illness", -120},
	{"Corrosion", -120},
	{"Burning", -120},
	{"Freezing", -120},
	{"Drunk", -120},
};
assert_enum(abilityi, Drunk)

abilityn abilityi::getindex() const {
	return (abilityn)(this - bsdata<abilityi>::elements);
}

void statable::add(abilityn v, int bonus, int minimum, int maximum) {
	bonus += abilities[v];
	if(bonus < minimum)
		bonus = minimum;
	else if(bonus > maximum)
		bonus = maximum;
	abilities[v] = bonus;
}

void statable::clear() {
	memset((void*)this, 0, sizeof(*this));
}

//color_s negative_color(abilityn v) {
//	switch(v) {
//	case Hits: return ColorRed;
//	default: return ColorNone;
//	}
//}
//
//color_s positive_color(abilityn v) {
//	switch(v) {
//	case Hits: return ColorGreen;
//	case Mana: return ColorBlue;
//	case Faith: return ColorYellow;
//	default: return ColorNone;
//	}
//}