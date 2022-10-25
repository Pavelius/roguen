#include "crt.h"
#include "direction.h"
#include "pushvalue.h"
#include "randomizer.h"
#include "world.h"

struct sitei;
static point stack[256 * 256];
static unsigned short push_stack, pop_stack;

static slice<point> random_directions() {
	static point source[]= {{0, -1}, {0, 1}, {1, 0}, {1, 0}};
	return source;
}

static int d100() {
	return rand() % 100;
}

static void pushv(worldi& a, point i, unsigned char v) {
	a[i] = v;
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
	pushv(*this, start, v);
	while(pop_stack < push_stack) {
		auto i = popv();
		auto t = (*this)[i];
		for(auto d : random_directions()) {
			auto n = i + d;
			if(!isvalid(n) || (*this)[n])
				continue;
			if(d100() < 70)
				pushv(*this, n, t);
			else {
				auto ntv = single("RandomOvelandTiles");
				if(ntv.iskind<sitei>())
					pushv(*this, n, (unsigned char)ntv.value);
			}
		}
	}
}



