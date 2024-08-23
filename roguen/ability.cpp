#include "ability.h"
#include "crt.h"

ability_s last_ability;

BSDATA(abilityi) = {
	{"LineOfSight"},
	{"Strenght"},
	{"Dexterity"},
	{"Wits"},
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
	{"Alertness", Wits},
	{"Gemcutting", Dexterity},
	{"Herbalism", Wits},
	{"Thievery", Dexterity},
	{"Literacy", Wits},
	{"Mining", Dexterity},
	{"Stealth", Dexterity},
	{"Survival", Wits},
	{"Haggling", Wits},
	{"History", Wits},
	{"Religion", Wits},
	{"Woodcutting", Dexterity},
	{"CarryCapacity"},
	{"Level"},
	{"Hits"},
	{"Mana"},
	{"Faith"},
	{"Mood"},
	{"SkillPoints"},
	{"Poison"},
	{"Illness"},
	{"Corrosion"},
	{"Burning"},
	{"Freezing"},
};
assert_enum(abilityi, Freezing)

ability_s abilityi::getindex() const {
	return (ability_s)(this - bsdata<abilityi>::elements);
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