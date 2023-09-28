#include "areaf.h"
#include "areapiece.h"
#include "condition.h"
#include "game.h"
#include "site.h"

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