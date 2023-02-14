#include "ability.h"
#include "crt.h"

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
	{"Hits"},
	{"Mana"},
	{"Poison"},
	{"Illness"},
	{"Alignment"},
	{"ParryCount"},
	{"Experience"},
	{"Satiation"},
	{"Money"},
};
assert_enum(abilityi, Money)