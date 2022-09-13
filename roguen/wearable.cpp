#include "main.h"

void wearable::additem(item& v) {
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
}

bool wearable::isitem(const void* pv) const {
	return pv >= wears && pv < wears + sizeof(wears) / sizeof(wears[0]);
}

const char* wearable::getwearname(wear_s id) const {
	auto it = wears[id];
	if(it)
		return it.getname();
	if(id == MeleeWeapon)
		return getnm("Fist");
	return 0;
}