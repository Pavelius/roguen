#include "ability.h"
#include "advancement.h"
#include "answers.h"
#include "areaf.h"
#include "areamap.h"
#include "class.h"
#include "collection.h"
#include "color.h"
#include "crt.h"
#include "dice.h"
#include "direction.h"
#include "duration.h"
#include "flagable.h"
#include "geoposition.h"
#include "global.h"
#include "hotkey.h"
#include "item.h"
#include "list.h"
#include "listcolumn.h"
#include "moveable.h"
#include "monster.h"
#include "pushvalue.h"
#include "quest.h"
#include "script.h"
#include "shape.h"
#include "site.h"
#include "speech.h"
#include "spell.h"
#include "talk.h"
#include "trigger.h"
#include "wear.h"
#include "wearable.h"

#pragma once

enum condition_s : unsigned char;

enum ability_s : unsigned char {
	LineOfSight,
	Strenght, Dexterity, Wits, Charisma,
	WeaponSkill, BalisticSkill, DodgeSkill, ShieldUse,
	Damage, DamageMelee, DamageRanged, DamageThrown,
	Armor,
	Speed, EnemyAttacks,
	Pickpockets, Stealth, OpenLocks, DisarmTraps,
	Survival,
	Level,
	HitsMaximum, ManaMaximum,
	Hits, Mana, Poison, Illness, Reputation, ParryCount, Experience, Money,
};
extern stringbuilder console;
class roomi;
class creature;
struct itema : collection<item> {
	void			select(point m);
	void			select(creature* p);
	void			selectbackpack(creature* p);
};
class ownerable {
	short unsigned	owner_id;
public:
	creature*		getowner() const { return bsdata<creature>::ptr(owner_id); }
	void			setowner(const creature* v) { bsset(owner_id, v); }
};
class creature : public wearable, public statable, public spellable, public ownerable {
	unsigned short	class_id, room_id;
	statable		basic;
	featable		feats, feats_active;
	point			moveorder, guardorder;
	unsigned		experience;
	int				wait_seconds;
	void			advance(variant kind, int level);
	void			advance(variants elements);
	void			advance(variant element);
	void			damage(const item& weapon, int effect);
	void			fixcantgo() const;
	bool			isfollowmaster() const;
	void			levelup();
	const speech*	matchfirst(const speecha& source) const;
	bool			matchspeech(variant v) const;
	bool			matchspeech(const variants& source) const;
	void			matchspeech(speecha& source) const;
	void			update();
	void			update_abilities();
	void			update_room_abilities();
public:
	geoposition		worldpos;
	operator bool() const { return abilities[Hits] > 0; }
	void			act(const char* format, ...) const;
	void			actp(const char* format, ...) const;
	void			apply(variant v);
	void			apply(const featable& v) { feats.add(v); }
	void			apply(const variants& source);
	void			attack(creature& enemy, wear_s v, int bonus = 0, int damage_multiplier = 100);
	void			attackmelee(creature& enemy);
	void			attackrange(creature& enemy);
	void			attackthrown(creature& enemy);
	bool			canhear(point i) const;
	bool			canshoot(bool interactive) const;
	bool			canthrown(bool interactive) const;
	void			cast(const spelli& e);
	void			cast(const spelli& e, int level, int mana);
	void			clear();
	void			dress(variant v, int multiplier);
	void			dress(variants v, int multiplier = 1);
	void			everyminute();
	void			every10minutes();
	void			every30minutes();
	void			every5minutes() {}
	void			every1hour() {}
	void			every4hour();
	static creature* create(point m, variant v, variant character = {}, bool female = false);
	void			damage(int v);
	void			finish();
	void			gainexperience(int v);
	int				get(ability_s v) const { return abilities[v]; }
	int				get(const spelli& e) const { return spells[bsid(&e)]; }
	const classi&	getclass() const { return bsdata<classi>::elements[class_id]; }
	int				getdamage(wear_s w) const;
	void			getinfo(stringbuilder& sb) const;
	int				getloh() const;
	int				getlos() const;
	const char*		getname() const { return actable::getname(); }
	static const char* getname(const void* p) { return ((creature*)p)->actable::getname(); }
	int				getpaymentcost() const;
	roomi*			getroom() const { return bsdata<roomi>::ptr(room_id); }
	void			getrumor(quest& e, stringbuilder& sb) const;
	int				getsellingcost() const;
	const char*		getspeech(const char* id, bool always_speak = true) const;
	int				getwait() const { return wait_seconds; }
	void			heal(int v);
	bool			is(condition_s v) const;
	bool			is(feat_s v) const { return feats.is(v) || feats_active.is(v); }
	bool			isallow(variant v) const;
	bool			isallow(const variants& source) const;
	bool			isenemy(const creature& opponent) const;
	bool			ishuman() const;
	static bool		isneed(const void* p);
	bool			ispresent() const;
	static bool		ispresent(const void* p) { return ((creature*)p)->ispresent(); }
	bool			isunaware() const { return wait_seconds >= 100 * 6; }
	bool			isvalid() const;
	void			interaction(creature& opponent);
	void			kill();
	void			logs(const char* format, ...) const { logv(format, xva_start(format), getname(), is(Female)); }
	void			makemove();
	void			makemovelong();
	void			movestep(direction_s i);
	void			movestep(point m);
	bool			moveto(point m);
	void			paint() const;
	void			paintbarsall() const;
	void			place(point m);
	void			remove(feat_s v) { feats.remove(v); }
	bool			roll(ability_s v, int bonus = 0) const;
	void			say(const char* format, ...) const { sayv(console, format, xva_start(format), getname(), is(Female)); }
	void			sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const;
	void			set(feat_s v) { feats.set(v); }
	void			set(ability_s i, int v) { abilities[i] = v; }
	void			setroom(const roomi* v) { bsset(room_id, v); }
	void			speech(const char* id, ...) const { sayv(console, getspeech(id), xva_start(id), getname(), is(Female)); }
	bool			speechneed();
	bool			speechrumor() const;
	bool			speechlocation() const;
	void			summon(point m, const variants& elements, int count, int level);
	bool			talk(const char* id, fncommand proc = 0);
	void			unlink();
	void			use(variants source);
	void			use(item& v);
	void			wait(int rounds = 1) { wait_seconds += 100 * rounds; }
	void			waitseconds(int value) { wait_seconds += value; }
};
struct creaturea : collection<creature> {
	void			match(feat_s v, bool keep);
	void			matchrange(point start, int v, bool keep);
	void			remove(const creature* v);
	void			select(point m, int los, bool visible, const creature* exclude);
	void			sort(point start);
};
class roomi : public geoposition, public ownerable {
	short unsigned	site_id;
public:
	rect			rc;
	static roomi*	add() { return bsdata<roomi>::add(); }
	point			center() const { return {(short)(rc.x1 + rc.width() / 2), (short)(rc.y1 + rc.height() / 2)}; }
	void			clear() { memset(this, 0, sizeof(*this)); setowner(0); }
	static roomi*	find(geoposition gp, point pt);
	const sitei&	geti() const { return bsdata<sitei>::elements[site_id]; }
	void			getrumor(stringbuilder& sb) const;
	const char*		getname() const { return geti().getname(); }
	static const char* getname(const void* p) { return ((roomi*)p)->getname(); }
	bool			is(feat_s v) const { return geti().feats.is(v); }
	bool			isexplored() const;
	bool			islocal() const;
	bool			ismarkable() const;
	bool			isnotable() const { return is(Notable); }
	void			set(const sitei* p) { bsset(site_id, p); }
};
typedef collection<roomi> rooma;
class gamei : public geoposition, public ownerable {
	unsigned		minutes;
	unsigned		restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	int				globals[128];
	void			setup_globals();
	static void		setup_rumors(int count);
public:
	static point	start_village;
	void			clear();
	static void		endgame();
	void			enter(point m, int level, const featurei* entry, direction_s appear_side);
	int				get(const globali& e) const { return globals[bsid(&e)]; }
	static int		getcount(variant v, int minimal = 1);
	unsigned		getminutes() const { return minutes; }
	static int		getpositivecount(variant v);
	static int		getrange(point m1, point m2);
	static bool		isvalid(point m) { return m.x > 0 && m.x < 256 && m.y > 0 && m.y < 256; }
	static void		next(fnevent proc);
	static void		newgame();
	static void		mainmenu();
	void			pass(unsigned minutes);
	void			passminute();
	static void		play();
	void			playminute();
	void			randomworld();
	void			read();
	void			set(const globali& e, int v);
	const char*		timeleft(unsigned end_stamp) const;
	void			write();
	static void		writelog();
};
namespace draw {
bool				isnext();
}
inline int			d100() { return rand() % 100; }
extern spella		allowed_spells;
extern areamap		area;
extern areaheadi	areahead;
extern creaturea	creatures, enemies, targets;
extern itema		items;
extern gamei		game;
extern creature*	last_enemy;
extern int			last_value;
extern ability_s	last_ability;
extern quest*		last_quest;
extern rect			last_rect;
extern const sitei*	last_site;
extern locationi*	last_location;
extern const sitegeni* last_method;
extern creature		*player, *opponent, *enemy;
extern int			window_width;
extern int			window_height;
point				center(const rect& rc);
void				dialog_message(const char* format);
int					getfloor();
int					getwall();