#include "nameable.h"

#pragma once

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits, Charisma,
	WeaponSkill, BalisticSkill, DodgeSkill, ShieldUse,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	Armor,
	Speed, EnemyAttacks,
	Herbalism, Pickpockets, Stealth, OpenLocks, DisarmTraps,
	Survival,
	Level,
	HitsMaximum, ManaMaximum,
	Hits, Mana, Poison, Illness, Reputation, ParryCount, Experience, Satiation, Money,
};
enum abilityf : unsigned char {
	HardSkill,
};
struct abilityfi : nameable {
};
struct abilityi : nameable {
	ability_s	basic;
	unsigned	flags;
	bool		is(abilityf v) const { return (flags & (1 << v)) != 0; }
};
extern ability_s last_ability;