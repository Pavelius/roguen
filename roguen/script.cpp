#include "bsreq.h"
#include "pushvalue.h"
#include "rand.h"
#include "script.h"

BSMETA(script) = {
	BSREQ(id),
	{}};
BSMETA(modifieri) = {
	BSREQ(id),
	{}};

variant* script_begin;
variant* script_end;
modifiern modifier;
bool script_fail;

const char* last_id;

static void script_run_def() {
	while(script_begin < script_end) {
		auto v = *script_begin++;
		bsdata<varianti>::elements[v.type].pscript(v.value, v.counter);
	}
}

fnevent script_run_proc = script_run_def;

template<> void fiscript<modifieri>(int value, int counter) {
	modifier = (modifiern)value;
}
template<> bool fitest<modifieri>(int value, int counter) {
	fiscript<modifieri>(value, counter);
	return true;
}

template<> void fiscript<script>(int value, int counter) {
	bsdata<script>::elements[value].proc(counter);
}

int script_count(int count, int minimal) {
	if(count < 0 && d100() >= -count)
		return -1;
	if(count > 100)
		count = xrand(1, count - 100);
	if(count < minimal)
		count = minimal;
	return count;
}

void script_empty() {
}

void script_stop() {
	script_begin = script_end;
}

bool script_stopped() {
	return script_begin == script_end;
}

static bool script_allow() {
	while(script_begin < script_end) {
		auto v = *script_begin++;
		auto proc = bsdata<varianti>::elements[v.type].ptest;
		if(!proc)
			break;
		if(!proc(v.value, v.counter))
			return false;
	}
	return true;
}

bool script_allow(const variants& elements) {
	pushscript push(elements);
	return script_allow();
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

void script_run(const variants& source) {
	pushscript push(source);
	script_run_proc();
}

void script_run(const variants& source, fnevent proc) {
	auto push = script_run_proc; script_run_proc = proc;
	script_run(source);
	script_run_proc = push;
}

void script_run(const char* id, const variants& source) {
	pushscriptid push(id);
	script_run(source);
}

void script_execute(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(p)
		p->proc(bonus);
}

variant next_script() {
	if(script_begin < script_end)
		return *script_begin++;
	return variant();
}