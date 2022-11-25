#include "feat.h"
#include "magic.h"
#include "point.h"
#include "wear.h"
#include "variant.h"

#pragma once

enum ability_s : unsigned char;

struct itemi : nameable {
	struct weaponi {
		char		parry, enemy_parry;
		char		block, enemy_block, block_ranged;
		char		damage, pierce;
		char		wait;
		short 		ammunition;
	};
	int				cost, weight, count;
	short			avatar;
	wear_s			wear;
	ability_s		ability;
	char			bonus;
	weaponi			weapon;
	featable		feats;
	char			wear_index;
	char			mistery;
	variant			dress;
	variants		use;
	bool operator==(const itemi& v) const { return this == &v; }
	const itemi*	getammunition() const { return weapon.ammunition ? bsdata<itemi>::elements + weapon.ammunition : 0; }
	int				getindex() const { return this - bsdata<itemi>::elements; }
	bool			is(feat_s v) const { return feats.is(v); }
	void			paint() const; // Exported paint function
};
class item {
	unsigned short	type;
	union {
		unsigned short count;
		struct {
			magic_s	magic : 2;
			unsigned char identified : 1;
			unsigned char broken : 2;
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
	short unsigned	getkind() const { return type; }
	int				getavatar() const { return geti().wear_index; }
	const itemi&	geti() const { return bsdata<itemi>::elements[type]; }
	void			getinfo(stringbuilder& sb, bool need_name) const;
	int				getcost() const;
	int				getcostall() const;
	int				getcount() const;
	int				getdamage() const;
	magic_s			getmagic() const { return magic; }
	const char*		getname() const { return geti().getname(); }
	static const char* getname(const void* p) { return ((item*)p)->getfullname(); }
	const char*		getfullname(int price_percent = 0) const;
	bool			is(ability_s v) const { return geti().ability == v; }
	bool			is(feat_s v) const { return geti().feats.is(v); }
	bool			is(wear_s v) const;
	bool			is(const itemi& v) const { return v == geti(); }
	bool			is(const itemi* p) const { return p == &geti(); }
	bool			is(const item& v) const { return type == v.type; }
	bool			iscountable() const { return geti().count != 0; }
	bool			isidentified() const { return identified != 0; }
	void			set(magic_s v);
	void			setcount(int v);
	void			setidentified(int v) { identified = v; }
	void			use() { setcount(getcount() - 1); }
	void			usecharge() { if(charges) charges--; else use(); }
};
struct itemground : item {
	point			position;
};
