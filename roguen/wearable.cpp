#include "wearable.h"
#include "math.h"

wear_s last_player_used_wear = IncorrectWearSlot;

void wearable::addcoins(int v) {
	if(!v)
		return;
	//if(answers::console)
	//	answers::console->addn(getnm("YouTakeCoins"), v);
	money += v;
	if(money < 0)
		money = 0;
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
				last_item = wears + i;
				v.clear();
				break;
			}
		}
	}
}

void wearable::equip(item& v) {
	for(auto i = MeleeWeapon; i <= Legs; i = (wear_s)(i + 1)) {
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

void wearable::equip(const itemi* pi, int count) {
	item it;
	it.create(pi, count);
	it.createpower(0);
	equip(it);
}

const item*	wearable::getwear(const void* data) const {
	if(data >= wears && data < wears + sizeof(wears) / sizeof(wears[0]))
		return (item*)data;
	return 0;
}

wear_s wearable::getwearslot(const item* data) const {
	if(!data)
		return IncorrectWearSlot;
	return imin(IncorrectWearSlot, (wear_s)(data - wears));
}

bool wearable::useitem(const itemi* pi, bool run) {
	if(!pi)
		return false;
	for(auto& v : backpack()) {
		if(!v.is(pi))
			continue;
		if(run)
			v.use();
		return true;
	}
	return false;
}