#include "main.h"

creature* player;

creature* creature::create(indext index, const monsteri* pv) {
	auto p = bsdata<creature>::add();
	p->kind = pv;
	p->gender = pv->gender;
	p->index = index;
	p->finish();
	return p;
}

void creature::movestep(direction_s i) {
	direction = i;
	switch(i) {
	case West: case NorthWest: case SouthWest: mirror = true; break;
	case East: case NorthEast: case SouthEast: mirror = false; break;
	}
	movestep(to(index, i));
}

void creature::movestep(indext ni) {
	if(ni == Blocked)
		return;
	index = ni;
	fixmovement();
}

void creature::finish() {
	hp = 10;
	fixappear();
}

void creature::aimove() {
	static direction_s allaround[] = {North, South, East, West};
	movestep(maprnd(allaround));
}
