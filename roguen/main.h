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
const int version_build = 5;

enum class res {
	Monsters,
	Borders, Floor, Walls, Decals, Features, Shadows, Items,
	Attack, Conditions, Splash,
	Fow, Missile,
	PCBody, PCArms, PCAccessories,
};
enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits, Charisma,
	WeaponSkill, BalisticSkill, DodgeSkill, ShieldUse,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	DamageReduciton,
	Speed, EnemyAttacks,
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
	Blunt, TwoHanded, CutWoods, Retaliate,
	Coins,
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
	void		add(ability_s i, int v = 1) { abilities[i] += v; }
	void		create();
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
};
struct indexa : adat<point> {
	void		select(point m, int range);
};
class actable {
	variant		kind; // Race or monster
public:
	static void	actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female = false, char separator = '\n');
	static bool confirm(const char* format, ...);
	variant		getkind() const { return kind; }
	static const char* getlog();
	const char*	getname() const { return kind.getname(); }
	static void	logv(const char* format, const char* format_param, const char* name, bool female);
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
	void		fixshoot(point target, const char* id, int frame) const;
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
		char	parry, enemy_parry;
		char	block, enemy_block, block_ranged;
		char	damage, pierce;
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
	bool operator==(const itemi& v) const { return this == &v; }
	const itemi* getammunition() const { return weapon.ammunition ? bsdata<itemi>::elements + weapon.ammunition : 0; }
	bool		is(feat_s v) const { return flags.is(v); }
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
	const char*	getfullname() const;
	class creature* getowner() const;
	void		getstatus(stringbuilder& sb) const;
	int			getweight() const;
	bool		is(ability_s v) const { return geti().ability == v; }
	bool		is(feat_s v) const { return geti().flags.is(v); }
	bool		is(wear_s v) const;
	bool		is(const itemi& v) const;
	bool		iscountable() const { return geti().count != 0; }
	bool		isidentified() const { return identified != 0; }
	void		setcount(int v);
	void		use() { setcount(getcount() - 1); }
};
struct itema : adat<item*> {
	item*		choose(const char* title, const char* cancel = 0) const;
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
struct skilli {
	ability_s	skill;
	int			value;
};
typedef skilli defencet[3];
class creature : public wearable, public statable, public spellable {
	unsigned short class_id;
	unsigned short enemy_id, master_id;
	statable	basic;
	spellf		active_spells;
	featable	feats;
	int			money;
	unsigned	experience;
	int			wait_seconds;
	void		advance(variant kind, int level);
	void		advance(variants elements);
	void		advance(variant element);
	void		damage(const item& weapon, int effect);
	void		dress(variant v, int multiplier);
	void		dress(variants v, int multiplier = 1);
	void		fixcantgo() const;
	void		fixdamage(int total, int damage_weapon, int damage_strenght, int damage_armor, int damage_skill, int damage_parry) const;
	int			getblocking(const item& enemy_weapon, const item& weapon, int value) const;
	unsigned short getid() const { return this - bsdata<creature>::elements; }
	int			getparrying(const item& enemy_weapon, const item& weapon, int value) const;
	int			getmightpenalty(int enemy_strenght) const;
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
	void		actp(const char* format, ...) const;
	void		aimove();
	void		attack(creature& enemy, wear_s v, int bonus = 0, int damage_multiplier = 100);
	void		attackmelee(creature& enemy);
	void		attackrange(creature& enemy);
	bool		canshoot(bool interactive) const;
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
	void		getdefence(int attacker_strenght, const item& attacker_weapon, defencet& result) const;
	creature*	getenemy() const { return enemy_id == 0xFFFF ? 0 : bsdata<creature>::elements + enemy_id; }
	void		getinfo(stringbuilder& sb) const;
	int			getlos() const { return get(LineOfSight); }
	int			getwait() const { return wait_seconds; }
	bool		is(condition_s v) const { return false; }
	bool		is(spell_s v) const { return active_spells.is(v); }
	bool		is(feat_s v) const { return feats.is(v); }
	bool		isactive() const;
	bool		isenemy(const creature& opponent) const;
	bool		isplayer() const;
	void		interaction(creature& opponent);
	void		logs(const char* format, ...) const { logv(format, xva_start(format), getname(), is(Female)); }
	void		lookenemies();
	void		makemove();
	void		movestep(direction_s i);
	void		movestep(point m);
	void		moveto(point m);
	void		paint() const;
	void		place(point m);
	void		restoration() {}
	void		remove(feat_s v) { feats.remove(v); }
	bool		roll(ability_s v, int bonus = 0) const;
	void		say(const char* format, ...) const { sayv(console, format, xva_start(format), getname(), is(Female)); }
	void		set(feat_s v) { feats.set(v); }
	void		set(ability_s i, int v) { abilities[i] = v; }
	void		setenemy(const creature* p) { enemy_id = p ? p->getid() : 0xFFFF; }
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
	short unsigned frame, flags;
	unsigned char priority = 15;
	int			dy;
	void		paint(unsigned char random) const;
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
	point		position = {0, 0};
	short		level = 0;
	bool		isoutdoor() const { return level == 0; }
};
struct location {
	char		tile[32];
	void		clear();
	void		settile(const char* id);
};
class gamei : public geoposition {
	unsigned	minutes;
	unsigned	restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day;
public:
	short unsigned player_id;
	static void	all(creature::fnupdate proc);
	static void endgame();
	void		enter(point m, int level, direction_s appear_side);
	unsigned	getminutes() const { return minutes; }
	static void newgame();
	void		pass(unsigned minutes);
	void		passminute();
	static void	play();
	void		playminute();
	void		read();
	void		write();
	static void writelog();
};
namespace draw {
struct keybind {
	unsigned	key;
	const void*	data;
	constexpr explicit operator bool() const { return key != 0; }
};
bool				isnext();
}
inline int			d100() { return rand() % 100; }
extern areamap		area;
extern location		loc;
extern worldi		world;
extern creaturea	creatures, enemies;
extern gamei		game;
extern creature*	last_enemy;
extern int			last_hit, last_parry, last_hit_result, last_parry_result, last_damage;
extern creature*	player;