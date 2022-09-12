#include "areamap.h"
#include "crt.h"
#include "nameable.h"
#include "script.h"

#pragma once

enum class res {
	Monsters,
	Floor, Decals, Features,
	PCBody, PCArms, PCAccessories,
};
enum ability_s : unsigned char {
	Strenght, Dexterity, Constitution, Intellect, Wisdow, Charisma,
	ToHit, ToHitMelee, ToHitRanged, ToHitThrown,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	ParryValue, DamageReduciton,
	Speed,
	FightLight, FightHeavy, Markship,
	Concentration, Healing,
	HitsMaximum, Hits,
	ManaMaximum, Mana,
};
enum tag_s : unsigned char {
	FireResistance,
};
extern point m2s(point v);
struct idable {
	const char* id;
};
struct abilityi : idable {
};
struct racei : idable {
};
struct classi {
	const char*	id;
};
struct statable {
	char		abilities[Mana+1];
};
struct location : areamap {
	indext		overland;
	short		level;
};
extern location area;
struct hotkey {
	const char*	id;
	unsigned	key;
	fnevent		proc;
};
struct posable : nameable {
	indext		index;
	void		fixappear() const;
	void		fixmovement() const;
	point		getposition() const { return m2s(i2m(index)); }
	void		setindex(indext i);
};
struct monsteri : idable, statable {
	const char*	avatar;
	gender_s	gender;
};
struct creature : posable, statable {
	direction_s	direction;
	statable	basic;
	short		hp, mp;
	unsigned	experience;
	operator bool() const { return hp > 0; }
	static creature* create(indext index, const monsteri* p);
	void		aimove();
	void		finish();
	void		movestep(direction_s i);
	void		movestep(indext i);
	void		paint() const;
};
extern creature* player;
void adventure_mode();
