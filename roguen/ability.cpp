#include "main.h"

BSDATA(abilityi) = {
	{"LineOfSight"},
	{"Strenght"},
	{"Dexterity"},
	{"Wits"},
	{"Charisma"},
	{"WeaponSkill", Strenght},
	{"BalisticSkill", Dexterity},
	{"DodgeSkill", Dexterity},
	{"ShieldUse", Dexterity},
	{"Damage"},
	{"DamageMelee"},
	{"DamageRanged"},
	{"DamageThrown"},
	{"DamageReduciton"},
	{"Speed"},
	{"Pickpockets"},
	{"Stealth"},
	{"OpenLocks"},
	{"DisarmTraps"},
	{"Survival"},
	{"Level"},
	{"HitsMaximum"},
	{"ManaMaximum"},
	{"Hits"},
	{"Mana"},
	{"Mood"},
	{"Alignment"},
	{"ParryCount"},
	{"Money"},
};
assert_enum(abilityi, Money)
