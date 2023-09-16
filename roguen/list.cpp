#include "bsreq.h"
#include "list.h"
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

template<> bool fttest<listi>(int value, int counter) {
	return script_allow(bsdata<listi>::elements[value].elements);
}
template<> void ftscript<listi>(int value, int counter) {
	// Do not use counter
	for(auto v : bsdata<listi>::elements[value].elements)
		script_run(v);
}