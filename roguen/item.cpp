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