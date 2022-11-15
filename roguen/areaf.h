#include "framerange.h"

#pragma once

enum areaf : unsigned char { Explored, Visible, Hidden, Darkened, Blooded, Iced, Webbed };
struct areafi {
	const char*		id;
	framerange		features;
};
