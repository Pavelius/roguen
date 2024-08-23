#include "creature.h"
#include "answers.h"
#include "charname.h"
#include "monster.h"
#include "pushvalue.h"
#include "textscript.h"

bool actable::iskind(variant v) const {
	if(v.iskind<monsteri>()) {
		if(!kind.iskind<monsteri>())
			return false;
		auto pn = bsdata<monsteri>::elements + v.value;
		auto pt = bsdata<monsteri>::elements + kind.value;
		return (pn == pt) || (pt->parent == pn);
	}
	return false;
}

bool actable::ischaracter() const {
	return !kind.iskind<monsteri>();
}

const char* actable::getname() const {
	if(name_id != 0xFFFF)
		return charname::getname(name_id);
	return kind.getname();
}

struct monsteri* actable::getmonster() const {
	return kind;
}