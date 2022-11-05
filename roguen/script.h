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