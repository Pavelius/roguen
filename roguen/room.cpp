#include "main.h"

point center(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = rc.x1 + rc.width() / 2;
	short y = rc.y1 + rc.height() / 2;
	return {x, y};
}

bool roomi::is(feat_s v) const {
	auto p = getsite();
	if(!p)
		return false;
	return p->feats.is(v);
}

bool roomi::is(condition_s v) const {
	switch(v) {
	case ShowMinimapBullet: return is(Notable) && area.is(center(rc), Explored);
	case Identified: return ideftified != 0;
	default: return false;
	}
}