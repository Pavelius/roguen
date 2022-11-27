#include "feat.h"
#include "magic.h"
#include "point.h"
#include "list.h"
#include "wear.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;

struct itemstat : nameable {
	char			damage, armor, skill, parry, dodge, block, block_ranged, pierce, speed, mistery;
	char			enemy_parry, enemy_block;
	short			weight, cost;
	featable		feats;
};
struct itemi : itemstat {
	short			count;
	short			avatar;
	wear_s			wear;
	char			wear_index;
	variants		use;
	const listi*	prefix;
	const listi*	suffix;
	const itemi*	ammunition;
	bool operator==(const itemi& v) const { return this == &v; }
	int				getindex() const { return this - bsdata<itemi>::elements; }
	bool			is(feat_s v) const { return feats.is(v); }
	bool			isblock() const { return block || block_ranged; }
	bool			iscountable() const { return count != 0; }
	bool			ismelee() const { return wear == MeleeWeapon || wear == MeleeWeaponOffhand; }
	void			paint() const; // Exported paint function
};
class item {
	unsigned short	type;
	union {
		unsigned short count;
		struct {
			magic_s	magic : 2;
			unsigned char broken : 2;
			unsigned char identified : 1;
			unsigned char charges : 3;
			unsigned char prefix : 4;
			unsigned char suffix : 4;
		};
	};
public:
	explicit operator bool() const { return type != 0; }
	void			add(item& v);
	bool			canequip(wear_s v) const;
	void			clear() { type = count = 0; }
	void			create(const char* id, int count = 1) { create(bsdata<itemi>::find(id), count); }
	void			create(const itemi* pi, int count = 1);
	void			damage() {}
	void			drop(point m);
	int				get(unsigned fo) const;
	short unsigned	getkind() const { return type; }
	int				getavatar() const { return geti().wear_index; }
	const itemi&	geti() const { return bsdata<itemi>::elements[type]; }
	void			getinfo(stringbuilder& sb) const;
	int				getcost() const;
	int				getcostall() const;
	int				getcount() const;
	magic_s			getmagic() const { return magic; }
	const char*		getname() const { return geti().getname(); }
	static const char* getname(const void* p) { return ((item*)p)->getfullname(); }
	const char*		getfullname(int price_percent = 0) const;
	const itemstat* getprefix() const;
	const itemstat* getsuffix() const;
	int				getweight() const;
	bool			is(feat_s v) const;
	bool			is(wear_s v) const;
	bool			is(const itemi& v) const { return v == geti(); }
	bool			is(const itemi* p) const { return p == &geti(); }
	bool			is(const item& v) const { return type == v.type; }
	bool			iscountable() const { return geti().iscountable(); }
	bool			isidentified() const { return identified != 0; }
	void			set(magic_s v);
	void			setcount(int v);
	void			setidentified(int v) { identified = v; }
	void			upgrade(int chance_suffix, int chance_prefix, int level);
	void			use() { setcount(getcount() - 1); }
	void			usecharge() { if(charges) charges--; else use(); }
};
struct itemground : item {
	point			position;
};
