#pragma once

typedef void(*fnevent)();
typedef bool(*fnenable)();

struct shortcuti {
	unsigned	key;
	fnevent		proc;
	fnenable	visible;
};
