#include "bsreq.h"
#include "randomizer.h"

BSMETA(randomizeri) = {
	BSREQ(id),
	BSREQ(chance),
	{}};
BSDATAC(randomizeri, 128)

static int getcounter(variant v) {
	if(v.counter < 1)
		return 1;
	return v.counter;
}

static int total_chance(const variants& elements) {
	auto result = 0;
	for(auto& e : elements)
		result += getcounter(e);
	return result;
}

variant randomizeri::random() const {
	auto total = total_chance(chance);
	if(total) {
		auto result = rand() % total;
		for(auto& e : chance) {
			auto n = getcounter(e);
			if(result < n) {
				auto r = e;
				r.counter = 0;
				return r;
			}
			result -= n;
		}
	}
	return variant();
}

variant single(variant v) {
	while(v.iskind<randomizeri>())
		v = bsdata<randomizeri>::elements[v.value].random();
	return v;
}