#include "ability.h"
#include "bsdata.h"

abilityn last_ability;

BSDATA(abilityi) = {
	{"LineOfSight", 0},
	{"Strenght", 0, 30, 4},
	{"Dexterity", 0, 30, 3},
	{"Wits", 0, 30, 3}, // 1 - Insects, 2 - Snakes, 3 - HerdAnimals, 4 - PredatorAnimals,
	{"WeaponSkill", 0, 30, 1},
	{"BalisticSkill", 0, 30, 1},
	{"Dodge", 0, 30, 2, Dexterity},
	{"DamageMelee", -120},
	{"DamageRanged", -120},
	{"DamageThrown", -120},
	{"Armor", -120},
	{"Block", 0},
	{"BlockRanged", 0},
	{"EnemyAttacks", 0},
	{"ChanceFailSpell", 0},
	{"Alertness", 0, 30, 1, Wits, 3},
	{"Gemcutting", 0, 30, 1, Dexterity},
	{"Alchemy", 0, 30, 1, Wits},
	{"Thievery", 0, 30, 1, Dexterity},
	{"Literacy", 0, 30, 1, Wits, 3},
	{"Metallurgy", 0, 30, 1, Wits, 3},
	{"Mining", 0, 30, 1, Strenght, 3},
	{"Stealth", 0, 30, 1, Dexterity, 3},
	{"Survival", 0, 30, 1, Wits, 3},
	{"Haggling", 0, 30, 1, Wits, 3},
	{"History", 0, 30, 1, Wits},
	{"Religion", 0, 30, 1, Wits},
	{"Woodcutting", 0, 30, 1, Dexterity, 3},
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