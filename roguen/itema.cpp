#include "itema.h"
#include "main.h"

void itema::select(point m) {
	auto pb = data + count;
	auto pe = endof();
	for(auto& e : bsdata<itemground>()) {
		if(!e)
			continue;
		if(e.position != m)
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
	for(auto i = Backpack; i <= BackpackLast; i = (wear_s)(i + 1)) {
		auto& e = p->wears[i];
		if(!e)
			continue;
		if(pb < pe)
			*pb++ = &e;
	}
	count = pb - data;
}