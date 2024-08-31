#pragma once

#include "ability.h"
#include "advancement.h"
#include "answers.h"
#include "areaf.h"
#include "areamap.h"
#include "craft.h"
#include "dice.h"
#include "moveable.h"
#include "monster.h"
#include "quest.h"
#include "site.h"
#include "spell.h"
#include "wearable.h"

class creature : public wearable, public statable, public spellable, public ownerable, public receipti {
	unsigned short	room_id, enemy_id, charmer_id, fear_id;
	void			cleanup();
	void			update_abilities();
public:
	int				experience, wait_seconds;
	statable		basic;
	featable		feats, feats_active;
	geoposition		worldpos;
	point			moveorder, guardorder;
	operator bool() const { return abilities[Hits] > 0; }
	bool			act(const char* action, const char* id = 0, ...) const;
	bool			actp(const char* action, const char* id = 0, ...) const;
	void			additem(item& v);
	void			apply(const featable& v) { feats.add(v); }
	bool			canhear(point i) const;
	bool			canremove(item& it) const;
	bool			canspeak() const { return abilities[Wits] >= 5; }
	void			clear();
	void			equipi(short unsigned type, int count);
	void			equip(item& v);
	void			equip(const itemi* pi, int count);
	void			damage(int v);
	void			finish();
	void			fixappear();
	int				get(ability_s v) const { return abilities[v]; }
	int				getcarry() const;
	creature*		getcharmer() const;
	creature*		getenemy() const;
	creature*		getfear() const;
	void			getinfo(stringbuilder& sb) const;
	int				getloh() const;
	int				getlos() const;
	int				getmaximum(ability_s v) const;
	const char*		getname() const { return actable::getname(); }
	static const char* getname(const void* p) { return ((creature*)p)->actable::getname(); }
	creature*		getowner() const;
	int				getpaymentcost() const;
	int				getreputation() const { return abilities[Reputation] / 20; }
	roomi*			getroom() const;
	void			getrumor(quest& e, stringbuilder& sb) const;
	int				getsellingcost() const;
	int				getwait() const { return wait_seconds; }
	bool			is(areaf v) const;
	bool			is(ability_s v) const { return get(v) > 0; }
	bool			is(feat_s v) const { return feats.is(v) || feats_active.is(v); }
	bool			is(feat_s v, const item& i) const { return feats.is(v) || i.is(v); }
	bool			isallow(const item& it) const;
	bool			isenemy(const creature& opponent) const;
	bool			isexpert(ability_s v) const { return get(v) >= 30; }
	bool			isevil() const { return abilities[Reputation] < -30; }
	bool			isfollowmaster() const;
	bool			isgood() const { return abilities[Reputation] >= 0; }
	bool			ishuman() const;
	bool			ismaster(ability_s v) const { return get(v) >= 60; }
	bool			ispresent() const;
	bool			istired() const { return abilities[Mood] <= -10; }
	bool			isunaware() const { return wait_seconds >= 25 * 4 * 6; }
	bool			isvalid() const;
	void			kill();
	void			logs(const char* format, ...) const;
	bool			moveto(point m);
	void			paint() const;
	void			paintbarsall() const;
	void			place(point m);
	void			remove(feat_s v) { feats.remove(v); }
	bool			resist(feat_s resist, feat_s immunity) const;
	bool			roll(ability_s v, int bonus = 0) const;
	void			say(const char* format, ...) const;
	void			set(feat_s v) { feats.set(v); }
	void			set(ability_s i, int v) { abilities[i] = v; }
	void			setcharmer(const creature* v);
	void			setenemy(const creature* v);
	void			setfear(const creature* v);
	void			setroom(const roomi* v);
	void			slowdown(int seconds) { wait_seconds += seconds; }
	bool			speechrumor() const;
	bool			speechlocation() const;
	bool			speak(const char* action, const char* id = 0, ...) const;
	void			unlink();
	void			update();
	void			wait(int rounds = 1) { wait_seconds += 100 * rounds; }
	void			waitseconds(int value) { wait_seconds += value; }
};
struct creaturea : collection<creature> {
	void			match(feat_s v, bool keep);
	void			match(fnvisible proc, bool keep) { collectiona::match(proc, keep); }
	void			matchrange(point start, int v, bool keep);
	void			remove(const creature* v);
	void			select(point m, int los, bool visible, const creature* exclude);
	void			sort(point start);
};

extern creature *player, *opponent;
extern creaturea creatures, enemies, targets;
extern int last_value, last_roll_result;
extern bool last_roll_successed;
extern rect last_rect;
extern int window_width, window_height;

void attack_melee(int bonus);
void attack_range(int bonus);
void attack_thrown(int bonus);
void cast_spell(const spelli& e, int mana, bool silent);
void cast_spell(const spelli& e);
void damage_backpack_item(wear_s type, int chance, int count = 1);
void dialog_message(const char* format);
bool is_ally(const void* object);
bool make_hostile(creature* player, const creature* opponent);
void make_move();
void move_step(direction_s v);
void pay_action();
void use_item(item& v);

creature* player_create(point m, variant kind, bool female);

int get_maximum_faith(creature* p);