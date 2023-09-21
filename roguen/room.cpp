#include "areaf.h"
#include "areapiece.h"
#include "condition.h"
#include "game.h"
#include "site.h"

point center(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = rc.x1 + rc.width() / 2;
	short y = rc.y1 + rc.height() / 2;
	return {x, y};
}

roomi* roomi::add() {
	return area->rooms.add();
}

roomi* roomi::find(point pt) {
	for(auto& e : area->rooms) {
		if(pt.in(e.rc))
			return &e;
	}
	return 0;
}

bool roomi::isexplored() const {
	return area->is(center(), Explored);
}

bool roomi::ismarkable() const {
	return is(Notable) && isexplored();
}