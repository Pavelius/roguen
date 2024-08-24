#include "areapiece.h"
#include "itema.h"
#include "creature.h"

void itema::select(point m) {
	auto pb = data + count;
	auto pe = endof();
	for(auto& e : area->items) {
		if(!e)
			continue;
		if(e.position != m)
			continue;
		if(area->getfeature(m).is(Container))
			continue;
		if(pb < pe)
			*pb++ = &e;
	}
	count = pb - data;
}

void itema::select(creature* p) {
	auto pb = data + count;
	auto pe = endof();
	if(!p)
		return;
	for(auto& e : p->wears) {
		if(!e)
			continue;
		if(pb < pe)
			*pb++ = &e;
	}
	count = pb - data;
}

void itema::selectbackpack(creature* p) {
	auto pb = data + count;
	auto pe = endof();
	if(!p)
		return;
	for(auto& e : p->backpack()) {
		if(!e)
			continue;
		if(pb < pe)
			*pb++ = &e;
	}
	count = pb - data;
}

void itema::matchf(wear_s type, bool keep) {
	auto ps = data;
	for(auto p : *this) {
		if((p->geti().wear==type) != keep)
			continue;
		*ps++ = p;
	}
	count = ps - data;
}