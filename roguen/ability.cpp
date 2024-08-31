#include "ability.h"
#include "bsdata.h"

ability_s last_ability;

BSDATA(abilityi) = {
	{"LineOfSight"},
	{"Strenght"},
	{"Dexterity"},
	{"Wits"}, // 1 - Insects, 2 - Snakes, 3 - HerdAnimals, 4 - PredatorAnimals,
	{"WeaponSkill"},
	{"BalisticSkill"},
	{"Dodge", Dexterity},
	{"DamageMelee"},
	{"DamageRanged"},
	{"DamageThrown"},
	{"Armor"},
	{"Block"},
	{"BlockRanged"},
	{"EnemyAttacks"},
	{"ChanceFailSpell"},
	{"Alertness", Wits, 3},
	{"Gemcutting", Dexterity},
	{"Alchemy", Wits},
	{"Thievery", Dexterity},
	{"Literacy", Wits, 3},
	{"Mining", Strenght, 3},
	{"Stealth", Dexterity, 3},
	{"Survival", Wits, 3},
	{"Haggling", Wits, 3},
	{"History", Wits},
	{"Religion", Wits},
	{"Woodcutting", Dexterity, 3},
	{"CarryCapacity"},
	{"Level"},
	{"Hits"},
	{"Mana"},
	{"Faith"},
	{"Mood"},
	{"Reputation"},
	{"SkillPoints"},
	{"Poison"},
	{"Illness"},
	{"Corrosion"},
	{"Burning"},
	{"Freezing"},
	{"Drunk"},
};
assert_enum(abilityi, Drunk)

ability_s abilityi::getindex() const {
	return (ability_s)(this - bsdata<abilityi>::elements);
}

void statable::add(ability_s v, int bonus, int minimum, int maximum) {
	bonus += abilities[v];
	if(bonus < minimum)
		bonus = minimum;
	else if(bonus > maximum)
		bonus = maximum;
	abilities[v] = bonus;
}

//color_s negative_color(ability_s v) {
//	switch(v) {
//	case Hits: return ColorRed;
//	default: return ColorNone;
//	}
//}
//
//color_s positive_color(ability_s v) {
//	switch(v) {
//	case Hits: return ColorGreen;
//	case Mana: return ColorBlue;
//	case Faith: return ColorYellow;
//	default: return ColorNone;
//	}
//}