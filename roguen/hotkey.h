#include "crt.h"

#pragma once

struct hotkey {
	typedef void(*fnevent)(int bonus);
	const char*		id;
	unsigned		key;
	const char*		keyid;
	fnevent			proc;
	struct dialogi*	dialog;
	static void		initialize();
};
