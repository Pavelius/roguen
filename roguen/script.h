#include "variant.h"

#pragma once

struct script {
	typedef void(*fnrun)(int bonus);
	typedef bool(*fntest)(int bonus);
	const char*		id;
	fnrun			proc;
	fntest			test;
	static bool		isallow(variant v);
	static bool		isallow(const variants& elements);
	void			run(int bonus);
	static void		run(const char* id, int bonus = 0);
	static void		run(variant v);
	static void		runv(const void* pv, int bonus = 0);
	static void		run(const variants& elements);
};