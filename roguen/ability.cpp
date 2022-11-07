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
	{"Armor"},
	{"Speed"},
	{"EnemyAttacks"},
	{"Pickpockets", Dexterity},
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
	{"Money"},
};
assert_enum(abilityi, Money)

void creature::update_abilities() {
	abilities[ManaMaximum] += get(Wits);
	//abilities[DamageMelee] += get(Strenght) / 10;
	//abilities[DamageThrown] += get(Strenght) / 10;
	abilities[Speed] += get(Dexterity);
	if(is(Light))
		abilities[LineOfSight] += 3;
}
