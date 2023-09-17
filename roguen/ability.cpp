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
	{"Dodge"},
	{"DamageMelee"},
	{"DamageRanged"},
	{"DamageThrown"},
	{"Armor"},
	{"Block"},
	{"BlockRanged"},
	{"Speed"},
	{"EnemyAttacks"},
	{"Herbalism"},
	{"Thievery"},
	{"Stealth"},
	{"Survival"},
	{"History"},
	{"Religion"},
	{"CarryCapacity"},
	{"Level"},
	{"Hits", ColorRed, ColorGreen},
	{"Mana", ColorBlue},
	{"Poison"},
	{"Illness"},
	{"Burning"},
	{"Freezing"},
	{"Experience", ColorYellow},
	{"Satiation"},
	{"Money"},
};
assert_enum(abilityi, Money)