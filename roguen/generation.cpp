#include "main.h"

static adat<rect, 32> locations;
static adat<variant, 32> sites;

typedef void(*fnscene)(const rect& rc);

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

static int getarea(const rect& rc) {
	return (rc.width() + 1) * (rc.height() + 1);
}

static int compare_locations(const void* v1, const void* v2) {
	return getarea(*((rect*)v2)) - getarea(*((rect*)v1));
}

static void sort_locations() {
	qsort(locations.data, locations.count, sizeof(locations.data[0]), compare_locations);
}

static void create_location_areas() {
	const int mpp = 4;
	const int mp4 = mps / mpp;
	const int mp8 = mp4 / 2;
	locations.clear();
	for(auto y = 0; y < 4; y++) {
		for(auto x = 0; x < 4; x++) {
			auto x1 = x * mp4 + xrand(2, mp4 - 2 - mp8);
			auto y1 = y * mp4 + xrand(2, mp4 - 2 - mp8);
			auto x2 = x1 + xrand(3, mp8);
			auto y2 = y1 + xrand(3, mp8);
			locations.add({x1, y1, x2, y2});
		}
	}
	sort_locations();
}

static void remove_smalest() {
	if(locations)
		locations.count--;
}

static void create_building(const rect& rc) {
}

static void create_locations(fnscene proc) {
	for(auto& e : locations)
		proc(e);
}

static void place_shape(const shapei& e, pointm m, direction_s d, tile_s floor, tile_s wall) {
	auto n = e.size.maximum();
	auto c = e.center(m);
	for(size_t i = 0; i < n; i++) {
		auto pm = e.translate(c, e.i2m(i), d);
		auto sm = e.content[i];
		switch(sm) {
		case ' ':
			break;
		case '.':
			area.set(m2i(pm), floor);
			break;
		case 'X':
			area.set(m2i(pm), wall);
			break;
		case '0':
			area.set(m2i(pm), floor);
			break;
		default:
			break;
		}
	}
}

static void place_features(pointm pm, const char* id) {
	static direction_s direction[] = {North, South, West, East};
	auto p = bsdata<shapei>::find(id);
	if(!p)
		return;
	place_shape(*p, pm, maprnd(direction), Cave, WallCave);
}

static void create_location_general() {
	const auto parts = 4;
	const auto size = mps / parts;
	const auto locmin = 3;
	const auto locmax = 2 * size / 3 - 3;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = (i % parts) * size + 2;
		rc.y1 = (i / parts) * size + 2;
		rc.x2 = rc.x1 + xrand(locmin, locmax - 1);
		rc.y2 = rc.y1 + xrand(locmin, locmax);
		locations.add(rc);
	}
	qsort(locations.data, locations.count, sizeof(locations.data[0]), compare_locations);
}

static void create_area(const rect& rca, variant v) {
	if(v.iskind<featurei>())
		area.set(rca, (feature_s)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(rca, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(rca, (mapf_s)v.value, v.counter);
	else if(v.iskind<sitei>()) {
		for(auto ev : bsdata<sitei>::elements[v.value].landscape)
			create_area(rca, ev);
	}
}

static void add_area_sites(variant v) {
	if(v.counter > 0 && d100() >= v.counter)
		return;
	if(v.iskind<sitei>()) {
		auto& ei = bsdata<sitei>::elements[v.value];
		if(ei.sites) {
			for(auto ev : ei.sites)
				add_area_sites(ev);
		} else
			sites.add(v);
	}
}

static void create_sites() {
	if(sites.count < locations.count)
		locations.count = sites.count;
	if(sites.count > locations.count)
		sites.count = locations.count;
	for(size_t i = 0; i < locations.count; i++)
		create_area(locations.data[i], sites.data[i]);
}

void create_area(const char* id) {
	locations.clear();
	sites.clear();
	create_area({0, 0, mps - 1, mps - 1}, id);
	add_area_sites(id);
	create_location_general();
	create_sites();
}