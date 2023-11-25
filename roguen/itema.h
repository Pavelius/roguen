#include "collection.h"
#include "item.h"

#pragma once

class creature;

struct itema : collection<item> {
	void select(point m);
	void select(creature* p);
	void selectbackpack(creature* p);
	void matchf(wear_s kind, bool keep);
};
extern itema items;