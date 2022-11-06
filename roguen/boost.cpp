#include "main.h"

void boosti::updateall() {
	auto ps = bsdata<boosti>::elements;
	for(auto& e : bsdata<boosti>()) {
		if(e.stamp < game.getminutes())
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

boosti* boosti::find(variant parent, spell_s effect) {
	for(auto& e : bsdata<boosti>()) {
		if(e.effect == effect && e.parent == parent)
			return &e;
	}
	return 0;
}

boosti* boosti::add(variant parent, spell_s effect) {
	auto p = find(parent, effect);
	if(p)
		return p;
	p = bsdata<boosti>::add();
	p->clear();
	p->parent = parent;
	p->effect = effect;
	return p;
}