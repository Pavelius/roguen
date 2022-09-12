#include "areamap.h"
#include "flagable.h"
#include "gender.h"
#include "crt.h"
#include "script.h"
#include "variant.h"

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
enum wear_s : unsigned char {
	Backpack, Potion, BackpackLast = Backpack + 15,
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, ThrownWeapon, Ammunition,
	Head, Torso, Legs, Gloves, FingerRight, FingerLeft, Elbows,
};
enum magic_s : unsigned char {
	Mudane, Blessed, Cursed, Artifact,
};
enum feat_s : unsigned char {
	EnergyDrain, Paralysis, PetrifyingGaze, PoisonImmunity, StrenghtDrain,
	SunSensitive, Slow, NormalWeaponImmunity,
	Blunt, Martial, TwoHanded,
	WearLeather, WearIron, WearLarge, WearShield, Countable,
	Undead, Summoned, Player, Enemy,
};
struct featable : flagable<4> {};
extern point m2s(point v);
struct nameable {
	const char* id;
	const char*	getname() const { return getnm(id); }
};
struct abilityi : nameable {
};
struct racei : nameable {
};
struct classi : nameable {
};
struct feati : nameable {
};
struct weari : nameable {
};
struct statable {
	char		abilities[Mana + 1];
};
struct dice {
	char		min, max;
	int			roll() const;
};
struct location : areamap {
	indext		overland;
	short		level;
};
extern location area;
struct hotkey : nameable {
	unsigned	key;
	fnevent		proc;
};
struct actable {
	variant		kind; // Race or monster
	void		actv(stringbuilder& sb, const char* format, const char* format_param);
	gender_s	getgender() const;
	const char*	getname() const { return kind.getname(); }
};
struct movable : actable {
	indext		index;
	direction_s	direction;
	bool		mirror;
	void		fixaction() const;
	void		fixappear() const;
	void		fixmovement() const;
	point		getposition() const { return m2s(i2m(index)); }
	void		setindex(indext i);
};
struct itemi : nameable {
	struct weaponi {
		dice	damage;
		short 	ammunition;
	};
	int			cost, weight, count;
	wear_s		wear;
	ability_s	ability;
	char		bonus;
	weaponi		weapon;
	featable	flags;
	char		wear_index;
};
struct item {
	unsigned short type;
	union {
		unsigned short count;
		struct {
			magic_s	magic : 2;
			unsigned char broken : 2;
			unsigned char identified : 1;
			unsigned char charges : 3;
			unsigned char subtype;
		};
	};
	explicit operator bool() const { return type != 0; }
	void		add(item& v);
	void		addname(stringbuilder& sb) const;
	bool		canequip(wear_s v) const;
	void		clear() { type = count = 0; }
	void		create(const char* id, int count = 1);
	void		create(const itemi* pi, int count = 1);
	const itemi& geti() const { return bsdata<itemi>::elements[type]; }
	int			getcost() const;
	int			getcount() const;
	dice		getdamage() const;
	const char*	getname() const { return geti().getname(); }
	void		getstatus(stringbuilder& sb) const;
	int			getweight() const;
	void		setcount(int v);
};
struct wearable : movable {
	item		wears[Elbows + 1];
};
struct monsteri : nameable, statable {
	const char*	avatar;
	gender_s	gender;
};
struct creature : wearable, statable {
	gender_s	gender;
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
struct gamei {
	unsigned	minutes;
	void		pass(unsigned minutes);
	void		playminute();
};
extern gamei game;
extern creature* player;
void adventure_mode();
