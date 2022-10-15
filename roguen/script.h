#pragma once

struct script {
	typedef void(*fnrun)(int bonus);
	const char*		id;
	fnrun			proc;
	static bool		stop;
	static void		run(const char* id, int bonus = 0);
};
