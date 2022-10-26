#include "variantc.h"

void variantc::select(array& source) {
	auto ps = data;
	auto pe = endof();
	auto ae = source.end();
	auto s = source.getsize();
	for(auto p = source.begin(); p < ae; p += s) {
		if(ps >= pe)
			break;
		variant v = p;
		if(v)
			*ps++ = v;
	}
	count = ps - data;
}

void variantc::shuffle() {
	zshuffle(data, count);
}

variant variantc::picktop() {
	if(!count)
		return variant();
	auto r = data[0];
	remove(0, 1);
	return r;
}