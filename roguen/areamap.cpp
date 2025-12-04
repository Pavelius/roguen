#include "areamap.h"
#include "direction.h"
#include "math.h"
#include "rand.h"

const short unsigned Blocked = 0xFFFF;
const short unsigned NotCalculatedMovement = 0xFFF0;

point last_index;
unsigned char last_map_object;

static point stack[256 * 256];
static point* push_counter;
static point* pop_counter;
static anymap<short unsigned, areamap::mps> movement_rate;
static point straight_directions[] = {
	{0, -1}, {0, 1}, {-1, 0}, {1, 0},
};
static const directionn orientations_7b7[49] = {
	NorthWest, NorthWest, North, North, North, NorthEast, NorthEast,
	NorthWest, NorthWest, NorthWest, North, NorthEast, NorthEast, NorthEast,
	West, West, NorthWest, North, NorthEast, East, East,
	West, West, West, North, East, East, East,
	West, West, SouthWest, South, SouthEast, East, East,
	SouthWest, SouthWest, SouthWest, South, SouthEast, SouthEast, SouthEast,
	SouthWest, SouthWest, South, South, South, SouthEast, SouthEast,
};

static void addwave(point v) {
	*push_counter++ = v;
	if(push_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		push_counter = stack;
}

static point getwave() {
	auto index = *pop_counter++;
	if(pop_counter >= stack + sizeof(stack) / sizeof(stack[0]))
		pop_counter = stack;
	return index;
}

point randomr(const rect& rc) {
	short x = rc.x1 + rand() % (rc.width() + 1);
	short y = rc.y1 + rand() % (rc.height() + 1);
	return {x, y};
}

static int randomcount(const rect& rc, int v) {
	if(v <= -100)
		return 0;
	if(v < 0)
		v = (rc.width() + 1) * (rc.height() + 1) * (-v) / 100;
	if(v == 0)
		v = 1;
	return v;
}

void areamap::blockfeatures() const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(!int(features[m]))
				continue;
			auto& ei = bsdata<featurei>::elements[int(features[m])];
			if(ei.is(Impassable))
				movement_rate[m] = Blocked;
		}
}

void areamap::blocktiles(int v) const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(tiles[m] == (unsigned char)v)
				movement_rate[m] = Blocked;
		}
}

void areamap::blockwalls() const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(iswall(m))
				movement_rate[m] = Blocked;
		}
}

void areamap::blockzero() {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(movement_rate[m] >= NotCalculatedMovement)
				movement_rate[m] = Blocked;
		}
}

void areamap::change(int t1, int t2) {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(tiles[m] == t1)
				tiles[m] = t2;
		}
}

void areamap::clear() {
	memset(this, 0, sizeof(*this));
	for(auto& e : param)
		e = (unsigned char)(rand() % 256);
}

void areamap::clearpath() {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++)
			movement_rate[m] = NotCalculatedMovement;
	push_counter = stack;
	pop_counter = stack;
}

point areamap::get(const rect& rc) {
	return (rc.x1 <= rc.x2 && rc.y1 <= rc.y2) ? randomr(rc) : point{-1000, -1000};
}

point areamap::getnext(point start, point goal) {
	auto n = getpath(start, goal, stack, sizeof(stack) / sizeof(stack[0]));
	if(!n)
		return {-1000, -1000};
	return stack[n - 1];
}

point areamap::getnextgreater(point start) {
	auto cost = 0;
	point next = {-1000, -1000};
	for(auto d : all_directions) {
		auto i1 = start + d;
		if(!isvalid(i1))
			continue;
		auto c1 = movement_rate[i1];
		if(c1 >= NotCalculatedMovement || c1 <= cost)
			continue;
		next = i1;
		cost = c1;
	}
	return next;
}

