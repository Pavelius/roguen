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
	Floor, Decals, Features, Items,
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
	HitsMaximum, ManaMaximum,
	Hits, Mana,
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
enum condition_s : unsigned char {
	Busy,
};
enum feat_s : unsigned char {
	EnergyDrain, Paralysis, PetrifyingGaze, PoisonImmunity, StrenghtDrain,
	SunSensitive, Slow, NormalWeaponImmunity,
	Blunt, Martial, TwoHanded,
	WearLeather, WearIron, WearLarge, WearShield, Countable,
	Undead, Summoned, Player, Enemy,
	Stun, Unaware,
};
enum spell_s : unsigned char {
	Sleep,
};
struct featable : flagable<4> {};
struct spellf : flagable<8> {};
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
	featable	feats;
	void		update();
};
struct dice {
	char		min, max;
	int			roll() const;
};
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
	const char*	avatar;
	void		paint() const;
};
class item {
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
public:
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
struct itemground : item {
	indext		index;
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
struct spellable {
	char		spells[Sleep + 1];
};
class creature : public wearable, public statable, public spellable {
	statable	basic;
	spellf		active_spells;
	unsigned	experience;
	int			wait_seconds;
	void		advance(variant kind, int level);
	void		advance(variants elements);
	void		advance(variant element);
	void		update();
public:
	typedef void (creature::*fnupdate)();
	operator bool() const { return abilities[Hits] > 0; }
	static creature* create(indext index, const monsteri* p);
	static creature* create(indext index, const racei* p, gender_s gender);
	void		aimove();
	void		checkmood() {}
	void		checkpoison() {}
	void		checksick() {}
	void		finish();
	int			get(ability_s v) const { return abilities[v]; }
	int			get(spell_s v) const { return spells[v]; }
	int			getwait() const { return wait_seconds; }
	bool		is(condition_s v) const { return false; }
	bool		is(spell_s v) const { return active_spells.is(v); }
	bool		is(feat_s v) const { return feats.is(v); }
	bool		isalive() const { return abilities[Hits] > 0; }
	bool		isactive() const;
	void		makemove();
	void		movestep(direction_s i);
	void		movestep(indext i);
	void		paint() const;
	void		restoration() {}
	void		remove(feat_s v) { feats.remove(v); }
	bool		roll(ability_s v) const;
	void		wait() { wait_seconds += 100; }
};
struct advancement {
	variant		type;
	char		level;
	variants	elements;
};
class gamei : public areamap {
	unsigned	minutes;
	unsigned	restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day;
public:
	static void	all(creature::fnupdate proc);
	void		pass(unsigned minutes);
	void		passminute();
	static void	play();
	void		playminute();
};
namespace draw {
bool			isnext();
}
inline int		d100() { return rand() % 100; }

extern gamei	game;
extern creature* player;