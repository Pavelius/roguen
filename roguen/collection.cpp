#include "answers.h"
#include "collection.h"

static fngetname sort_proc;

static int compare_proc(const void* v1, const void* v2) {
	auto p1 = *((void**)v1);
	auto p2 = *((void**)v2);
	return strcmp(sort_proc(p1), sort_proc(p2));
}

static bool exist(void** pb, void** pe, const void* v) {
	while(pb < pe) {
		if(*pb == v)
			return true;
		pb++;
	}
	return false;
}

void collectiona::distinct() {
	auto ps = data;
	for(auto p : *this) {
		if(!exist(data, ps, p))
			*ps++ = p;
	}
	count = ps - data;
}

void collectiona::group(fngroup proc) {
	for(auto& e : *this)
		e = proc(e);
	distinct();
}

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

void* collectiona::choose(fngetname proc, const char* title, const char* cancel, bool autochoose) const {
	if(autochoose && count == 1)
		return data[0];
	answers an;
	for(auto p : *this)
		an.add(p, proc(p));
	return an.choose(title, cancel);
}

void collectiona::sort(fngetname proc) {
	qsort(data, count, sizeof(data), compare_proc);
}