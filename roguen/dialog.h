#pragma once

typedef void(*fnevent)();

struct buttoni {
	const char*	id;
	const char*	key;
};
struct dialogi {
	const char*	id;
	fnevent		mainscene, beforeopen;
	buttoni		buttons[16];
	int			open() const;
};
