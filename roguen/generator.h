#include "variant.h"

#pragma once

struct generatori {
	const char*		id;
	char			chance;
	variant			value;
	void			clear();
	static void		read(const char* id);
};
variant random_value(const char* id);
variant random_list_value(variant value);