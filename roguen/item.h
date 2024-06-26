#include "feat.h"
#include "point.h"
#include "list.h"
#include "magic.h"
#include "wear.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;

struct itemi;

struct weaponi {
	char			damage, pierce, speed;
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
	char			charges, rotting;
	listi*			powers;
	char			chance_power;
	char			required[5];
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
			magic_s	magic : 2;
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
	void			clear() { type = count = stats = 0; }
	void			create(const char* id, int count = 1) { create(bsdata<itemi>::find(id), count); }
	void			create(const itemi* pi, int count = 1);
	void			createpower(int chance_power = 0);
	int				ischarge() const { return !iscountable() && (broken < 7); }
	void			damage(int value = 1);
	void			drop(point m);
	int				getavatar() const { return geti().wear_index; }
	int				getcharges() const { return ischargeable() ? broken : 0; }
	int				getcost() const;
	int				getcostall() const;
	int				getcount() const;
	const char*		getfullname(int price_percent = 0, bool uppercase = false) const;
	const itemi&	geti() const { return bsdata<itemi>::elements[type]; }
	void			getinfo(stringbuilder& sb) const;
	short unsigned	getkind() const { return type; }
	magic_s			getmagic() const { return magic; }
	const char*		getname() const;
	static const char* getname(const void* p) { return ((item*)p)->getfullname(); }
	variant			getpower() const;
	variants		getuse() const;
	int				getweight() const;
	bool			is(feat_s v) const;
	bool			is(magic_s v) const { return magic == v; }
	bool			is(wear_s v) const;
	bool			is(const itemi& v) const { return v == geti(); }
	bool			is(const itemi* p) const { return p == &geti(); }
	bool			is(const item& v) const { return type == v.type; }
	bool			ischarges() const { return getcharges() > 0; }
	bool			ischargeable() const;
	bool			iscountable() const { return geti().count != 0; }
	bool			iscursed() const { return is(Cursed);}
	bool			isidentified() const { return identified != 0; }
	bool			ismagical() const;
	bool			isranged() const { return geti().wear == RangedWeapon; }
	bool			isusable() const { return geti().use.size() != 0; }
	bool			iswounded() const { return iscountable() && broken > 0; }
	void			set(magic_s v) { magic = v; }
	void			setborken(int v) { if(!iscountable()) broken = v; }
	void			setcount(int v);
	void			setidentified(int v) { identified = v; }
	void			use();
};
struct itemground : item {
	point			position;
};
extern item* last_item;