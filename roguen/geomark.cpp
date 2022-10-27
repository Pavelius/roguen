#include "main.h"

geomark* geomark::find(point v) {
	for(auto& e : bsdata<geomark>()) {
		if(e.position == v)
			return &e;
	}
	return 0;
}