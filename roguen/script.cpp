#include "bsreq.h"
#include "pushvalue.h"
#include "script.h"

const script*	last_script;
variant			param1, param2;

BSMETA(script) = {
	BSREQ(id),
	{}};

void script::run(int bonus) {
	auto push = last_script;
	last_script = this;
	proc(bonus);
	last_script = push;
}

void script::run(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(p)
		p->run(bonus);
}