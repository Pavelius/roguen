#include "bsreq.h"
#include "pushvalue.h"
#include "script.h"

variant param1, param2;
script* script_begin;
script* script_end;

BSMETA(script) = {
	BSREQ(id),
	{}};

bool script::isallow(const variants& elements) {
	for(auto v : elements) {
		if(!isallow(v))
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