#include "collection.h"

void collectiona::select(array& source) {
	auto ps = data;
	auto pe = endof();
	auto ae = source.end();
	auto s = source.getsize();
	for(auto p = source.begin(); p < ae; p += s) {
		if(ps >= pe)
			break;
		*ps++ = p;
	}
	count = ps - data;
}

void collectiona::select(array& source, fnvisible proc) {
	if(!proc) {
		select(source);
		return;
	}
	auto ps = data;
	auto pe = endof();
	auto ae = source.end();
	auto s = source.getsize();
	for(auto p = source.begin(); p < ae; p += s) {
		if(ps >= pe)
			break;
		if(!proc(p))
			continue;
		*ps++ = p;
	}
	count = ps - data;
}

void* collectiona::random() const {
	if(!count)
		return 0;
	return data[rand() % count];
}