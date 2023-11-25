#include "ability.h"
#include "crt.h"

ability_s last_ability;
ability_s raw_abilities[3];

BSDATA(abilityi) = {
	{"LineOfSight"},
	{"Strenght"},
	{"Dexterity"},
	{"Wits"},
	{"WeaponSkill"},
	{"BalisticSkill"},
	{"Dodge", ColorNone, ColorNone, Dexterity},
	{"DamageMelee"},
	{"DamageRanged"},
	{"DamageThrown"},
	{"Armor"},
	{"Block"},
	{"BlockRanged"},
	{"Speed"},
	{"EnemyAttacks"},
	{"Alertness", ColorNone, ColorNone, Wits},
	{"Gemcutting", ColorNone, ColorNone, Dexterity},
	{"Herbalism", ColorNone, ColorNone, Wits},
	{"Thievery", ColorNone, ColorNone, Dexterity},
	{"Literacy", ColorNone, ColorNone, Wits},
	{"Mining", ColorNone, ColorNone, Dexterity},
	{"Stealth", ColorNone, ColorNone, Dexterity},
	{"Survival", ColorNone, ColorNone, Wits},
	{"History", ColorNone, ColorNone, Wits},
	{"Religion", ColorNone, ColorNone, Wits},
	{"Woodcutting", ColorNone, ColorNone, Dexterity},
	{"CarryCapacity"},
	{"Level"},
	{"Hits", ColorRed, ColorGreen},
	{"Mana", ColorBlue},
	{"Faith", ColorYellow},
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