#include "areamap.h"
#include "indexa.h"

extern areamap area;

void indexa::select(point pt, int range) {
	auto pb = data;
	auto pe = endof();
	point m;
	for(m.y = pt.y - range; m.y <= pt.y + range; m.y++) {
		if(!area.isvalid(m))
			continue;
		for(m.x = pt.x - range; m.x <= pt.x + range; m.x++) {
			if(!area.isvalid(m))
				continue;
			if(!area.features[m])
				continue;
			if(pb < pe)
				*pb++ = m;
		}
	}
	count = pb - data;
}