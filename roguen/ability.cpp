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
	abilities[Speed] += 25 + get(Dexterity) / 5;
	if(is(Stun)) {
		abilities[WeaponSkill] -= 10;
		abilities[BalisticSkill] -= 10;
		abilities[DodgeSkill] -= 20;
		abilities[ShieldUse] -= 10;
	}
	if(!is(IgnoreWeb) && ispresent() && area.is(getposition(), Webbed)) {
		abilities[WeaponSkill] -= 10;
		abilities[DodgeSkill] -= 20;
		abilities[ShieldUse] -= 10;
	}
	if(is(LightSource))
		abilities[LineOfSight] += 3;
}
