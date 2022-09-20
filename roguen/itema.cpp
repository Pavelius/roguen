#include "main.h"

void itema::select(indext index) {
	auto pb = data;
	auto pe = endof();
	for(auto& e : bsdata<itemground>()) {
		if(e.index != index)
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
		an.add(p, p->getname());
	return (item*)an.choose(title, 0);
}