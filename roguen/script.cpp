#include "bsreq.h"
#include "pushvalue.h"
#include "script.h"

BSMETA(script) = {
	BSREQ(id),
	{}};

variant param1, param2;
variant* script_begin;
variant* script_end;

template<> bool fttest<script>(int value, int counter) {
	if(bsdata<script>::elements[value].test)
		return bsdata<script>::elements[value].test(counter);
	return true;
}
template<> void ftscript<script>(int value, int counter) {
	bsdata<script>::elements[value].proc(counter);
}

int script_count(int count, int minimal) {
	if(count < 0 && d100() >= -count)
		return -1;
	if(count < minimal)
		count = minimal;
	return count;
}

void script_stop() {
	script_begin = script_end;
}

bool script_allow(variant v) {
	auto proc = bsdata<varianti>::elements[v.type].ptest;
	if(proc)
		return proc(v.value, v.counter);
	return true;
}

bool script_allow(const variants& elements) {
	pushvalue push_begin(script_begin, elements.begin());
	pushvalue push_end(script_end, elements.end());
	while(script_begin < script_end) {
		if(!script_allow(*script_begin++))
			return false;
	}
	return true;
}

void script_run(variant v, int bonus) {
	if(!v)
		return;
	v.counter = bonus;
	script_run(v);
}

void script_run(variant v) {
	auto proc = bsdata<varianti>::elements[v.type].pscript;
	if(proc)
		proc(v.value, v.counter);
}

void script_run(const variants& elements) {
	pushvalue push_begin(script_begin, elements.begin());
	pushvalue push_end(script_end, elements.end());
	while(script_begin < script_end)
		script_run(*script_begin++);
}

void script_execute(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(p)
		p->proc(bonus);
}