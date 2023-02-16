#pragma once

typedef void(*fnevent)();

struct widget {
	const char*	id;
	fnevent		proc, click;
	explicit operator bool() const { return id != 0; }
};
extern widget last_widget;