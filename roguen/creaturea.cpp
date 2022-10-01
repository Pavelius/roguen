#include "main.h"

static point compare_position;

static int compare_distace(const void* v1, const void* v2) {
	auto p1 = *((creature**)v1);
	auto p2 = *((creature**)v2);
	auto d1 = area.getrange(p1->getposition(), compare_position);
	auto d2 = area.getrange(p2->getposition(), compare_position);
	return d1 - d2;
}

void creaturea::sort(point m) {
	compare_position = m;
	qsort(data, count, sizeof(data[0]), compare_distace);
}

void creaturea::select(point pt, int los) {
	rect rc = {pt.x - los, pt.y - los, pt.x + los, pt.y + los};
	auto ps = data;
	auto pe = endof();
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		if(!e.in(rc))
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	count = ps - data;
}

void creaturea::match(feat_s v, bool keep) {
	auto ps = data;
	for(auto e : *this) {
		if(e->is(v)!=keep)
			continue;
		*ps++ = e;
	}
	count = ps - data;
}