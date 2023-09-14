#include "areamap.h"
#include "site.h"
#include "item.h"

#pragma once

struct areapiece : areaheadi, areamap {
	vector<roomi>		rooms;
	vector<itemground>	items;
	void				clear();
};
extern areapiece* area;