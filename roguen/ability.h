#include "nameable.h"

#pragma once

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits,
	WeaponSkill, BalisticSkill, Dodge,
	DamageMelee, DamageRanged, DamageThrown,
	Armor, Block, BlockRanged, EnemyAttacks, ChanceFailSpell,
	Alertness, Gemcutting, Alchemy, Thievery, Literacy, Mining,
	Stealth, Survival, Haggling, History, Religion, Woodcutting,
	CarryCapacity, Level,
	Hits, Mana, Faith, Mood, Reputation, SkillPoints,
	Poison, Illness, Corrosion, Burning, Freezing, Drunk,
	FirstSkill = Alertness, LastSkill = Woodcutting,
};
enum color_s : unsigned char {
	ColorNone,
	ColorRed, ColorBlue, ColorGreen, ColorYellow,
};
struct abilityi : nameable {
	ability_s	base;
	int			skill_divider;
	ability_s	getindex() const;
	bool		isskill() const { return !base; }
};
extern ability_s last_ability;
struct statable {
	char		abilities[Drunk + 1];
	void		add(ability_s i, int v = 1) { abilities[i] += v; }
};