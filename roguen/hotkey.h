#include "variant.h"

#pragma once

struct hotkey {
	typedef void(*fnevent)(int bonus);
	const char*		id;
	unsigned		key;
	const char*		keyid;
	variant			data;
};
void check_hotkeys(int bonus);
unsigned hotkey_parse(const char* p);