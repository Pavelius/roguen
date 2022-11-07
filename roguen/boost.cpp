#include "boost.h"

BSDATAC(boosti, 256);

void boosti::updateall(unsigned current_stamp) {
	auto ps = bsdata<boosti>::elements;
	for(auto& e : bsdata<boosti>()) {
		if(e.stamp < current_stamp)
			continue;
		*ps++= e;
	}
	bsdata<boosti>::source.count = ps - bsdata<boosti>::begin();
}

void boosti::remove(variant parent) {
	for(auto& e : bsdata<boosti>()) {
		if(e.parent==parent)
			e.clear();
	}
}

boosti* boosti::find(variant parent, variant effect) {
	for(auto& e : bsdata<boosti>()) {
		if(e.effect == effect && e.parent == parent)
			return &e;
	}
	return 0;
}

boosti* boosti::add(variant parent, variant effect, unsigned stop_time) {
	auto p = find(parent, effect);
	if(!p) {
		p = bsdata<boosti>::add();
		p->clear();
		p->parent = parent;
		p->effect = effect;
	}
	if(p->stamp < stop_time)
		p->stamp = stop_time;
	return p;
}