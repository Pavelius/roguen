#include "nameable.h"

#pragma once

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits,
	WeaponSkill, BalisticSkill, Dodge,
	DamageMelee, DamageRanged, DamageThrown,
	Armor, Block, BlockRanged,
	Speed, EnemyAttacks,
	Herbalism, Thievery, Stealth, Survival, History, Religion, FirstAid,
	CarryCapacity, Level,
	Hits, Mana, Faith,
	Poison, Illness, Burning, Freezing,
	Experience, Satiation, Money,
};
enum color_s : unsigned char {
	ColorNone,
	ColorRed, ColorBlue, ColorGreen, ColorYellow,
};
struct abilityi : nameable {
	color_s		negative, positive;
	ability_s	base;
	ability_s	getindex() const;
	bool		isskill() const { return base != 0; }
};
extern ability_s last_ability;
extern ability_s raw_abilities[3];
struct statable {
	char		abilities[Freezing + 1];
	void		add(ability_s i, int v = 1) { abilities[i] += v; }
};