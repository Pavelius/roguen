#include "variant.h"

#pragma once

struct script {
	typedef void(*fnrun)(int bonus);
	const char*		id;
	fnrun			proc;
	void			run(int bonus);
	static void		run(const char* id, int bonus = 0);
};

extern bool			stop_script;
extern const script* last_script;
extern fnvariant	last_scipt_proc;
extern variant		param1, param2;

void				runscript(variant v);
void				runscript(const variants& elements);
void				runscript(const variants& elements, fnvariant proc);