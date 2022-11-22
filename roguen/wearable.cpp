#include "wearable.h"

void wearable::addcoins(unsigned v) {
	if(!v)
		return;
	//if(answers::console)
	//	answers::console->addn(getnm("YouTakeCoins"), v);
	money += v;
}

void wearable::additem(item& v) {
	if(v.is(Coins)) {
		addcoins(v.getcostall());
		v.clear();
		return;
	}
	// Try stack existing item
	for(auto i = Backpack; i <= BackpackLast; i = (wear_s)(i + 1)) {
		if(!v)
			break;
		if(wears[i])
			wears[i].add(v);
	}
	if(v) {
		// Try add new item
		for(auto i = Backpack; i <= BackpackLast; i = (wear_s)(i + 1)) {
			if(!wears[i]) {
				wears[i] = v;
				v.clear();
				break;
			}
		}
	}
}

void wearable::equip(item& v) {
	for(auto i = MeleeWeapon; i <= Elbows; i = (wear_s)(i + 1)) {
		if(wears[i])
			continue;
		if(!v.canequip(i))
			continue;
		wears[i] = v;
		v.clear();
		break;
	}
	if(v)
		additem(v);
}

const item*	wearable::getwear(const void* data) const {
	if(data >= wears && data < wears + sizeof(wears) / sizeof(wears[0]))
		return (item*)data;
	return 0;
}

wear_s wearable::getwearslot(const item* data) const {
	if(!data)
		return Backpack;
	return (wear_s)(data - wears);
}

const char* wearable::getwearname(wear_s id) const {
	auto it = wears[id];
	if(it)
		return it.getfullname();
	if(id == MeleeWeapon)
		return getnm("Fist");
	return 0;
}