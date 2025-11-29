#include "bsreq.h"
#include "script.h"
#include "stringbuilder.h"
#include "list.h"

BSMETA(script) = {
	BSREQ(id),
	{}};
variant* script_begin;
variant* script_end;
script*	last_script;

const char* last_id;

static void script_run_def() {
	while(script_begin < script_end) {
		auto v = *script_begin++;
		bsdata<varianti>::elements[v.type].pscript(v.value, v.counter);
	}
}

fnevent script_run_proc = script_run_def;

template<> void fnscript<script>(int value, int bonus) {
	last_script = bsdata<script>::elements + value;
	last_script->proc(bonus);
}

void script_none(int bonus) {
}

bool choosing_script(int bonus) {
	last_script->proc(bonus);
	return true;
}

void script_stop() {
	script_begin = script_end;
}

bool script_isrun() {
	return script_end > script_begin;
}

void script_list(const char* id) {
	auto p = bsdata<listi>::find(id);
	if(p)
		script_run(p->id, p->elements);
}

void script_run(const variants& source) {
	pushscript push(source);
	script_run_proc();
}

void script_run(const variants& source, fnevent proc) {
	auto push = script_run_proc; script_run_proc = proc;
	script_run(source);
	script_run_proc = push;
}

void script_run(const char* id, const variants& source, fnevent proc) {
	pushscriptid push(id);
	script_run(source, proc);
}

void script_run(const char* id, const variants& source) {
	pushscriptid push(id);
	script_run(source);
}

const char* script_header() {
	if(last_id) {
		auto pn = getnme(ids(last_id, "Header"));
		if(pn)
			return pn;
	}
	return getnm("ChooseDefHeader");
}
