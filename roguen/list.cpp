#include "bsreq.h"
#include "list.h"

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