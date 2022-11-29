#include "bsreq.h"
#include "pushvalue.h"
#include "script.h"

variant	param1, param2;

BSMETA(script) = {
	BSREQ(id),
	{}};

void script::run(int bonus) {
	proc(bonus);
}

void script::run(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(p)
		p->run(bonus);
}