#include "bsreq.h"
#include "pushvalue.h"
#include "script.h"

variant param1, param2;
variant* script_begin;
variant* script_end;

BSMETA(script) = {
	BSREQ(id),
	{}};

void script_stop() {
	script_begin = script_end;
}

bool script::isallow(const variants& elements) {
	pushvalue push_begin(script_begin, elements.begin());
	pushvalue push_end(script_end, elements.end());
	while(script_begin < script_end) {
		if(!isallow(*script_begin++))
			return false;
	}
	return true;
}

void script::runv(const void* pv, int bonus) {
	variant v = pv;
	if(!v)
		return;
	v.counter = bonus;
	run(v);
}

void script::run(int bonus) {
	proc(bonus);
}

void script::run(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(p)
		p->run(bonus);
}