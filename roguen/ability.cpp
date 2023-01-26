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
	{"WeaponSkill", Strenght},
	{"BalisticSkill", Dexterity},
	{"Dodge"},
	{"Damage"},
	{"DamageMelee"},
	{"DamageRanged"},
	{"DamageThrown"},
	{"Armor"},
	{"Speed"},
	{"EnemyAttacks"},
	{"Herbalism", Wits, FG(HardSkill)},
	{"Pickpockets", Dexterity, FG(HardSkill)},
	{"Stealth", Dexterity},
	{"OpenLocks", Dexterity},
	{"DisarmTraps", Dexterity},
	{"Survival", Wits},
	{"Level"},
	{"HitsMaximum"},
	{"ManaMaximum"},
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