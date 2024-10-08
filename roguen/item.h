#pragma once

#include "feat.h"
#include "point.h"
#include "list.h"
#include "magic.h"
#include "wear.h"
#include "variant.h"

enum ability_s : unsigned char;

struct itemi;

struct weaponi {
	char			damage, pierce;
	const itemi*	ammunition;
};
struct itemi : nameable {
	const char*		unidentified;
	short			count, weight, cost;
	short			avatar;
	wear_s			wear;
	char			wear_index;
	const itemi*	parent;
	weaponi			weapon;
	featable		feats;
	variants		wearing, use;
	char			chance_consume, chance_ill, rotting;
	listi*			powers;
	listi*			cursed;
	char			chance_power;
	char			required[3]; // required Strenght, Dexterity or Wits to use this item
	bool operator==(const itemi& v) const { return this == &v; }
	int				getindex() const { return this - bsdata<itemi>::elements; }
	bool			is(feat_s v) const { return feats.is(v); }
	bool			ismelee() const { return wear == MeleeWeapon || wear == MeleeWeaponOffhand; }
	void			paint() const; // Exported paint function
};
class item {
	unsigned short type;
	union {
		unsigned char stats;
		struct {
			magicn	magic : 2;
			unsigned char identified : 1;
			unsigned char personal : 1;
		};
	};
	union {
		unsigned char count;
		struct {
			unsigned char power : 5; // Item magical power index (1-31) or 0 - if no magical power
			unsigned char broken : 3; // Charges or Broken status
		};
	};
public:
	explicit operator bool() const { return type != 0; }
	void			add(item& v);
	bool			canequip(wear_s v) const;
	int				chance_consume() const;
	void			clear() { type = count = stats = 0; }
	void			create(const itemi* pi, int count = 1);
	void			createpower(int chance_power = 0);
	void			createmagical(int magical = 15, int cursed = 10, int artifact = 2);
	bool			damage();
	void			drop(point m);
	int				getavatar() const { return geti().wear_index; }
	int				getcost() const;
	int				getcost(int payment_cost) const;
	int				getcostall() const;
	int				getcount() const;
	int				geteffect() const;
	const char*		getfullname(int price_percent = 0, bool uppercase = true) const;
	const itemi&	geti() const { return bsdata<itemi>::elements[type]; }
	void			getexamine(stringbuilder& sb) const;
	void			getinfo(stringbuilder& sb) const;
	short unsigned	getkind() const { return type; }
	magicn			getmagic() const { return magic; }
	const char*		getname() const;
	static const char* getname(const void* p) { return ((item*)p)->getfullname(); }
	variant			getpower() const;
	variants		getuse() const;
	int				getweight() const;
	bool			is(feat_s v) const;
	bool			is(magicn v) const { return magic == v; }
	bool			is(wear_s v) const;
	bool			is(const itemi& v) const { return v == geti(); }
	bool			is(const itemi* p) const { return p == &geti(); }
	bool			is(const item& v) const { return type == v.type; }
	bool			isbroken() const { return !iscountable() && broken >= 7; }
	bool			iscountable() const { return !geti().powers; }
	bool			iscursed() const { return is(Cursed); }
	bool			isidentified() const { return identified != 0; }
	bool			ismagical() const;
	bool			ispersonal() const { return personal != 0; }
	bool			isranged() const { return geti().wear == RangedWeapon; }
	bool			isusable() const { return geti().use.size() != 0; }
	bool			isdamaged() const { return !iscountable() && broken > 0; }
	bool			isheavydamaged() const { return !iscountable() && broken >= 5; }
	void			repair(int value);
	void			set(magicn v) { magic = v; }
	void			setborken(int v) { if(!iscountable()) broken = v; }
	void			setcount(int v, const char* interactive = 0);
	void			setidentified(int v) { identified = v; }
	void			setpersonal(int v) { personal = v; }
	void			use();
};
struct itemground : item {
	point			position;
};
extern item* last_item;
void item_weight(stringbuilder& sb, int value, bool add_unit);