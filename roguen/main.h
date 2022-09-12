#include "areamap.h"
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
struct statable {
	char		abilities[Mana+1];
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
	char		bonus; // ToHitMelee, ToHitRange, ToHitThrown, ParryValue
	weaponi		weapon;
};
struct item {
	unsigned char type, subtype;
	union {
		unsigned short count;
		struct {
			unsigned char identified : 1;
			unsigned char broken : 1;
			unsigned char charge : 5;
			unsigned char count_nocountable;
		};
	};
	explicit operator bool() const { return type != 0; }
	void			add(item& v);
	void			addname(stringbuilder& sb) const;
	bool			canequip(wear_s v) const;
	void			clear() { memset(this, 0, sizeof(*this)); }
	void			create(const char* id, int count = 1);
	void			create(const itemi* pi, int count = 1);
	const itemi&	geti() const { return bsdata<itemi>::elements[type]; }
	int				getcost() const;
	int				getcount() const;
	dice			getdamage() const;
	const char*		getname() const { return geti().getname(); }
	void			getstatus(stringbuilder& sb) const;
	int				getweight() const;
	void			setcount(int v);
};
struct monsteri : nameable, statable {
	const char*	avatar;
	gender_s	gender;
};
struct creature : movable, statable {
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
extern creature* player;
void adventure_mode();
