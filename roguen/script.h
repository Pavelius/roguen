#include "variant.h"

#pragma once

struct script {
	typedef void(*fnrun)(int bonus);
	typedef bool(*fntest)(int bonus);
	const char*		id;
	fnrun			proc;
	fntest			test;
};
extern variant* script_begin;
extern variant* script_end;

int script_count(int count, int minimal = 1);

bool script_allow(variant v);
bool script_allow(const variants& elements);

void script_execute(const char* id, int bonus = 0);
void script_stop();
void script_run(variant v);
void script_run(variant v, int counter);
void script_run(const variants& elements);

template<typename T> void ftscript(int index, int value);
template<typename T> bool fttest(int index, int value);