#include "item.h"
#include "moveable.h"

#pragma once

struct wearable : movable {
	item			wears[Elbows + 1];
	int				money;
	void			addcoins(unsigned v);
	void			additem(item& v);
	slice<item>		backpack() { return slice<item>(wears + Backpack, wears + BackpackLast + 1); }
	void			equip(item& v);
	slice<item>		equipment() { return slice<item>(wears + MeleeWeapon, wears + Elbows + 1); }
	bool			iswear(const void* p) const { return p >= wears && p <= wears + Elbows; }
	slice<item>		gears() { return slice<item>(wears + Head, wears + Elbows + 1); }
	int				getmoney() const { return money; }
	item*			getwear(wear_s id) { return wears + id; }
	const char*		getwearname(wear_s id) const;
	wear_s			getwearslot(const item* data) const;
	const item*		getwear(const void* data) const;
	bool			useitem(const itemi* pi);
	slice<item>		weapons() { return slice<item>(wears + MeleeWeapon, wears + RangedWeapon + 1); }
};
