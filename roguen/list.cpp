#include "bsreq.h"
#include "list.h"
#include "rand.h"
#include "script.h"

BSMETA(listi) = {
	BSREQ(id),
	BSREQ(elements),
	{}};
BSDATAC(listi, 256)

variant	listi::random() const {
	if(!elements.count)
		return variant();
	return elements.begin()[rand() % elements.count];
}

template<> bool fitest<listi>(int value, int counter) {
	return script_allow(bsdata<listi>::elements[value].elements);
}
template<> void fiscript<listi>(int value, int counter) {
	auto p = bsdata<listi>::elements + value;
	script_run(p->id, p->elements);
}