#include "nameable.h"

#pragma once

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits, Charisma,
	WeaponSkill, BalisticSkill, DodgeSkill, ShieldUse,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	Armor,
	Speed, EnemyAttacks,
	Pickpockets, Stealth, OpenLocks, DisarmTraps,
	Survival,
	Level,
	HitsMaximum, ManaMaximum,
	Hits, Mana, Poison, Illness, Reputation, ParryCount, Experience, Satiation, Money,
};
struct abilityi : nameable {
	ability_s	basic;
};
extern ability_s last_ability;