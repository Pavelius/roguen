#include "bsreq.h"
#include "randomizer.h"

BSMETA(randomizeri) = {
	BSREQ(id),
	BSREQ(chance),
	{}};
BSDATAC(randomizeri, 512)

static int getcounter(variant v) {
	if(v.counter < 1)
		return 1;
	return v.counter;
}

int randomizeri::total(const variants& elements) {
	auto result = 0;
	for(auto& e : elements)
		result += getcounter(e);
	return result;
}

variant randomizeri::random(const variants& elements) {
	auto summary = total(elements);
	if(summary) {
		auto result = rand() % summary;
		for(auto& e : elements) {
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