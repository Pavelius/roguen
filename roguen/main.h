#include "answers.h"
#include "areamap.h"
#include "dice.h"
#include "direction.h"
#include "flagable.h"
#include "generator.h"
#include "hotkey.h"
#include "color.h"
#include "crt.h"
#include "list.h"
#include "pushvalue.h"
#include "script.h"
#include "shape.h"
#include "speech.h"
#include "variant.h"
#include "world.h"

#pragma once

const int version_major = 0;
const int version_minor = 0;
const int version_build = 1;

enum class res {
	Monsters,
	Borders, Floor, Walls, Decals, Features, Shadows, Items,
	Attack, Conditions, Splash,
	Fow,
	PCBody, PCArms, PCAccessories,
};
enum ability_s : unsigned char {
	LineOfSight,
	Brawl, Dexterity, Wits, Charisma,
	WeaponSkill, HeavyWeaponSkill, PolearmSkill, RangedWeaponSkill, DodgeSkill, ShieldUse,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	ParryValue, DamageReduciton,
	Speed,
	Concentration, Healing,
	Pickpockets, Stealth, OpenLocks, DisarmTraps,
	Survival,
	Level,
	HitsMaximum, ManaMaximum,
	Hits, Mana, Mood, Reputation, ParryCount, Money,
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
	SunSensitive, Slow, NormalWeaponImmunity, FireResistance,
	Blunt, Martial, TwoHanded, CutWoods, ArmorPirce,
	WearLeather, WearIron, WearLarge, WearShield, Coins,
	Female, Undead, Summoned, Ally, Enemy,
	Stun, Unaware,
};
enum target_s : unsigned char {
	You, YouOrAlly,
	EnemyOrAllyClose, EnemyOrAllyNear,
	EnemyClose, EnemyNear,
};
enum spell_s : unsigned char {
	Sleep, Web
};
enum feature_s : unsigned char {
	NoFeature,
	Tree, DeadTree, FootMud, FootHill, Grave, Statue,
	HiveHole, Hive, Hole, Plant, Herbs,
	Trap, Door,
};
enum tile_s : unsigned char {
	NoTile, WoodenFloor, Cave, DungeonFloor, Grass, GrassCorupted, Rock, Sand, Snow, Lava,
	Water, DarkWater, DeepWater,
	WallCave, WallBuilding, WallDungeon, WallFire, WallIce,
};
enum reaction_s : unsigned char {
	Indiferent, Friendly, Hostile,
};
extern stringbuilder console;
struct featable : flagable<4> {};
struct spellf : flagable<8> {};
extern point m2s(point v);
struct statable {
	char		abilities[Money + 1];
	void		create();
	int			getbonus(ability_s v) const;
};
struct abilityi : nameable {
	ability_s	basic;
};
struct racei : nameable {
};
struct classi : nameable {
	char		hd;
	char		cap, player;
};
struct feati : nameable {
};
struct weari : nameable {
	ability_s	ability;
};
struct indexa : adat<point> {
	void		select(point m, int range);
};
class actable {
	variant		kind; // Race or monster
public:
	void		actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female = false, char separator = '\n') const;
	static bool confirm(const char* format, ...);
	variant		getkind() const { return kind; }
	const char*	getname() const { return kind.getname(); }
	static void	pressspace();
	void		sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const;
	void		setkind(variant v) { kind = v; }
};
class movable : public actable {
	point		position;
	direction_s	direction;
	bool		mirror;
public:
	void		fixaction() const;
	void		fixappear() const;
	void		fixcantgo() const;
	void		fixdisappear() const;
	void		fixeffect(const char* id) const;
	static void	fixeffect(point position, const char* id);
	void		fixmovement() const;
	void		fixremove() const;
	void		fixvalue(const char* v, int color = 0) const;
	void		fixvalue(int v) const;
	bool		in(const rect& rc) const { return position.in(rc); }
	bool		ismirror() const { return mirror; }
	point		getposition() const { return position; }
	point		getsposition() const;
	void		setdirection(direction_s v);
	void		setposition(point m);
};
struct itemi : nameable {
	struct weaponi {
		char	parry, damage;
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
	variant		dress, use;
	void		paint() const;
	bool		is(feat_s v) const { return flags.is(v); }
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
		};
	};
