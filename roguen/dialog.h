#pragma once

typedef void(*fnevent)();

struct buttoni {
	const char*	id;
	const char*	key;
};
struct dialogi {
	const char*	id;
	fnevent		mainscene;
	int			open() const;
};
void open_dialog(const char* id);