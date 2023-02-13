#include "feat.h"
#include "point.h"
#include "list.h"
#include "wear.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;

enum magic_s : unsigned char {
	Mundane, Cursed, Blessed, Mighty,
};

struct itemi;

struct weaponi {
	char			damage, skill, pierce, speed;
	const itemi*	ammunition;
};
struct itemi : nameable {
	short			count, weight, cost;
	short			avatar;
	wear_s			wear;
	char			wear_index;
	const itemi*	parent;
	weaponi			weapon;
	featable		feats;
	variants		wearing, use, use_cursed, use_blessed;
	char			rotting;
	listi*			powers;
	char			chance_power;
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
			unsigned char identified : 1;
			unsigned char identified_cub : 1;
			unsigned char personal : 1;
			magic_s magic : 2; // Cursed Uncursed Blessed
		};
	};
	union {
		unsigned char count;
		struct {
			unsigned char power : 5; // Item magical power. 0 - if none
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
	void			createpower(int chance_power);
	void			damage();
	void			drop(point m);
	int				get(unsigned fo) const;
	short unsigned	getkind() const { return type; }
	int				getavatar() const { return geti().wear_index; }
	const itemi&	geti() const { return bsdata<itemi>::elements[type]; }
	void			getinfo(stringbuilder& sb) const;
	int				getcost() const;
	int				getcostall() const;
	int				getcount() const;
	const char*		getname() const;
	static const char* getname(const void* p) { return ((item*)p)->getfullname(); }
	const char*		getfullname(int price_percent = 0) const;
	variant			getpower() const;
	variants		getuse() const;
	int				getweight() const;
	bool			is(feat_s v) const;
	bool			is(wear_s v) const;
	bool			is(magic_s v) const { return magic == v; }
	bool			is(const itemi& v) const { return v == geti(); }
	bool			is(const itemi* p) const { return p == &geti(); }
	bool			is(const item& v) const { return type == v.type; }
	static bool		iscondition(const void* object, int v);
	bool			isidentified() const { return identified != 0; }
	bool			iscountable() const { return geti().count != 0; }
	void			set(magic_s v) { magic = v; }
	void			setcount(int v);
	void			setidentified(int v) { identified = v; }
	void			setidentifiedcub(int v) { identified_cub = v; }
	void			use() { setcount(getcount() - 1); }
};
struct itemground : item {
	point			position;
};
extern item *last_item;
