#include "areamap.h"
#include "direction.h"
#include "crt.h"

const short unsigned Blocked = 0xFFFF;
const short unsigned NotCalculatedMovement = 0xFFF0;

static point stack[256 * 256];
static point* push_counter;
static point* pop_counter;
static anymap<short unsigned, areamap::mps> movement_rate;
static point straight_directions[] = {
	{0, -1}, {0, 1}, {-1, 0}, {1, 0},
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
	memset(this, 0, sizeof(*this));
	for(auto& e : random)
		e = (unsigned char)(rand() % 256);
}

void areamap::set(point m, tile_s v) {
	if(!isvalid(m))
		return;
	(*this)[m] = v;
	features[m] = (feature_s)0;
}

void areamap::set(point m, feature_s v) {
	if(!isvalid(m))
		return;
	features[m] = v;
}

void areamap::set(rect rc, tile_s v) {
	point m;
	for(m.y = rc.y1; m.y <= rc.y2; m.y++)
		for(m.x = rc.x1; m.x <= rc.x2; m.x++)
			set(m, v);
}

void areamap::set(rect rc, mapf_s v) {
	point m;
	for(m.y = rc.y1; m.y <= rc.y2; m.y++)
		for(m.x = rc.x1; m.x <= rc.x2; m.x++)
			set(m, v);
}

point randomr(const rect& rc) {
	short x = rc.x1 + rand() % (rc.width() + 1);
	short y = rc.y1 + rand() % (rc.height() + 1);
	return {x, y};
}

point areamap::get(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = xrand(rc.x1, rc.x2);
	short y = xrand(rc.y1, rc.y2);
	return {x, y};
}

void areamap::set(rect rc, mapf_s v, int random_count) {
	if(random_count <= -100) {
		set(rc, v);
		return;
	}
	random_count = randomcount(rc, random_count);
	while(random_count > 0) {
		set(randomr(rc), v);
		random_count--;
	}
}

void areamap::set(rect rc, tile_s v, int random_count) {
	if(random_count <= -100) {
		set(rc, v);
		return;
	}
	random_count = randomcount(rc, random_count);
	while(random_count > 0) {
		set(randomr(rc), v);
		random_count--;
	}
}

void areamap::set(rect rc, feature_s v, int random_count) {
	if(random_count <= -100) {
		set(rc, v);
		return;
	}
	random_count = randomcount(rc, random_count);
	while(random_count > 0) {
		set(randomr(rc), v);
		random_count--;
	}
}

void areamap::set(rect rc, feature_s v) {
	point m;
	for(m.y = rc.y1; m.y < rc.y2; m.y++)
		for(m.x = rc.x1; m.x < rc.x2; m.x++)
			features[m] = v;
}

int areamap::randomcount(const rect& rc, int v) {
	if(v <= -100)
		return 0;
	if(v < 0)
		v = (rc.width() + 1) * (rc.height() + 1) * (-v) / 100;
	if(v == 0)
		v = 1;
	return v;
}

void areamap::removechance(mapf_s v, int chance) {
	point m;
	for(m.y = 0; m.y < mps; m.y++) {
		for(m.x = 0; m.x < mps; m.x++) {
			if(!is(m, v))
				continue;
			if(d100() >= chance)
				continue;
			remove(m, v);
		}
	}
}

bool areamap::iswall(point m) const {
	return bsdata<tilei>::elements[(*this)[m]].iswall();
}

bool areamap::isfree(point m) const {
	if(!isvalid(m))
		return false;
	if(iswall(m))
		return false;
	auto f = features[m];
	if(f) {
		auto& ei = bsdata<featurei>::elements[f];
		if(ei.is(Impassable))
			return false;
		if(ei.is(ImpassableNonActive) && !is(m, Activated))
			return false;
	}
	return true;
}

void areamap::clearpath() {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++)
			movement_rate[m] = NotCalculatedMovement;
	push_counter = stack;
	pop_counter = stack;
}

point areamap::getnext(point start, point goal) {
	auto n = getpath(start, goal, stack, sizeof(stack) / sizeof(stack[0]));
	if(!n)
		return {-1000, -1000};
	return stack[n - 1];
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

void areamap::setblock(point m, unsigned short v) {
	if(m)
		movement_rate[m] = v;
}

void areamap::blockzero() {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(movement_rate[m] >= NotCalculatedMovement)
				movement_rate[m] = Blocked;
		}
}

void areamap::blocktiles(tile_s v) const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if((*this)[m] == v)
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

void areamap::blockfeatures() const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(!features[m])
				continue;
			auto& ei = bsdata<featurei>::elements[features[m]];
			if(ei.is(Impassable))
				movement_rate[m] = Blocked;
		}
}

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

int areamap::getindex(point m, tile_s tile) const {
	auto r = 0;
	auto f = 1;
	for(auto d : straight_directions) {
		auto m1 = m + d;
		auto t1 = (*this)[m1];
		if(isvalid(m1) && ((t1 == tile) || bsdata<tilei>::elements[t1].tile == tile))
			r |= f;
		f = f << 1;
	}
	return r;
}

unsigned char areamap::getlos(point m) const {
	unsigned char r = 0;
	unsigned char f = 1;
	for(auto d : straight_directions) {
		auto m1 = m + d;
		if(isvalid(m1) && !is(m1, Visible))
			r |= f;
		f = f << 1;
	}
	return r;
}

