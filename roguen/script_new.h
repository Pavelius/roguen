#include "variant.h"

#pragma once

template<typename T> void fnscript(int index, int bonus);
template<typename T> bool fntest(int index, int bonus);

enum modifiern : unsigned char;
struct modifieri {
	const char*	id;
};
extern modifiern modifier;

struct script {
	typedef void(*fnrun)(int bonus);
	const char*	id;
	fnrun proc, method;
};
extern variant* script_begin;
extern variant* script_end;
extern script* last_script;
extern fnevent script_run_proc;
extern const char* last_id;

struct pushscript {
	variant* begin;
	variant* end;
	pushscript() : begin(script_begin), end(script_end) {}
	pushscript(variant* vbegin, variant* vend) : pushscript() { script_begin = vbegin, script_end = vend; }
	pushscript(const variants& v) : pushscript() { script_begin = v.begin(), script_end = v.end(); }
	~pushscript() { script_begin = begin; script_end = end; }
};
struct pushscriptid {
	const char* id;
	pushscriptid() : id(last_id) {}
	pushscriptid(const char* nid) : pushscriptid() { last_id = nid; }
	~pushscriptid() { last_id = id; }
};

bool choosing_script(int bonus);
bool script_isrun();
void script_list(const char* id);
void script_none(int bonus);
void script_run(const variants& source);
void script_run(const variants& source, fnevent proc);
void script_run(const char* id, const variants& source);
void script_run(const char* id, const variants& source, fnevent proc);
void script_stop();

const char* script_header();
