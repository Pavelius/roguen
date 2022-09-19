#include "areamap.h"
#include "crt.h"

static indext stack[256 * 256];
static indext* push_counter;
static indext* pop_counter;
static indext movement_rate[mps * mps];

static direction_s straight_directions[] = {
	North, South, West, East
};
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

void areamap::set(indext i, tile_s v) {
	if(i == Blocked)
		return;
	tiles[i] = v;
}

void areamap::set(rect rc, tile_s v) {
	for(short y = rc.y1; y <= rc.y2; y++) {
		if(y < 0)
			continue;
		if(y >= mps)
			break;
		for(short x = rc.x1; x <= rc.x2; x++) {
			if(x < 0)
				continue;
			if(x >= mps)
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
	if(bsdata<tilei>::elements[tiles[i]].iswall())
		return false;
	auto f = features[i];
	if(f) {
		auto& ei = bsdata<featurei>::elements[f];
		if(ei.is(Impassable))
			return false;
		if(ei.is(ImpassableNonActive) && !is(i, Activated))
			return false;
	}
	return true;
}

bool areamap::isfreelt(indext i) const {
	return isfree(i);
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

int areamap::getindex(indext i, tile_s tile) const {
	auto m = 0;
	auto f = 1;
	for(auto d : straight_directions) {
		auto i1 = to(i, d);
		auto t1 = tiles[i1];
		if((i1 != Blocked) && ((t1 == tile) || bsdata<tilei>::elements[t1].tile == tile))
			m |= f;
		f = f << 1;
	}
	return m;
}

unsigned char areamap::getfow(indext i) const {
	unsigned char m = 0;
	unsigned char f = 1;
	for(auto d : straight_directions) {
		auto i1 = to(i, d);
		if(i1 != Blocked && !is(i1, Explored))
			m |= f;
		f = f << 1;
	}
	return m;
}

bool areamap::iswall(indext i, direction_s d) const {
	auto i1 = to(i, d);
	if(i1 == Blocked)
		return false;
	if(!is(i1, Explored))
		return true;
	return bsdata<tilei>::elements[tiles[i1]].iswall();
}

bool areamap::linelossv(int x0, int y0, int x1, int y1) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < mps && y0 >= 0 && y0 < mps) {
			auto i = m2i(x0, y0);
			set(i, Visible);
			set(i, Explored);
			if(!isfreelt(i))
				return false;
		}
		if(x0 == x1 && y0 == y1)
			return true;
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

bool areamap::linelos(int x0, int y0, int x1, int y1) const {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < mps && y0 >= 0 && y0 < mps) {
			auto i = m2i(x0, y0);
			if(!isfreelt(i))
				return false;
		}
		if(x0 == x1 && y0 == y1)
			return true;
		e2 = err;
		if(e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void areamap::setlos(indext index, int r) {
	auto pt = i2m(index);
	for(auto x = pt.x - r; x <= pt.x + r; x++) {
		linelossv(pt.x, pt.y, x, pt.y - r);
		linelossv(pt.x, pt.y, x, pt.y + r);
	}
	for(auto y = pt.y - r; y <= pt.y + r; y++) {
		linelossv(pt.x, pt.y, pt.x - r, y);
		linelossv(pt.x, pt.y, pt.x + r, y);
	}
}