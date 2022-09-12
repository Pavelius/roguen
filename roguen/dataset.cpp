#include "dataset.h"

void dataset::select(array& source) {
	short unsigned i = 0;
	size_t c = source.getcount();
	auto p = data;
	for(auto pe = endof(); p < pe && i < c; p++, i++)
		*p = i;
	count = p - data;
}

void dataset::select(array& source, fnvisible proc) {
	short unsigned i = 0;
	size_t c = source.getcount();
	auto p = data;
	for(auto pe = endof(); p < pe && i < c; p++, i++) {
		if(!proc(source.ptr(i)))
			continue;
		*p = i;
	}
	count = p - data;
}