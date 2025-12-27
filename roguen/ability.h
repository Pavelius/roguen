#include "nameable.h"

#pragma once

enum abilityn : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits,
	WeaponSkill, BalisticSkill, Dodge,
	DamageMelee, DamageRanged, DamageThrown,
	Armor, Block, BlockRanged, EnemyAttacks, ChanceFailSpell,
	Alertness, Gemcutting, Alchemy, Thievery, Literacy, Metallurgy, Mining,
	Stealth, Survival, Haggling, History, Religion, Woodcutting,
	CarryCapacity, Level,
	Hits, Mana, Faith, Mood, Reputation,
	Poison, Illness, Corrosion, Burning, Freezing, Drunk,
	FirstSkill = Alertness, LastSkill = Woodcutting,
};
enum color_s : unsigned char {
	ColorNone,
	ColorRed, ColorBlue, ColorGreen, ColorYellow,
};
struct abilityi : nameable {
	char		minimal;
	char		raise;
	abilityn	base;
	int			skill_divider;
	abilityn	getindex() const;
	bool		isskill() const { return !base; }
};
extern abilityn last_ability;
struct statable {
	char		abilities[Drunk + 1];
	void add(abilityn i, int v, int minimal = -120, int maximal = 120);
	void clear();
};
struct pushability {
	abilityn	value;
	pushability() : value(last_ability) {}
	pushability(abilityn v) : value(last_ability) { last_ability = v; }
	~pushability() { last_ability = value; }
};