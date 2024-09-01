#include "ability.h"
#include "bsdata.h"

ability_s last_ability;

BSDATA(abilityi) = {
	{"LineOfSight", 0},
	{"Strenght", 0},
	{"Dexterity", 0},
	{"Wits", 0}, // 1 - Insects, 2 - Snakes, 3 - HerdAnimals, 4 - PredatorAnimals,
	{"WeaponSkill", 0},
	{"BalisticSkill", 0},
	{"Dodge", 0, Dexterity},
	{"DamageMelee", -120},
	{"DamageRanged", -120},
	{"DamageThrown", -120},
	{"Armor", -120},
	{"Block", 0},
	{"BlockRanged", 0},
	{"EnemyAttacks", 0},
	{"ChanceFailSpell", 0},
	{"Alertness", 0, Wits, 3},
	{"Gemcutting", 0, Dexterity},
	{"Alchemy", 0, Wits},
	{"Thievery", 0, Dexterity},
	{"Literacy", 0, Wits, 3},
	{"Mining", 0, Strenght, 3},
	{"Stealth", 0, Dexterity, 3},
	{"Survival", 0, Wits, 3},
	{"Haggling", 0, Wits, 3},
	{"History", 0, Wits},
	{"Religion", 0, Wits},
	{"Woodcutting", 0, Dexterity, 3},
	{"CarryCapacity", 0},
	{"Level", 0},
	{"Hits", 0},
	{"Mana", 0},
	{"Faith", 0},
	{"Mood", -120},
	{"Reputation", -120},
	{"SkillPoints", 0},
	{"Poison", -120},
	{"Illness", -120},
	{"Corrosion", -120},
	{"Burning", -120},
	{"Freezing", -120},
	{"Drunk", -120},
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