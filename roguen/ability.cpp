#include "ability.h"
#include "crt.h"

ability_s last_ability;

BSDATA(abilityfi) = {
	{"HardSkill"},
};
assert_enum(abilityfi, HardSkill)

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
	{"Herbalism", FG(HardSkill)},
	{"Pickpockets", FG(HardSkill)},
	{"Stealth"},
	{"OpenLocks"},
	{"DisarmTraps"},
	{"Survival"},
	{"Level"},
	{"Hits", 0, ColorRed, ColorGreen},
	{"Mana", 0, ColorBlue},
	{"Poison"},
	{"Illness"},
	{"Alignment"},
	{"ParryCount"},
	{"Experience", 0, ColorYellow},
	{"Satiation"},
	{"Money"},
};
assert_enum(abilityi, Money)