unsigned areamap::getpath(point start, point goal, point* result, unsigned maximum) {
	auto pb = result;
	auto pe = result + maximum;
	auto curr = goal;
	auto cost = 0xFFFF;
	if(pb == pe || curr == start)
		return 0;
	*pb++ = goal;
	while(pb < pe) {
		auto next = curr;
		for(auto d : all_directions) {
			auto i1 = curr + d;
			if(!isvalid(i1))
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

bool areamap::isfree(point m) const {
	if(!isvalid(m))
		return false;
	if(iswall(m))
		return false;
	auto pe = bsdata<featurei>::elements + (int)features[m];
	if(pe->isvisible()) {
		if(pe->is(Impassable))
			return false;
	}
	return true;
}

void areamap::set(rect rc, fnset proc, int v) {
	point m;
	for(m.y = rc.y1; m.y <= rc.y2; m.y++)
		for(m.x = rc.x1; m.x <= rc.x2; m.x++)
			(this->*proc)(m, v);
}

void areamap::set(rect rc, fnset proc, int v, int random_count) {
	if(random_count <= -100) {
		set(rc, proc, v);
		return;
	}
	random_count = randomcount(rc, random_count);
	while(random_count > 0) {
		(this->*proc)(randomr(rc), v);
		random_count--;
	}
}

void areamap::settile(point m, int v) {
	if(!isvalid(m))
		return;
	tiles[m] = (unsigned char)v;
	features[m] = 0;
}

void areamap::setblock(point m, unsigned short v) {
	if(isvalid(m))
		movement_rate[m] = v;
}

void areamap::makewavex() {
	while(pop_counter != push_counter) {
		auto m = getwave();
		auto cost = movement_rate[m] + 1;
		for(auto d : all_directions) {
			auto m1 = m + d;
			if(!isvalid(m1))
				continue;
			auto c1 = movement_rate[m1];
			if(c1 == Blocked || c1 <= cost)
				continue;
			movement_rate[m1] = cost;
			addwave(m1);
		}
	}
	blockzero();
}

void areamap::makewave(point start_index) {
	movement_rate[start_index] = 0;
	addwave(start_index);
	makewavex();
}

void areamap::blockrange(int range) {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			auto v = movement_rate[m];
			if(v == Blocked)
				continue;
			if(v > range)
				movement_rate[m] = Blocked;
		}
}

int	areamap::getrange(point m1, point m2) {
	if(!isvalid(m1) || !isvalid(m2))
		return Blocked;
	auto dx = iabs(m1.x - m2.x);
	auto dy = iabs(m1.y - m2.y);
	return (dx > dy) ? dx : dy;
}

directionn	areamap::getdirection(point s, point d) {
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

int areamap::getindex(point m, int tile) const {
	auto r = 0;
	auto f = 1;
	for(auto d : straight_directions) {
		auto m1 = m + d;
		auto t1 = tiles[m1];
		if(isvalid(m1) && ((t1 == tile) || bsdata<tilei>::elements[t1].tile == tile))
			r |= f;
		f = f << 1;
	}
	return r;
}

bool areamap::iswall(point m, directionn d) const {
	auto m1 = to(m, d);
	if(!isvalid(m1))
		return false;
	return bsdata<tilei>::elements[tiles[m1]].iswall();
}

bool areamap::linelos(int x0, int y0, int x1, int y1, fitest test) const {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < mps && y0 >= 0 && y0 < mps) {
			point m = {(short)x0, (short)y0};
			if(!test(m))
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

void areamap::setlos(point m, int r, fitest test) {
	for(auto x = m.x - r; x <= m.x + r; x++) {
		linelos(m.x, m.y, x, m.y - r, test);
		linelos(m.x, m.y, x, m.y + r, test);
	}
	for(auto y = m.y - r; y <= m.y + r; y++) {
		linelos(m.x, m.y, m.x - r, y, test);
		linelos(m.x, m.y, m.x + r, y, test);
	}
}

point areamap::getpoint(const rect& rc, directionn dir) {
	switch(dir) {
	case West: return {short(rc.x1), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case East: return {short(rc.x2), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case North: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rc.y1)};
	default: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rc.y2)};
	}
}

point areamap::getpoint(const rect& rco, const rect& bound, directionn dir) {
	rect rc = rco;
	if(rc.x1 < bound.x1)
		rc.x1 = bound.x1;
	if(rc.y1 < bound.y1)
		rc.y1 = bound.y1;
	if(rc.x2 > bound.x2)
		rc.x2 = bound.x2;
	if(rc.y2 > bound.y2)
		rc.y2 = bound.y2;
	if(rc.width() < 3 || rc.height() < 3)
		return point{(short)rc.x1, (short)rc.y1};
	switch(dir) {
	case West: return {short(rco.x1), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case East: return {short(rco.x2), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case North: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rco.y1)};
	default: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rco.y2)};
	}
}

