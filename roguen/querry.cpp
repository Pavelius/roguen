#include "list.h"
#include "math.h"
#include "querry.h"
#include "script.h"
#include "variant.h"

collectiona records;

static bool querry_allow(const void* object, const variants& source) {
	pushscript push(source);
	return querry_allow(object);
}

bool querry_allow(const void* object) {
	while(script_begin < script_end) {
		auto v = *script_begin++;
		auto keep = v.counter >= 0;
		if(v.iskind<listi>()) {
			if(querry_allow(object, bsdata<listi>::elements[v.value].elements) == keep)
				return true;
		} else {
			auto pf = bsdata<varianti>::elements[v.type].pfilter;
			if(pf) {
				if(pf(object, v.value) == keep)
					return true;
			} else {
				script_begin--; // Not handle
				break;
			}
		}
	}
	return false;
}

bool querry_allow_all(const void* object) {
	while(script_begin < script_end) {
		auto v = *script_begin++;
		auto keep = v.counter >= 0;
		if(v.iskind<listi>()) {
			if(querry_allow(object, bsdata<listi>::elements[v.value].elements) != keep)
				return false;
		} else {
			auto pf = bsdata<varianti>::elements[v.type].pfilter;
			if(pf) {
				if(pf(object, v.value) != keep)
					return false;
			} else {
				script_begin--; // Not handle
				break;
			}
		}
	}
	return true;
}

void querry_filter() {
	while(script_begin < script_end) {
		auto v = *script_begin++;
		if(v.iskind<listi>()) {
			pushscript push(bsdata<listi>::elements[v.value].elements);
			records.match(querry_allow, v.counter >= 0);
		} else if(v.iskind<querryi>())
			bsdata<querryi>::elements[v.value].proc(); // Grouping data (other querry overlaps)
		else {
			auto pf = bsdata<varianti>::elements[v.type].pfilter;
			if(pf)
				records.match(pf, v.value, v.counter >= 0);
			else {
				script_begin--; // Not handle
				break;
			}
		}
	}
}

template<> void fiscript<querryi>(int value, int counter) {
	bsdata<querryi>::elements[value].proc();
	if(!records)
		script_stop();
}
template<> bool fitest<querryi>(int value, int counter) {
	fiscript<querryi>(value, counter);
	return records.operator bool();
}