#include "areamap.h"
#include "crt.h"

static indext stack[256 * 256];
static indext* push_counter;
static indext* pop_counter;
static indext movement_rate[mps * mps];

static const direction_s orientations_7b7[49] = {
	NorthWest, NorthWest, North, North, North, NorthEast, NorthEast,
	NorthWest, NorthWest, NorthWest, North, NorthEast, NorthEast, NorthEast,
	West, West, NorthWest, North, NorthEast, East, East,
	West, West, West, North, East, East, East,
	West, West, SouthWest, South, SouthEast, East, East,
	SouthWest, SouthWest, SouthWest, South, SouthEast, SouthEast, SouthEast,
	SouthWest, SouthWest, South, South, South, SouthEast, SouthEast,
};

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

void areamap::clearpath() {
	for(auto i = 0; i < mps * mps; i++)
		movement_rate[i] = NotCalculatedMovement;
	push_counter = stack;
	pop_counter = stack;
}

indext areamap::getnext(indext start, indext goal) {
	auto n = getpath(start, goal, stack, sizeof(stack) / sizeof(stack[0]));
	if(!n)
		return Blocked;
	return stack[n - 1];
}

unsigned areamap::getpath(indext start, indext goal, indext* result, unsigned maximum) {
	auto pb = result;
	auto pe = result + maximum;
	auto curr = goal;
	auto cost = Blocked;
	if(pb == pe || curr == start)
		return 0;
	*pb++ = goal;
	while(pb < pe) {
		auto next = curr;
		for(auto i = North; i <= NorthWest; i = (direction_s)(i + 1)) {
			auto i1 = to(curr, i);
			if(i1 == Blocked)
				continue;
			if(i1 == start) {
				next = i1;
				break;
			}
			auto c1 = movement_rate[i1];
			if(c1 >= cost)
				continue;
			next = i1;
			cost = c1;
		}
		if(next == curr || next == start)
			break;
		*pb++ = next;
		curr = next;
	}
	return pb - result;
}

void areamap::blockzero() {
	for(indext i = 0; i < mps * mps; i++) {
		if(movement_rate[i] >= NotCalculatedMovement)
			movement_rate[i] = Blocked;
	}
}

void areamap::blockwalls() const {
}

void areamap::blockfeatures() const {
	for(indext i = 0; i < mps * mps; i++) {
		if(!features[i])
			continue;
		auto& ei = bsdata<featurei>::elements[features[i]];
		if(ei.is(Impassable))
			movement_rate[i] = Blocked;
	}
}

void areamap::addwave(indext i) {
	*push_counter++ = i;
	if(push_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		push_counter = stack;
}

indext areamap::getwave() {
	auto index = *pop_counter++;
	if(pop_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		pop_counter = stack;
	return index;
}

void areamap::makewavex() {
	while(pop_counter != push_counter) {
		auto index = getwave();
		auto cost = movement_rate[index] + 1;
		for(auto d = North; d <= NorthWest; d = (direction_s)(d + 1)) {
			auto i1 = to(index, d);
			if(i1 == Blocked)
				continue;
			auto c1 = movement_rate[i1];
			if(c1 == Blocked || c1 <= cost)
				continue;
			movement_rate[i1] = cost;
			addwave(i1);
		}
	}
	blockzero();
}

void areamap::makewave(indext start_index) {
	movement_rate[start_index] = 0;
	addwave(start_index);
	makewavex();
}

void areamap::blockrange(int range) {
	for(indext i = 0; i < mps * mps; i++) {
		auto v = movement_rate[i];
		if(v == Blocked)
			continue;
		if(v > range)
			movement_rate[i] = Blocked;
	}
}

int	areamap::getrange(indext i1, indext i2) {
	if(i1 == Blocked || i2 == Blocked)
		return Blocked;
	auto p1 = i2m(i1);
	auto p2 = i2m(i2);
	auto dx = iabs(p1.x - p2.x);
	auto dy = iabs(p1.y - p2.y);
	return (dx > dy) ? dx : dy;
}

direction_s	areamap::getdirection(point s, point d) {
	const int osize = 7;
	int dx = d.x - s.x;
	int dy = d.y - s.y;
	int st = (2 * imax(iabs(dx), iabs(dy)) + osize - 1) / osize;
	if(!st)
		return North;
	int ax = dx / st;
	int ay = dy / st;
	return orientations_7b7[(ay + (osize / 2)) * osize + ax + (osize / 2)];
}