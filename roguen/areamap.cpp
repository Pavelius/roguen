#include "areamap.h"
#include "crt.h"

static int d100() {
	return rand() % 100;
}

void areamap::clear() {
	memset(tiles, 0, sizeof(tiles));
	memset(flags, 0, sizeof(flags));
	for(auto& e : random)
		e = (unsigned char)(rand() % 256);
}

direction_s round(direction_s d, direction_s v) {
	switch(v) {
	case East:
		switch(d) {
		case North: return NorthEast;
		case NorthEast: return East;
		case East: return SouthEast;
		case SouthEast: return South;
		case South: return SouthWest;
		case SouthWest: return West;
		case West: return NorthWest;
		case NorthWest: return North;
		default: return d;
		}
		break;
	case West:
		switch(d) {
		case North: return NorthWest;
		case NorthWest: return West;
		case West: return SouthWest;
		case SouthWest: return South;
		case South: return SouthEast;
		case SouthEast: return East;
		case East: return NorthEast;
		case NorthEast: return North;
		default: return d;
		}
		break;
	default:
		return d;
	}
}

indext to(indext i, direction_s v) {
	if(i == Blocked)
		return Blocked;
	auto m = i2m(i);
	switch(v) {
	case North:
		if(m.y <= 0)
			return Blocked;
		return i - mps;
	case NorthWest:
		if(m.y <= 0 || m.x <= 0)
			return Blocked;
		return i - mps - 1;
	case NorthEast:
		if(m.y <= 0 || m.x >= mps - 1)
			return Blocked;
		return i - mps + 1;
	case South:
		if(m.y >= mps - 1)
			return Blocked;
		return i + mps;
	case SouthWest:
		if(m.y >= mps - 1 || m.x <= 0)
			return Blocked;
		return i + mps - 1;
	case SouthEast:
		if(m.y >= mps - 1 || m.x >= mps - 1)
			return Blocked;
		return i + mps + 1;
	case West:
		if(m.x <= 0)
			return Blocked;
		return i - 1;
	case East:
		if(m.x >= mps - 1)
			return Blocked;
		return i + 1;
	default:
		return i;
	}
}

point areamap::correct(point v) {
	if(v.x < 0)
		v.x = 0;
	if(v.y < 0)
		v.y = 0;
	if(v.x >= mps)
		v.x = mps - 1;
	if(v.y >= mps)
		v.y = mps - 1;
	return v;
}

void areamap::set(indext i, tile_s v) {
	if(i == Blocked)
		return;
	tiles[i] = v;
}

void areamap::set(indext i, tile_s v, short w, short h) {
	if(i == Blocked)
		return;
	auto p1 = i2m(i);
	auto p2 = p1 + point{w, h};
	for(auto y = p1.y; y < p2.y; y++) {
		if(y < 0)
			continue;
		if(y >= mps - 1)
			break;
		for(auto x = p1.x; x < p2.x; x++) {
			if(x < 0)
				continue;
			if(x >= mps - 1)
				break;
			set(m2i({x, y}), v);
		}
	}
}

void areamap::removechance(mapf_s v, int chance) {
	for(auto i = 0; i < mps * mps; i++) {
		if(!is(i, v))
			continue;
		if(d100() >= chance)
			continue;
		remove(i, v);
	}
}

bool areamap::isfree(indext i) const {
	if(i == Blocked)
		return false;
	if(features[i] && bsdata<featurei>::elements[features[i]].is(Impassable))
		return false;
	return true;
}