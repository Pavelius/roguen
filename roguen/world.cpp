#include "crt.h"
#include "direction.h"
#include "generator.h"
#include "pushvalue.h"
#include "world.h"

struct sitei;
static point stack[256 * 256];
static unsigned short push_stack, pop_stack;

static slice<direction_s> random_directions() {
	static direction_s table[][8] = {
		{North, South, East, West},
		{South, East, West, North},
		{East, West, North, South},
		{West, North, South, East},
	};
	return maprnd(table);
}

static int d100() {
	return rand() % 100;
}

static void pushv(worldi* p, point i, unsigned char v) {
	p->set(i, v);
	stack[push_stack++] = i;
}

static point popv() {
	return stack[pop_stack++];
}

void worldi::clear() {
	memset(this, 0, sizeof(*this));
}

void worldi::generate(point start, unsigned char v) {
	if(!start)
		return;
	pushv(this, start, v);
	while(pop_stack < push_stack) {
		auto i = popv();
		auto t = get(i);
		for(auto d : random_directions()) {
			auto n = to(i, d, mps);
			if(!n || get(n))
				continue;
			if(d100() < 70)
				pushv(this, n, t);
			else {
				auto ntv = random_value("RandomOvelandTiles");
				if(ntv.iskind<sitei>())
					pushv(this, n, (unsigned char)ntv.value);
			}
		}
	}
}



