#include "main.h"

void movable::setposition(point v) {
	position = v;
}

void movable::setdirection(direction_s v) {
	direction = v;
	switch(v) {
	case West: case NorthWest: case SouthWest: mirror = true; break;
	case East: case NorthEast: case SouthEast: mirror = false; break;
	}
}

void movable::fixvalue(int v) const {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1i", v);
	fixvalue(temp, (v > 0) ? 2 : 1);
}

void movable::fixability(ability_s i, int v) const {
	if(!answers::interactive)
		return;
	if(!area.is(position, Visible))
		return;
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1%+2i", bsdata<abilityi>::elements[i].getname(), v);
	fixvalue(temp, (v > 0) ? 2 : 1);
}