unsigned char areamap::getfow(point m) const {
	unsigned char r = 0;
	unsigned char f = 1;
	for(auto d : straight_directions) {
		auto m1 = m + d;
		if(isvalid(m1) && !is(m1, Explored))
			r |= f;
		f = f << 1;
	}
	return r;
}

bool areamap::iswall(point m, direction_s d) const {
	auto m1 = to(m, d);
	if(!isvalid(m1))
		return false;
	return bsdata<tilei>::elements[(*this)[m1]].iswall();
}

bool areamap::linelossv(int x0, int y0, int x1, int y1, fntest test) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	for(;;) {
		if(x0 >= 0 && x0 < mps && y0 >= 0 && y0 < mps) {
			point m = {(short)x0, (short)y0};
			set(m, Visible);
			set(m, Explored);
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

bool areamap::linelos(int x0, int y0, int x1, int y1, fntest test) const {
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

void areamap::setlos(point m, int r, fntest test) {
	for(auto x = m.x - r; x <= m.x + r; x++) {
		linelossv(m.x, m.y, x, m.y - r, test);
		linelossv(m.x, m.y, x, m.y + r, test);
	}
	for(auto y = m.y - r; y <= m.y + r; y++) {
		linelossv(m.x, m.y, m.x - r, y, test);
		linelossv(m.x, m.y, m.x + r, y, test);
	}
}

feature_s areamap::getfeature(point m) const {
	if(!isvalid(m))
		return (feature_s)0;
	return features[m];
}

void areamap::set(feature_s v, int bonus) {
	auto count = mps * mps;
	rect rc = {0, 0, mps - 1, mps - 1};
	if(bonus < 0)
		set(rc, v, xrand(-bonus / 2, -bonus));
	else
		set(rc, v, count * bonus / 100);
}

point areamap::getpoint(const rect& rc, direction_s dir) {
	switch(dir) {
	case West: return {short(rc.x1), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case East: return {short(rc.x2), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case North: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rc.y1)};
	default: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rc.y2)};
	}
}

point areamap::getpoint(const rect& rco, const rect& bound, direction_s dir) {
	rect rc = rco;
	if(rc.x1 < bound.x1)
		rc.x1 = bound.x1;
	if(rc.y1 < bound.y1)
		rc.y1 = bound.y1;
	if(rc.x2 > bound.x2)
		rc.x2 = bound.x2;
	if(rc.y2 > bound.y2)
		rc.y2 = bound.y2;
	switch(dir) {
	case West: return {short(rco.x1), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case East: return {short(rco.x2), short(xrand(rc.y1 + 1, rc.y2 - 1))};
	case North: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rco.y1)};
	default: return {short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rco.y2)};
	}
}

void areamap::horz(int x1, int y1, int x2, tile_s tile) {
	while(x1 <= x2) {
		if(x1 >= 0 && x1 < mps)
			set(point{short(x1), short(y1)}, tile);
		x1++;
	}
}

void areamap::vert(int x1, int y1, int y2, tile_s tile) {
	while(y1 <= y2) {
		if(y1 >= 0 && y1 < mps)
			set(point{short(x1), short(y1)}, tile);
		y1++;
	}
}

point areamap::getfree(point m, short maximum, fntest test) {
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

point areamap::bordered(direction_s d) {
	switch(d) {
	case North: return {mps / 2, 0};
	case South: return {mps / 2, mps - 1};
	case West: return {0, mps / 2};
	case East: return {mps - 1, mps / 2};
	default: return {mps / 2, mps / 2};
	}
}

void areamap::change(tile_s t1, tile_s t2) {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if((*this)[m] == t1)
				(*this)[m] = t2;
		}
}

point areamap::find(feature_s v) const {
	point m;
	for(m.y = 0; m.y < mps; m.y++)
		for(m.x = 0; m.x < mps; m.x++) {
			if(features[m] == v)
				return m;
		}
	return {-1000, -1000};
}

static point center(const rect& rc) {
	short x = rc.x1 + rc.width() / 2;
	short y = rc.y1 + rc.height() / 2;
	return {x, y};
}

direction_s areamap::getmost(const rect& rc) const {
	auto c = mps / 2;
	auto x = (iabs(rc.x1 - c) < iabs(rc.x2 - c)) ? rc.x1 : rc.x2;
	auto y = (iabs(rc.y1 - c) < iabs(rc.y2 - c)) ? rc.y1 : rc.y2;
	if(iabs(x - c) < iabs(y - c))
		return (x == rc.x1) ? West : East;
	else
		return (y == rc.y1) ? North : South;
}

rect areamap::get(const rect& rca, point offset, point minimum, point maximum) {
	if(rca.x1 > rca.x2 || rca.y1 > rca.y2)
		return {-1000, -1000};
	auto w = rca.width() - offset.x * 2;
	auto h = rca.height() - offset.y * 2;
	rect rc;
	rc.x1 = rca.x1 + offset.x + xrand(0, w - maximum.x);
	rc.x2 = rc.x1 + xrand(minimum.x, maximum.x);
	rc.y1 = rca.y1 + offset.y + xrand(0, h - maximum.y);
	rc.y2 = rc.y1 + xrand(minimum.y, maximum.y);
	return rc;
}