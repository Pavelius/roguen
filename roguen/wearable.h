#include "item.h"
#include "moveable.h"

#pragma once

struct wearable : movable {
	item			wears[Legs + 1];
	int				money;
	void			addcoins(int v);
	void			additem(item& v);
	slice<item>		backpack() { return slice<item>(wears + Backpack, wears + BackpackLast + 1); }
	void			equip(item& v);
	void			equip(const itemi* pi, int count);
	slice<item>		equipment() { return slice<item>(wears + MeleeWeapon, wears + Legs + 1); }
	bool			iswear(const void* p) const { return p >= wears && p <= wears + Legs; }
	slice<item>		gears() { return slice<item>(wears + Torso, wears + Legs + 1); }
	int				getmoney() const { return money; }
	item*			getwear(wear_s id) { return wears + id; }
	wear_s			getwearslot(const item* data) const;
	const item*		getwear(const void* data) const;
	bool			useitem(const itemi* pi, bool run);
	slice<item>		weapons() { return slice<item>(wears + MeleeWeapon, wears + RangedWeapon + 1); }
};
extern wear_s last_player_used_wear;

