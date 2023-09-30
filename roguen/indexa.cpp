#include "areaf.h"
#include "areapiece.h"
#include "indexa.h"

static point compare_position;

static int compare_distace(const void* v1, const void* v2) {
	auto p1 = *((point*)v1);
	auto p2 = *((point*)v2);
	auto d1 = area->getrange(p1, compare_position);
	auto d2 = area->getrange(p2, compare_position);
	return d1 - d2;
}

void indexa::sort(point m) {
	compare_position = m;
	qsort(data, count, sizeof(data[0]), compare_distace);
}

void indexa::select(point pt, int range) {
	auto pb = data;
	auto pe = endof();
	point m;
	for(m.y = pt.y - range; m.y <= pt.y + range; m.y++) {
		for(m.x = pt.x - range; m.x <= pt.x + range; m.x++) {
			if(!area->isvalid(m))
				continue;
			if(!area->features[m])
				continue;
			if(pb < pe)
				*pb++ = m;
		}
	}
	count = pb - data;
}

void indexa::select(fnallowposition proc, bool keep, int offset) {
	auto pb = data;
	auto pe = endof();
	point m;
	for(m.y = offset; m.y < area->mps - offset; m.y++) {
		for(m.x = offset; m.x < area->mps - offset; m.x++) {
			if(proc(m) != keep)
				continue;
			if(pb < pe)
				*pb++ = m;
		}
	}
	count = pb - data;
}

void indexa::match(fnallowposition proc, bool keep) {
	auto ps = data;
	for(auto m : *this) {
		if(proc(m) != keep)
			continue;
		*ps++ = m;
	}
	count = ps - data;
}

void indexa::shuffle() {
	zshuffle(data, count);
}

void indexa::match(fnvisible proc, bool keep) {
	auto ps = data;
	for(auto p : *this) {
		auto& ei = area->getfeature(p);
		if(proc(&ei) != keep)
			continue;
		*ps++ = p;
	}
	count = ps - data;
}

void indexa::match(areaf v, bool keep) {
	auto ps = data;
	for(auto p : *this) {
		if(area->is(p, v) != keep)
			continue;
		*ps++ = p;
	}
	count = ps - data;
}