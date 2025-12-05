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

inline bool haveposition(const void* p) { return area && area->tiles.have(p); }
inline bool haveitem(const void* p) { return area && area->items.have(p); }