#include "answers.h"
#include "areamap.h"
#include "charname.h"
#include "collection.h"
#include "dice.h"
#include "direction.h"
#include "flagable.h"
#include "hotkey.h"
#include "color.h"
#include "crt.h"
#include "duration.h"
#include "list.h"
#include "listcolumn.h"
#include "pushvalue.h"
#include "randomizer.h"
#include "script.h"
#include "shape.h"
#include "speech.h"
#include "talk.h"
#include "variantc.h"
#include "world.h"

#pragma once

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
	Hits, Mana, Mood, Reputation, ParryCount, Experience, Money,
};
enum wear_s : unsigned char {
	Backpack, Potion, BackpackLast = Backpack + 15,
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, Ammunition,
	Head, Torso, Backward, Legs, Gloves, FingerRight, FingerLeft, Elbows,
};
enum magic_s : unsigned char {
	Mudane, Blessed, Cursed, Artifact,
};
enum condition_s : unsigned char {
	NoModifier,
	Identified, NPC, Random, ShowMinimapBullet,
	NoWounded, LightWounded, HeavyWounded,
	Busy,
	NoInt, AnimalInt, LowInt, AveInt, HighInt,
};
enum feat_s : unsigned char {
	Darkvision, Blunt, TwoHanded, CutWoods, Retaliate, Thrown,
	IgnoreWeb,
	Coins, Notable, Natural, KnowRumor, KnowLocation,
	Female, PlaceOwner, Undead, Summoned, Ally, Enemy,
	Stun, Unaware,
};
enum spell_s : unsigned char {
	CureWounds, Gate, Light, ManaRegeneration, Regeneration, Sleep, Teleport, Web,
	LastSpell = Web
};
enum feature_s : unsigned char {
	NoFeature,
	Tree, TreePalm, DeadTree, ThornBushe,
	FootMud, FootHill, Grave, Statue,
	HiveHole, Hive, Hole, Plant, Herbs,
	AltarGood, AltarNeutral, AltarEvil,
	Pit, Trap, Door, StairsUp, StairsDown, GatePortal,
};
enum tile_s : unsigned char {
	NoTile, WoodenFloor, Cave, DungeonFloor, Grass, GrassCorupted, Rock, Sand, Snow, Lava,
	Water, DarkWater, DeepWater,
	WallCave, WallBuilding, WallDungeon, WallFire, WallIce,
};
enum trigger_s : unsigned char {
	WhenCreatureP1EnterSiteP2, WhenCreatureP1Dead,
};
enum targetf : unsigned char {
	Item, Feature, You, Allies, Enemies, FarRange,
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
struct triggeri : nameable {
};
struct trigger {
	trigger_s	type;
	variant		p1, p2;
	variants	effect;
	void		apply(variant v) const;
	void		apply(variants v) const;
	static void	fire(trigger_s t, variant p1 = {}, variant p2 = {});
};
struct abilityi : nameable {
	ability_s	basic;
};
struct conditioni : nameable {
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
struct geoposition {
	point		position;
	short		level;
	constexpr bool operator==(const geoposition& e) const { return e.position == position && e.level == level; }
	constexpr bool operator!=(const geoposition& e) const { return e.position != position || e.level != level; }
	bool		isoutdoor() const { return level == 0; }
};
class actable {
	variant		kind; // Race or monster
	short unsigned name_id;
public:
	static void	actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female = false, char separator = '\n');
	static void	actvf(stringbuilder& sb, const char* name, bool female, char separator, const char* format, ...);
	static bool confirm(const char* format, ...);
	variant		getkind() const { return kind; }
	static const char* getlog();
	struct monsteri* getmonster() const;
	const char*	getname() const;
	bool		ischaracter() const;
	bool		iskind(variant v) const;
	static void	logv(const char* format, const char* format_param, const char* name, bool female);
	static void	pressspace();
	void		sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const;
	void		setkind(variant v) { kind = v; }
	void		setnoname() { name_id = 0xFFFF; }
	void		setname(unsigned short v) { name_id = v; }
};
class movable : public actable {
	point		position;
	direction_s	direction;
	bool		mirror;
public:
	void		fixaction() const;
	void		fixappear() const;
	void		fixability(ability_s i, int v) const;
	void		fixcantgo() const;
	void		fixdisappear() const;
	void		fixeffect(const char* id) const;
	static void	fixeffect(point position, const char* id);
	void		fixmovement() const;
	void		fixremove() const;
	void		fixshoot(point target, int frame) const;
	void		fixthrown(point target, const char* id, int frame) const;
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
	short		avatar;
	wear_s		wear;
	ability_s	ability;
	char		bonus;
	weaponi		weapon;
	featable	feats;
	char		wear_index;
	char		mistery;
	variant		dress;
	variants	use;
	bool operator==(const itemi& v) const { return this == &v; }
	const itemi* getammunition() const { return weapon.ammunition ? bsdata<itemi>::elements + weapon.ammunition : 0; }
	int			getindex() const { return this - bsdata<itemi>::elements; }
	bool		is(feat_s v) const { return feats.is(v); }
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
	int			getcostall() const;
	int			getcount() const;
	int			getdamage() const;
	magic_s		getmagic() const { return magic; }
	const char*	getname() const { return geti().getname(); }
	static const char* getname(const void* p) { return ((item*)p)->getfullname(); }
	const char*	getfullname() const;
	class creature* getowner() const;
	void		getstatus(stringbuilder& sb) const;
	int			getweight() const;
	bool		is(ability_s v) const { return geti().ability == v; }
	bool		is(feat_s v) const { return geti().feats.is(v); }
	bool		is(wear_s v) const;
	bool		is(const itemi& v) const;
	bool		iscountable() const { return geti().count != 0; }
	bool		isidentified() const { return identified != 0; }
	void		setcount(int v);
	void		use() { setcount(getcount() - 1); }
};
struct itema : collection<item> {
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
	unsigned short avatar;
	char		friendly;
	dice		appear;
	variants	use;
	monsteri*	parent;
	randomizeri* minions;
	variants	treasure;
	monsteri*	ally() const;
	const monsteri& getbase() const { return parent ? parent->getbase() : *this; }
	static bool	isboss(const void* p) { return ((monsteri*)p)->minions != 0; }
};
struct spellable {
	char		spells[LastSpell + 1];
};
struct skilli {
	ability_s	skill;
	int			value;
};
typedef skilli defencet[3];
class roomi;
class ownerable {
	short unsigned owner_id;
public:
	creature*	getowner() const { return bsdata<creature>::ptr(owner_id); }
	void		setowner(const creature* v) { bsset(owner_id, v); }
};
class creature : public wearable, public statable, public spellable, public ownerable {
	unsigned short class_id;
	unsigned short room_id;
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
	void		droptreasure() const;
	void		fixcantgo() const;
	void		fixdamage(int total, int damage_weapon, int damage_strenght, int damage_armor, int damage_skill, int damage_parry) const;
	int			getblocking(const item& enemy_weapon, const item& weapon, int value) const;
	int			getparrying(const item& enemy_weapon, const item& weapon, int value) const;
	int			getmightpenalty(int enemy_strenght) const;
	void		interaction(point m);
	void		levelup();
	void		lookcreatures() const;
	void		lookitems() const;
	const speech* matchfirst(const speecha& source) const;
	bool		matchspeech(variant v) const;
	bool		matchspeech(const variants& source) const;
	void		matchspeech(speecha& source) const;
	void		paintbars() const;
	void		restore(ability_s a, ability_s m, ability_s test);
	void		update();
	void		update_abilities();
	void		update_basic();
	void		update_boost();
	void		update_wears();
public:
	geoposition worldpos;
	typedef void (creature::*fnupdate)();
	operator bool() const { return abilities[Hits] > 0; }
	void		act(const char* format, ...) const;
	void		actp(const char* format, ...) const;
	void		aimove();
	void		apply(variant v);
	void		apply(spell_s v, unsigned minutes);
	bool		apply(spell_s v, int level, bool run);
	void		attack(creature& enemy, wear_s v, int bonus = 0, int damage_multiplier = 100);
	void		attackmelee(creature& enemy);
	void		attackrange(creature& enemy);
	void		attackthrown(creature& enemy);
	bool		canhear(point i) const;
	bool		canshoot(bool interactive) const;
	bool		canthrown(bool interactive) const;
	void		everyminute();
	void		every10minutes() {}
	void		every30minutes();
	void		every5minutes() {}
	void		every1hour() {}
	void		every4hour();
	void		clear();
	static creature* create(point m, variant v, variant character = {}, bool female = false);
	void		damage(int v);
	void		finish();
	void		gainexperience(int v);
	int			get(ability_s v) const { return abilities[v]; }
	int			get(spell_s v) const { return spells[v]; }
	const classi& getclass() const { return bsdata<classi>::elements[class_id]; }
	int			getdamage(wear_s w) const;
	void		getdefence(int attacker_strenght, const item& attacker_weapon, defencet& result) const;
	void		getinfo(stringbuilder& sb) const;
	int			getloh() const;
	int			getlos() const;
	roomi*		getroom() const { return bsdata<roomi>::ptr(room_id); }
	void		getrumor(struct dungeon& e, stringbuilder& sb) const;
	const char* getspeech(const char* id) const;
	int			getwait() const { return wait_seconds; }
	void		heal(int v);
	bool		is(condition_s v) const;
	bool		is(spell_s v) const { return active_spells.is(v); }
	bool		is(feat_s v) const { return feats.is(v); }
	bool		isenemy(const creature& opponent) const;
	bool		isplayer() const;
	bool		isvalid() const;
	void		interaction(creature& opponent);
	void		kill();
	void		logs(const char* format, ...) const { logv(format, xva_start(format), getname(), is(Female)); }
	void		lookenemies();
	void		makemove();
	void		movestep(direction_s i);
	void		movestep(point m);
	void		moveto(point m);
	void		paint() const;
	void		paintbarsall() const;
	void		place(point m);
	void		restoration() {}
	void		remove(feat_s v) { feats.remove(v); }
	bool		roll(ability_s v, int bonus = 0) const;
	void		say(const char* format, ...) const { sayv(console, format, xva_start(format), getname(), is(Female)); }
	void		sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const;
	void		set(feat_s v) { feats.set(v); }
	void		set(ability_s i, int v) { abilities[i] = v; }
	void		setroom(const roomi* v) { bsset(room_id, v); }
	void		speech(const char* id, ...) const { sayv(console, getspeech(id), xva_start(id), getname(), is(Female)); }
	bool		speechrumor() const;
	bool		speechlocation() const;
	bool		talk(const char* id);
	void		unlink();
	void		update_room();
	void		use(variants source);
	void		use(item& v);
	void		wait(int rounds = 1) { wait_seconds += 100 * rounds; }
};
struct creaturea : collection<creature> {
	void		match(feat_s v, bool keep);
	void		matchrange(point start, int v, bool keep);
	void		remove(const creature* v);
	void		select(point m, int los, bool visible, const creature* exclude);
	void		sort(point start);
};
struct advancement {
	variant		type;
	char		level;
	variants	elements;
};
struct visualeffect : nameable {
	res			resid;
	short unsigned frame, feats;
	unsigned char priority = 15;
	int			dy;
	void		paint(unsigned char random) const;
};
struct sitegeni;
struct sitei : nameable {
	typedef void (sitei::*fnproc)() const;
	variants	landscape, sites, loot;
	color		minimap;
	tile_s		walls, floors;
	char		darkness, chance_finale;
	point		offset;
	featable	feats;
	const shapei* shape;
	const sitegeni*	global;
	const sitegeni*	local;
	const sitegeni*	global_finish;
	void		building() const;
	void		cityscape() const;
	void		corridors() const;
	void		dungeon() const;
	void		fillfloor() const;
	void		fillwalls() const;
	void		fillwallsall() const;
	void		nogenerate() const {}
	void		outdoor() const;
	void		room() const;
};
class siteable {
	short unsigned site_id;
public:
	explicit operator bool() const { return site_id != 0xFFFF; }
	sitei*		getsite() const { return bsdata<sitei>::ptr(site_id); }
	void		setsite(const sitei* v) { bsset(site_id, v); }
};
struct sitegeni : nameable {
	sitei::fnproc proc;
};
struct boosti {
	variant		parent;
	spell_s		effect;
	unsigned	stamp;
	constexpr explicit operator bool() const { return parent.operator bool(); }
	static boosti* add(variant parent, spell_s effect);
	void		clear() { memset(this, 0, sizeof(*this)); }
	static void	remove(variant parent);
	static void	updateall();
};
struct dungeon {
	point		position;
	const sitei* entrance;
	const sitei* modifier;
	const sitei* level;
	const sitei* final_level;
	monsteri*	guardian;
	char		rumor;
	variant		reward, twist; // Can't be random table
	constexpr operator bool() const { return level != 0; }
	static dungeon*	add(point position);
	static dungeon*	add(point position, sitei* modifier, sitei* type, variant reward);
	void		clear() { memset(this, 0, sizeof(*this)); }
	static dungeon* find(point position);
};
class roomi : public geoposition, public siteable, public ownerable {
	unsigned char ideftified : 1;
	unsigned char explored : 1;
public:
	rect		rc;
	static void* operator new(size_t size) { return bsdata<roomi>::addz(); }
	void		clear() { memset(this, 0, sizeof(*this)); setsite(0); setowner(0); }
	static roomi* find(geoposition gp, point pt);
	void		getrumor(stringbuilder& sb) const;
	static bool	notknown(const void* object);
	const char*	getname() const { return getsite()->getname(); }
	bool		is(condition_s v) const;
	bool		is(feat_s v) const;
};
struct location : siteable {
	char		darkness;
	void		clear();
};
struct spelli : nameable {
	int			mana;
	unsigned	flags;
	duration_s	duration;
	dice		count;
	int			getcount(int level) const;
	bool		is(targetf v) const { return (flags & FG(v)) != 0; }
	static void	linerow(const void* object);
};
struct spella : collection<spelli> {
	spelli*		choose(const char* title, const char* cancel) const;
	void		select(const spellable* p);
};
class gamei : public geoposition, public ownerable {
	unsigned	minutes;
	unsigned	restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day;
public:
	static point start_village;
	variantc	events;
	static void	all(creature::fnupdate proc);
	void		createarea();
	static void endgame();
	void		enter(point m, int level, feature_s feature, direction_s appear_side);
	unsigned	getminutes() const { return minutes; }
	static int	getpositivecount(variant v);
	static int	getrange(point m1, point m2);
	static bool	isvalid(point m) { return m.x >= 0 && m.x < 256 && m.y >= 0 && m.y < 256; }
	static void newgame();
	void		pass(unsigned minutes);
	void		passminute();
	static void	play();
	void		playminute();
	void		read();
	static bool	testcount(variant v);
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
extern itema		items;
extern gamei		game;
extern creature*	last_enemy;
extern int			last_hit, last_parry, last_hit_result, last_parry_result, last_damage;
extern variant		last_variant;
extern dungeon*		last_dungeon;
extern rect			last_rect;
extern sitei*		last_site;
extern creature		*player, *opponent, *enemy;
extern bool			stop_script;
extern int			window_width;
extern int			window_height;
point				center(const rect& rc);