#include "nameable.h"

#pragma once

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits,
	WeaponSkill, BalisticSkill, Dodge,
	DamageMelee, DamageRanged, DamageThrown,
	Armor, Block, BlockRanged,
	Speed, EnemyAttacks,
	Alertness, Gemcutting, Herbalism, Thievery, Literacy, Mining,
	Stealth, Survival, Haggling, History, Religion, Woodcutting,
	CarryCapacity, Level,
	Hits, Mana, Faith,
	Poison, Illness, Corrosion, Burning, Freezing,
	FirstSkill = Alertness, LastSkill = Woodcutting,
};
enum color_s : unsigned char {
	ColorNone,
	ColorRed, ColorBlue, ColorGreen, ColorYellow,
};
struct abilityi : nameable {
	ability_s	base;
	ability_s	getindex() const;
	bool		isskill() const { return !base; }
};
extern ability_s last_ability;
extern ability_s raw_abilities[3];
struct statable {
	char		abilities[Freezing + 1];
	void		add(ability_s i, int v = 1) { abilities[i] += v; }
};

color_s get_negative_color(ability_s v);
color_s get_positive_color(ability_s v);