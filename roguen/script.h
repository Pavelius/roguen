#include "variant.h"

#pragma once

struct script {
	typedef void(*fnrun)(int bonus);
	const char*		id;
	fnrun			proc;
	void			run(int bonus);
	static void		run(const char* id, int bonus = 0);
	static void		run(variant v);
	static void		runv(const void* pv, int bonus = 0);
	static void		run(const variants& elements);
};
extern variant		param1, param2;
extern bool			ifscript(const variants& source);