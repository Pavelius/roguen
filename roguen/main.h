#include "areamap.h"
#include "flagable.h"
#include "gender.h"
#include "crt.h"
#include "list.h"
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
	Head, Torso, Backward, Legs, Gloves, FingerRight, FingerLeft, Elbows,
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
class actable {
	variant		kind; // Race or monster
	gender_s	gender;
public:
	void		actv(stringbuilder& sb, const char* format, const char* format_param);
	gender_s	getgender() const { return gender; }
	variant		getkind() const { return kind; }
	const char*	getname() const { return kind.getname(); }
	void		setgender(gender_s v) { gender = v; }
	void		setkind(variant v) { kind = v; }
};
class movable : public actable {
	indext		index;
	direction_s	direction;
	bool		mirror;
public:
	void		fixaction() const;
	void		fixappear() const;
	void		fixmovement() const;
	bool		ismirror() const { return mirror; }
	indext		getindex() const { return index; }
	point		getposition() const { return m2s(i2m(index)); }
	void		setdirection(direction_s v);
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
	int			getavatar() const { return geti().wear_index; }
	const itemi& geti() const { return bsdata<itemi>::elements[type]; }
	int			getcost() const;
	int			getcount() const;
	dice		getdamage() const;
	const char*	getname() const { return geti().getname(); }
	void		getstatus(stringbuilder& sb) const;
	int			getweight() const;
	bool		is(feat_s v) const { return geti().flags.is(v); }
	bool		iscountable() const { return is(Countable); }
	void		setcount(int v);
};
struct wearable : movable {
	item		wears[Elbows + 1];
	void		additem(item& v);
	void		equip(item& v);
	const char*	getwearname(wear_s id) const;
	bool		isitem(const void* pv) const;
};
struct monsteri : nameable, statable {
	const char*	avatar;
	gender_s	gender;
};
class creature : public wearable, public statable {
	statable	basic;
	short		hp, mp;
	unsigned	experience;
	void		advance(variant kind, int level);
	void		advance(variants elements);
	void		advance(variant element);
public:
	operator bool() const { return hp > 0; }
	static creature* create(indext index, const monsteri* p);
	static creature* create(indext index, const racei* p, gender_s gender);
	void		aimove();
	void		finish();
	void		movestep(direction_s i);
	void		movestep(indext i);
	void		paint() const;
};
struct advancement {
	variant		type;
	char		level;
	variants	elements;
};
struct gamei {
	unsigned	minutes;
	void		pass(unsigned minutes);
	void		playminute();
};

extern gamei game;
extern creature* player;

void adventure_mode();