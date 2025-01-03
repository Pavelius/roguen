#include "bsreq.h"
#include "list.h"
#include "rand.h"
#include "randomizer.h"
#include "script.h"

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

int random_total(const variants& elements) {
	auto result = 0;
	for(auto& e : elements)
		result += getcounter(e);
	return result;
}

variant random_value(const variants& elements, int bonus, int summary) {
	if(summary) {
		auto result = rand() % summary + bonus;
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
	while(true) {
		if(v.iskind<randomizeri>()) {
			v = bsdata<randomizeri>::elements[v.value].random();
			continue;
		} else if(v.iskind<listi>()) {
			v = bsdata<listi>::elements[v.value].random();
			continue;
		}
		return v;
	}
}

template<> void ftscript<randomizeri>(int value, int counter) {
	auto count = script_count(counter, 1);
	for(auto i = 0; i < count; i++)
		script_run(bsdata<randomizeri>::elements[value].random());
}