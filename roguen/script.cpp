#include "bsreq.h"
#include "pushvalue.h"
#include "script.h"

bool			stop_script;
const script*	last_script;
fnvariant		last_scipt_proc;
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

void runscript(const variants& elements) {
	if(!last_scipt_proc)
		return;
	if(stop_script)
		return;
	pushvalue push_stop(stop_script);
	for(auto v : elements) {
		if(stop_script)
			break;
		last_scipt_proc(v);
	}
}

void runscript(const variants& elements, fnvariant proc) {
	pushvalue push_proc(last_scipt_proc, proc);
	runscript(elements);
}

void runscript(variant v) {
	if(last_scipt_proc)
		last_scipt_proc(v);
}