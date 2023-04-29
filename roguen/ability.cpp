#include "ability.h"
#include "crt.h"

ability_s last_ability;
ability_s raw_abilities[4];

BSDATA(abilityi) = {
	{"LineOfSight"},
	{"Strenght"},
	{"Dexterity"},
	{"Wits"},
	{"Charisma"},
	{"WeaponSkill"},
	{"BalisticSkill"},
	{"Dodge"},
	{"Damage"},
	{"DamageMelee"},
	{"DamageRanged"},
	{"DamageThrown"},
	{"Armor"},
	{"Block"},
	{"BlockRanged"},
	{"Speed"},
	{"EnemyAttacks"},
	{"Herbalism"},
	{"Pickpockets"},
	{"Stealth"},
	{"OpenLocks"},
	{"DisarmTraps"},
	{"Survival"},
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