public:
	explicit operator bool() const { return type != 0; }
	void		add(item& v);
	void		addname(stringbuilder& sb) const;
	bool		canequip(wear_s v) const;
	void		clear() { type = count = 0; }
	void		create(const char* id, int count = 1) { create(bsdata<itemi>::find(id), count); }
	void		create(const itemi* pi, int count = 1);
	void		drop(point m);
	int			getavatar() const { return geti().wear_index; }
	const itemi& geti() const { return bsdata<itemi>::elements[type]; }
	void		getinfo(stringbuilder& sb, bool need_name) const;
	int			getcost() const;
	int			getcount() const;
	int			getdamage() const;
	magic_s		getmagic() const { return magic; }
	const char*	getname() const { return geti().getname(); }
	class creature* getowner() const;
	void		getstatus(stringbuilder& sb) const;
	int			getweight() const;
	bool		is(ability_s v) const { return geti().ability == v; }
	bool		is(feat_s v) const { return geti().flags.is(v); }
	bool		is(wear_s v) const;
	bool		iscountable() const { return geti().count != 0; }
	bool		isidentified() const { return identified != 0; }
	void		setcount(int v);
};
struct itema : adat<item*> {
	item*		choose(const char* title) const;
	void		select(point m);
	void		select(creature* p);
	void		selectbackpack(creature* p);
};
struct itemground : item {
	point		position;
	static void dropitem(item& it);
};
struct wearable : movable {
	item		wears[Elbows + 1];
	int			money;
	void		additem(item& v);
	void		equip(item& v);
	int			getmoney() const { return money; }
	item*		getwear(wear_s id) { return wears + id; }
	const char*	getwearname(wear_s id) const;
	wear_s		getwearslot(const item* data) const;
	const item*	getwear(const void* data) const;
};
struct monsteri : nameable, statable {
	const char*	avatar;
	featable	feats;
	const char*	treasure;
	dice		appear, appear_outdoor;
	variants	use;
	const monsteri* parent;
	bool		is(feat_s v) const { return feats.is(v); }
	const monsteri& getbase() const { return parent ? parent->getbase() : *this; }
};
struct spellable {
	char		spells[Sleep + 1];
};
class creature : public wearable, public statable, public spellable {
	unsigned short class_id;
	statable	basic;
	spellf		active_spells;
	featable	feats;
	int			money;
	unsigned	experience;
	int			wait_seconds;
	void		advance(variant kind, int level);
	void		advance(variants elements);
	void		advance(variant element);
	void		dress(variant v, int multiplier);
	void		dress(variants v, int multiplier = 1);
	void		fixcantgo() const;
	void		interaction(point m);
	void		levelup();
	void		lookcreatures();
	void		paintbars() const;
	void		update();
	void		update_abilities();
	void		update_basic();
	void		update_wears();
public:
	typedef void (creature::*fnupdate)();
	operator bool() const { return abilities[Hits] > 0; }
	static creature* create(point m, variant v, variant character = {});
	void		act(const char* format, ...) const;
	void		acts(const char* format, ...) const;
	void		aimove();
	void		attack(creature& enemy, wear_s v, int bonus = 0, int damage_multiplier = 100);
	void		attackmelee(creature& enemy);
	void		checkmood() {}
	void		checkpoison() {}
	void		checksick() {}
	void		clear();
	void		damage(int v);
	void		finish();
	int			get(ability_s v) const { return abilities[v]; }
	int			get(spell_s v) const { return spells[v]; }
	const classi& getclass() const { return bsdata<classi>::elements[class_id]; }
	int			getdamage(wear_s w) const;
	void		getinfo(stringbuilder& sb) const;
	int			getlos() const { return get(LineOfSight); }
	int			getwait() const { return wait_seconds; }
	bool		is(condition_s v) const { return false; }
	bool		is(spell_s v) const { return active_spells.is(v); }
	bool		is(feat_s v) const { return feats.is(v); }
	bool		isactive() const;
	bool		isenemy(const creature& opponent) const;
	void		interaction(creature& opponent);
	void		makemove();
	ability_s	matchparry(wear_s attack, int attacker_strenght, int value) const;
	void		movestep(direction_s i);
	void		movestep(point m);
	void		moveto(point m);
	void		paint() const;
	void		restoration() {}
	void		remove(feat_s v) { feats.remove(v); }
	bool		roll(ability_s v, int bonus = 0) const;
	void		say(const char* format, ...) const { sayv(console, format, xva_start(format), getname(), is(Female)); }
	void		set(feat_s v) { feats.set(v); }
	void		unlink();
	void		wait(int rounds = 1) { wait_seconds += 100 * rounds; }
};
struct creaturea : adat<creature*> {
	void		match(feat_s v, bool keep);
	void		select(point m, int los);
	void		sort(point start);
};
struct advancement {
	variant		type;
	char		level;
	variants	elements;
};
struct visualeffect : nameable {
	res			resid;
	int			frame;
	unsigned char priority = 15;
	int			dy;
	void		paint() const;
};
struct sitei : nameable {
	char		site_count;
	variants	landscape;
	variants	sites;
	color		minimap;
	tile_s		walls, floors;
};
struct boosti {
	variant		parent;
	spell_s		effect;
	unsigned	stamp;
	constexpr explicit operator bool() const { return parent.operator bool(); }
	void		clear() { memset(this, 0, sizeof(*this)); }
	static void	remove(variant parent);
	static void	updateall();
};
struct geoposition {
	point		position;
	short		level;
	bool		isoutdoor() const { return level == 0; }
};
class gamei : public geoposition {
	unsigned	minutes;
	unsigned	restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day;
public:
	static void	all(creature::fnupdate proc);
	unsigned	getminutes() const { return minutes; }
	void		pass(unsigned minutes);
	void		passminute();
	static void	play();
	void		playminute();
	void		read();
	void		write();
};
namespace draw {
struct keybind {
	unsigned	key;
	const void*	data;
	constexpr explicit operator bool() const { return key != 0; }
};
bool			isnext();
}
inline int		d100() { return rand() % 100; }
extern areamap		area;
extern worldi		world;
extern creaturea	creatures, enemies;
extern gamei		game;
extern creature*	last_enemy;
extern int			last_hit, last_parry, last_hit_result, last_parry_result, last_damage;
extern creature*	player;
extern bool			show_detail_info;