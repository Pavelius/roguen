#include "variant.h"

#pragma once

struct hotkey {
	typedef void(*fnevent)(int bonus);
	const char*		id;
	unsigned		key;
	const char*		keyid;
	variant			data;
};
void hotkey_initialize();
unsigned hotkey_parse(const char* p);