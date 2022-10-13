#include "main.h"

void itema::select(point m) {
	auto pb = data;
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

item* itema::choose(const char* title) const {
	if(count == 1)
		return data[0];
	answers an;
	for(auto p : *this)
		an.add(p, p->getfullname());
	return (item*)an.choose(title, 0);
}

void itema::select(creature* p) {
	auto pb = data;
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
	auto pb = data;
	auto pe = endof();
	if(!p)
		return;
	for(auto i = Backpack; i <= BackpackLast; i=(wear_s)(i+1)) {
		auto& e = p->wears[i];
		if(!e)
			continue;
		if(pb < pe)
			*pb++ = &e;
	}
	count = pb - data;
}