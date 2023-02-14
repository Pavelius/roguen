#pragma once

typedef void(*fnevent)();

struct dialogi {
	const char*	id;
	fnevent		mainscene, beforeopen;
	int			open() const;
};
