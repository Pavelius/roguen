#include "areamap.h"
#include "site.h"
#include "item.h"
#include "timemanage.h"
#include "vector.h"

#pragma once

struct areapiece : areaheadi, areamap, timemanage {
	vector<roomi>		rooms;
	vector<itemground>	items;
	void				clear();
};
extern areapiece* area;