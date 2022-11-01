#include "stringbuilder.h"

#pragma once

struct listcolumn {
	const char*		id;
	int				width;
	fntext			proc;
	int				totalwidth() const;
	explicit operator bool() const { return id != 0; }
};
extern listcolumn* current_columns;
