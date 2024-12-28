#include "monster.h"
#include "item.h"
#include "log.h"

monsteri* monsteri::ally() const {
	if(minions)
		return minions->random();
	return 0;
}

bool monsteri::islower(ability_s v, int value) const {
	for(auto v : use) {
		if(v.iskind<abilityi>() && v.counter <= value)
			return true;
	}
	return false;
}

bool monsteri::have(variant v) const {
	for(auto e : use) {
		if(e == v)
			return true;
	}
	return false;
}

bool is_boss(const void* p) {
	return ((monsteri*)p)->minions != 0;
}

void check_monsters() {
	for(auto& e : bsdata<monsteri>()) {
		for(auto& v : e.use) {
			if(v.iskind<itemi>()) {
				auto pi = bsdata<itemi>::elements + v.value;
				for(auto i = Strenght; i <= Wits; i = (ability_s)(i + 1)) {
					auto need = pi->required[i - Strenght];
					if(e.abilities[i] < need) {
						log::errorp(0, " `%1` can't use item `%2` because of requipment %3 %4i (have only %5i), but have it in tag `use`.",
							e.id, pi->id, bsdata<abilityi>::elements[i].id, need, e.abilities[i]);
						break;
					}
				}
			}
		}
	}
}