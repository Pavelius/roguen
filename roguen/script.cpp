#include "bsreq.h"
#include "script.h"

BSMETA(script) = {
	BSREQ(id),
	{}};

bool script::stop;

void script::run(int bonus) {
	proc(bonus);
}

void script::run(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(!p)
		return;
	p->proc(0);
}