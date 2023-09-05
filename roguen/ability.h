#include "nameable.h"

#pragma once

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits, Charisma,
	WeaponSkill, BalisticSkill, Dodge,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	Armor, Block, BlockRanged,
	Speed, EnemyAttacks,
	Herbalism, OpenLocks, Pickpockets, Survival,
	CarryCapacity, Level,
	Hits, Mana,
	Poison, Illness, Burning, Freezing,
	Experience, Satiation, Money,
};
enum color_s : unsigned char {
	ColorNone,
	ColorRed, ColorBlue, ColorGreen, ColorYellow,
};
struct abilityi : nameable {
	color_s		negative, positive;
};
extern ability_s last_ability;
extern ability_s raw_abilities[4];
struct statable {
	char		abilities[Freezing + 1];
	void		add(ability_s i, int v = 1) { abilities[i] += v; }
};