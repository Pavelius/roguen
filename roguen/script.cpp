#include "bsreq.h"
#include "script.h"

bool			stop_script;
const script*	last_script;

BSMETA(script) = {
	BSREQ(id),
	{}};

void script::run(int bonus) {
	last_script = this;
	proc(bonus);
}

void script::run(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(p)
		p->run(bonus);
}