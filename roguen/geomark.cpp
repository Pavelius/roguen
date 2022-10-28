#include "main.h"

void geomark::clear() {
	memset(this, 0, sizeof(*this));
	position = {-1000, -1000};
}

geomark* geomark::create(point position, variant site, variant adjective) {
	auto p = bsdata<geomark>::add();
	p->clear();
	p->position = position;
	p->site = site;
	p->adjective = adjective;
	return p;
}

geomark* geomark::find(point v) {
	for(auto& e : bsdata<geomark>()) {
		if(e.position == v)
			return &e;
	}
	return 0;
}