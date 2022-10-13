#include "main.h"

void item::setcount(int v) {
	if(v <= 0)
		clear();
	else if(iscountable())
		count = v - 1;
}

int item::getcount() const {
	if(!type)
		return 0;
	return iscountable() ? count + 1 : 1;
}

void item::add(item& v) {
	if(type != v.type)
		return;
	if(iscountable()) {
		unsigned n1 = count + v.count + 1;
		if(n1 >= 0xFFFF) {
			count = 0xFFFF;
			v.count = n1 - count - 1;
		} else {
			count = n1;
			v.clear();
		}
	}
}

bool item::canequip(wear_s v) const {
	auto w = geti().wear;
	switch(w) {
	case FingerRight: case FingerLeft:
		return v == FingerLeft || v == FingerRight;
	default:
		return v == w;
	}
}

void item::create(const itemi* pi, int count) {
	if(!pi)
		return;
	clear();
	type = pi - bsdata<itemi>::elements;
	if(pi->count)
		setcount(count * pi->count);
	else
		setcount(count);
}

int item::getdamage() const {
	switch(geti().wear) {
	case MeleeWeapon:
	case MeleeWeaponOffhand:
	case RangedWeapon:
		return geti().weapon.damage;
	default:
		return 0;
	}
}

bool item::is(const itemi& v) const {
	return geti() == v;
}

bool item::is(wear_s v) const {
	auto wear = geti().wear;
	switch(v) {
	case FingerLeft: case FingerRight:
		return (wear == FingerRight);
	default:
		return wear == v;
	}
}

creature* item::getowner() const {
	auto i = bsdata<creature>::source.indexof(this);
	if(i == -1)
		return 0;
	return bsdata<creature>::elements + i;
}

void item::drop(point m) {
	if(!area.isvalid(m))
		return;
	auto pi = bsdata<itemground>::add();
	memcpy(static_cast<item*>(pi), static_cast<item*>(this), sizeof(item));
	pi->position = m;
	clear();
}