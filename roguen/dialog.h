#pragma once

typedef void(*fnevent)();

struct buttoni {
	const char*	id;
	const char*	key;
};
struct dialogi {
	const char*	id;
	fnevent		mainscene, beforeopen;
	bool		std_paint, no_keys;
	int			open() const;
};
void open_dialog(const char* id);