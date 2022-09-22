#include "main.h"

static int compare_rect(const void* p1, const void* p2) {
	auto e1 = (rect*)p1;
	auto e2 = (rect*)p2;
	return e2->width() * e2->height() - e1->width() * e1->height();
}

static bool iswall(indext i, direction_s d1, direction_s d2) {
	return area.iswall(i, d1)
		&& area.iswall(i, d2);
}

static bool isonewall(indext i, direction_s d1, direction_s d2) {
	return area.iswall(i, d1)
		|| area.iswall(i, d2);
}

static bool isoneof(indext i, direction_s d1, direction_s d2, feature_s v) {
	auto f1 = area.getfeature(to(i, d1));
	auto f2 = area.getfeature(to(i, d1));
	return f1 == v || f2 == v;
}

static void update_doors() {
	for(indext i = 0; i <= mps * mps; i++) {
		if(bsdata<featurei>::elements[area.features[i]].is(BetweenWalls)) {
			if(iswall(i, West, East) && !isoneof(i, North, South, Door) && !isonewall(i, North, South))
				continue;
			if(iswall(i, North, South) && !isoneof(i, West, East, Door) && !isonewall(i, West, East))
				continue;
			area.set(i, NoFeature);
		}
	}
}

static void update_creatures() {
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		auto i = e.getindex();
		if(area.getfeature(i) == Door)
			e.clear();
	}
}

static void update_items() {
	for(auto& e : bsdata<itemground>()) {
		if(!e || e.index == Blocked)
			continue;
		if(area.getfeature(e.index) == Door)
			e.clear();
	}
}