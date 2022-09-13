#include "main.h"

void creaturea::select(indext index, int los) {
	auto pt = i2m(index);
	rect rc = {pt.x - los, pt.y - los, pt.x + los, pt.y + los};
	auto ps = data;
	auto pe = endof();
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		if(!e.in(rc))
			continue;
		if(ps < pe)
			*ps = &e;
	}
	count = ps - data;
}