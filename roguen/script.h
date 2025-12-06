#include "variant.h"

#pragma once

enum modifiern : unsigned char;

struct script {
	typedef void(*fnrun)(int bonus);
	const char*	id;
	fnrun proc;
};
struct modifieri {
	const char*	id;
};
extern modifiern modifier;
extern variant* script_begin;
extern variant* script_end;
extern fnevent script_run_proc;
extern const char* last_id;
extern bool script_fail;

struct pushmodifier {
	modifiern m;
	pushmodifier() : m(modifier) {}
	~pushmodifier() { modifier = m; }
};
struct pushscript : pushmodifier {
	variant* begin;
	variant* end;
	pushscript() : begin(script_begin), end(script_end) {}
	pushscript(variant* vbegin, variant* vend) : pushscript() { script_begin = vbegin, script_end = vend; }
	pushscript(const variants& v) : pushscript() { script_begin = v.begin(), script_end = v.end(); }
	~pushscript() { restore(); }
	void restore() const { script_begin = begin; script_end = end; }
	void stop() { begin = end; }
};
struct pushscriptid {
	const char* id;
	pushscriptid() : id(last_id) {}
	pushscriptid(const char* nid) : pushscriptid() { last_id = nid; }
	~pushscriptid() { last_id = id; }
};

int script_count(int count, int minimal = 1);

bool script_allow(variant v);
bool script_allow(const variants& elements);
void script_empty();
void script_empty(int bonus);
void script_execute(const char* id, int bonus = 0);
void script_stop();
bool script_stopped();
void script_run(variant v);
void script_run(variant v, int counter);
void script_run(const char* id, const variants& source);
void script_run(const variants& elements);
void script_run(const variants& source, fnevent proc);