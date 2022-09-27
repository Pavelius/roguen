#include "crt.h"
#include "direction.h"
#include "generator.h"
#include "pushvalue.h"
#include "world.h"

struct sitei;
static pointm stack[256 * 256];
static unsigned short push_stack, pop_stack;
static direction_s all_around[] = {North, South, East, West, NorthEast, NorthWest, SouthEast, SouthWest};

static int d100() {
	return rand() % 100;
}

static void pushv(worldi* p, pointm i, unsigned char v) {
	if(!p->get(i)) {
		p->set(i, v);
		stack[push_stack++] = v;
	}
}

static pointm popv() {
	return stack[pop_stack++];
}

void worldi::clear() {
	memset(this, 0, sizeof(*this));
}

void worldi::generate(pointm start, mpt v) {
	pushvalue push(pointm::mps);
	pointm::mps = mps;
	if(!start)
		return;
	pushv(this, start, v);
	while(pop_stack < push_stack) {
		auto i = popv();
		auto t = get(i);
		for(auto d : all_around) {
			auto n = i.to(d);
			if(!n)
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



