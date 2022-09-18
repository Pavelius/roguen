#include "main.h"

void indexa::select(point pt, int range) {
	auto pb = data;
	auto pe = endof();
	for(auto y = pt.y - range; y <= pt.y + range; y++) {
		if(y < 0)
			continue;
		if(y >= mps)
			break;
		for(auto x = pt.x - range; x <= pt.x + range; x++) {
			if(x < 0)
				continue;
			if(x >= mps)
				break;
			auto i = m2i(x, y);
			if(!area.features[i])
				continue;
			if(pb < pe)
				*pb++ = i;
		}
	}
	count = pb - data;
}