void areamap::horz(int x1, int y1, int x2, fnset proc, int v) {
	while(x1 <= x2) {
		if(x1 >= 0 && x1 < mps)
			(this->*proc)(point{short(x1), short(y1)}, v);
		x1++;
	}
}

void areamap::vert(int x1, int y1, int y2, fnset proc, int v) {
	while(y1 <= y2) {
		if(y1 >= 0 && y1 < mps)
			(this->*proc)(point{short(x1), short(y1)}, v);
		y1++;
	}
}

point areamap::getfree(point m, short maximum, fitest test) {
	if(!test)
		return {-1000, -1000};
	if(test(m))
		return m;
	point n;
	for(short r = 1; r < maximum; r++) {
		for(short i = -r + 1; i < r; i++) {
			n = m.to(i, -r);
			if(test(n))
				return n;
			n = m.to(i, r);
			if(test(n))
				return n;
		}
		for(short i = m.y - r; i <= m.y + r; i++) {
			n = m.to(r, i);
			if(test(n))
				return n;
			n = m.to(-r, i);
			if(test(n))
				return n;
		}
	}
	return {-1000, -1000};
}

point areamap::bordered(directionn d) {
	switch(d) {
	case North: return {mps / 2, 0};
	case South: return {mps / 2, mps - 1};
	case West: return {0, mps / 2};
	case East: return {mps - 1, mps / 2};
	default: return {mps / 2, mps / 2};
	}
}

point areamap::findfeature(unsigned char v) const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(features[m] == v)
				return m;
		}
	return {-1000, -1000};
}

directionn areamap::getmost(const rect& rc) const {
	auto c = mps / 2;
	auto n = center(rc);
	auto dx = iabs(n.x - c);
	auto dy = iabs(n.y - c);
	if(dx >= dy)
		return (n.x < c) ? East : West;
	else
		return (n.y < c) ? South : North;
}

rect areamap::get(const rect& rca, point offset, point minimum, point maximum) {
	if(rca.x1 > rca.x2 || rca.y1 > rca.y2)
		return {-1000, -1000};
	auto w = rca.width() - offset.x * 2;
	if(w < maximum.x)
		maximum.x = w;
	auto h = rca.height() - offset.y * 2;
	if(h < maximum.y)
		maximum.y = h;
	rect rc;
	rc.x1 = rca.x1 + offset.x + xrand(0, w);
	rc.x2 = rc.x1 + xrand(minimum.x, maximum.x);
	rc.y1 = rca.y1 + offset.y + xrand(0, h);
	rc.y2 = rc.y1 + xrand(minimum.y, maximum.y);
	return rc;
}

void areamap::setreveal(point m, int floor) {
	auto& ei = getfeature(m);
	auto pa = ei.getactivate();
	if(ei.isvisible() || !pa)
		return;
	settile(m, floor);
	setfeature(m, (unsigned char)bsid(pa));
}

void areamap::setactivate(point m) {
	auto& ei = getfeature(m);
	auto pa = ei.getactivate();
	if(!pa)
		return;
	setfeature(m, (unsigned char)bsid(pa));
}