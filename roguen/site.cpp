#include "areaf.h"
#include "areapiece.h"
#include "creature.h"
#include "game.h"
#include "site.h"
#include "speech.h"

rooma rooms;
roomi* last_room;

void areaheadi::clear() {
	memset(this, 0, sizeof(*this));
}

variants sitei::getloot() const {
	return loot;
}

roomi* add_room() {
	return area->rooms.add();
}

roomi* find_room(point pt) {
	for(auto& e : area->rooms) {
		if(pt.in(e.rc))
			return &e;
	}
	return 0;
}

bool roomi::isexplored() const {
	return area->is(center(), Explored);
}

bool roomi::ismarkable() const {
	return is(Notable) && isexplored();
}

int	roomi::getseed() const {
	return rc.x1 * 1 + rc.y1 * 2 + rc.x2 * 3 + rc.y2 * 4;
}

point roomi::getitems() const {
	return {PlacementRoom, (short)area->rooms.indexof(this)};
}

const char* roomi::getname() const {
	auto id = geti().id;
	auto push_seed = speech_random;
	speech_random = getseed();
	auto p = speech_get(id);
	if(p) {
		static char temp[128];
		auto pc = getowner();
		auto owner_name = "None";
		if(pc)
			owner_name = pc->getname();
		stringbuilder sb(temp);
		sb.clear();
		sb.add(p, owner_name);
		return temp;
	}
	speech_random = push_seed;
	return getnm(id);
}
