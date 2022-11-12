#include "variant.h"

#pragma once

struct script {
	typedef void(*fnrun)(int bonus);
	const char*		id;
	fnrun			proc;
	void			run(int bonus);
	static void		run(const char* id, int bonus = 0);
};
extern variant		param1, param2;
extern void			runscript(variant v);
extern void			runscript(const variants& elements);