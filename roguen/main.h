#include "ability.h"
#include "advancement.h"
#include "answers.h"
#include "areaf.h"
#include "areamap.h"
#include "class.h"
#include "color.h"
#include "crt.h"
#include "dice.h"
#include "direction.h"
#include "duration.h"
#include "flagable.h"
#include "game.h"
#include "global.h"
#include "hotkey.h"
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
#include "wearable.h"

#pragma once

enum condition_s : unsigned char;

class creature;
struct itema : collection<item> {
	void			select(point m);
	void			select(creature* p);
	void			selectbackpack(creature* p);
};
class creature : public wearable, public statable, public spellable, public ownerable {
	unsigned short	class_id, room_id;
	statable		basic;
	featable		feats, feats_active;
	point			moveorder, guardorder;
	unsigned		experience;
	int				satiation, wait_seconds;
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
	void			attackmelee(creature& enemy);
	void			attackrange(creature& enemy);
	void			attackthrown(creature& enemy);
	bool			canhear(point i) const;
	bool			canshoot(bool interactive) const;
	bool			canthrown(bool interactive) const;
	void			cast(const spelli& e);
	void			cast(const spelli& e, int level, int mana);
	void			clear();
	void			dress(variant v);
	void			dress(variants v);
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
	void			update();
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
extern creature *player, *opponent, *enemy;
extern creaturea creatures, enemies, targets;
namespace draw {
bool				isnext();
}
inline int d100() { return rand() % 100; }
extern spella allowed_spells;
extern itema items;
extern int last_value, last_cap;
extern ability_s last_ability;
extern quest* last_quest;
extern rect last_rect;
extern int window_width, window_height;
point center(const rect& rc);
void dialog_message(const char* format);