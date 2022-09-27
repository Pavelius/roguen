#include "main.h"

void boosti::updateall() {
	auto ps = bsdata<boosti>::elements;
	for(auto& e : bsdata<boosti>()) {
		if(e.stamp >= game.getminutes())
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