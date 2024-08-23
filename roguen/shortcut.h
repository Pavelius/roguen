#pragma once

typedef void(*fnevent)();

struct shortcuti {
	unsigned	key;
	fnevent		proc;
};
