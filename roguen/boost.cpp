#include "boost.h"

BSDATAC(boosti, 256);

void update_all_boost(unsigned current_stamp) {
	auto ps = bsdata<boosti>::elements;
	for(auto& e : bsdata<boosti>()) {
		if(e.stamp < current_stamp)
			continue;
		*ps++= e;
	}
	bsdata<boosti>::source.count = ps - bsdata<boosti>::begin();
}

void remove_boost(variant parent) {
	for(auto& e : bsdata<boosti>()) {
		if(e.parent==parent)
			e.clear();
	}
}

static boosti* find_boost(variant parent, variant effect) {
	for(auto& e : bsdata<boosti>()) {
		if(e.effect == effect && e.parent == parent)
			return &e;
	}
	return 0;
}

void add_boost(variant parent, variant effect, unsigned stop_time) {
	auto p = find_boost(parent, effect);
	if(!p) {
		p = bsdata<boosti>::add();
		p->clear();
		p->parent = parent;
		p->effect = effect;
	}
	if(p->stamp < stop_time)
		p->stamp = stop_time;
}