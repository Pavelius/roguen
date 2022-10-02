#include "main.h"

static adat<rect, 32> locations;
static adat<variant, 32> sites;

typedef void(*fnscene)(const rect& rc);

static point random(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = xrand(rc.x1, rc.x2);
	short y = xrand(rc.y1, rc.y2);
	return {x, y};
}

static point center(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = rc.x1 + rc.width() / 2;
	short y = rc.y1 + rc.height() / 2;
	return {x, y};
}

static bool iswall(point i, direction_s d1, direction_s d2) {
	return area.iswall(i, d1)
		&& area.iswall(i, d2);
}

static bool isonewall(point i, direction_s d1, direction_s d2) {
	return area.iswall(i, d1)
		|| area.iswall(i, d2);
}

static bool isoneof(point i, direction_s d1, direction_s d2, feature_s v) {
	auto f1 = area.getfeature(to(i, d1));
	auto f2 = area.getfeature(to(i, d1));
	return f1 == v || f2 == v;
}

static void update_doors() {
	point i;
	for(i.y = 0; i.y < area.mps; i.y++) {
		for(i.x = 0; i.x < area.mps; i.x++) {
			if(bsdata<featurei>::elements[area.features[i]].is(BetweenWalls)) {
				if(iswall(i, West, East) && !isoneof(i, North, South, Door) && !isonewall(i, North, South))
					continue;
				if(iswall(i, North, South) && !isoneof(i, West, East, Door) && !isonewall(i, West, East))
					continue;
				area.set(i, NoFeature);
			}
		}
	}
}

static void update_creatures() {
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		auto i = e.getposition();
		if(area.getfeature(i) == Door)
			e.clear();
	}
}

static void update_items() {
	for(auto& e : bsdata<itemground>()) {
		if(!e || !area.isvalid(e.position))
			continue;
		if(area.getfeature(e.position) == Door)
			e.clear();
	}
}

static int getarea(const rect & rc) {
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
	const int mp4 = area.mps / mpp;
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

static void create_building(const rect & rc) {
}

static void create_locations(fnscene proc) {
	for(auto& e : locations)
		proc(e);
}

static void place_shape(const shapei & e, point m, direction_s d, tile_s floor, tile_s wall) {
	auto c = e.center(m);
	for(m.y = 0; m.y < e.size.y; m.y++) {
		for(m.x = 0; m.x < e.size.x; m.x++) {
			auto pm = e.translate(c, m, d);
			auto sm = e[m];
			switch(sm) {
			case ' ':
				break;
			case '.':
				area.set(pm, floor);
				break;
			case 'X':
				area.set(pm, wall);
				break;
			case '0':
				area.set(pm, floor);
				break;
			default:
				break;
			}
		}
	}
}

static void place_shape(const shapei& e, point m, tile_s floor, tile_s walls) {
	static direction_s direction[] = {North, South, West, East};
	place_shape(e, m, maprnd(direction), floor, walls);
}

static void create_location_general() {
	const auto parts = 4;
	const auto size = area.mps / parts;
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

static void place_monsters(const rect & rca, const monsteri & e, int count) {
	if(count == 0) {
		if(game.isoutdoor())
			count = e.getbase().appear_outdoor.roll();
		else
			count = e.getbase().appear.roll();
	}
	for(auto i = 0; i < count; i++)
		creature::create(center(rca), &e);
}

static bool test_counter(variant v) {
	if(v.counter > 0 && d100() >= v.counter)
		return false;
	return true;
}

static void create_landscape(const rect & rca, variant v) {
	static sitei* last_site;
	if(v.iskind<featurei>())
		area.set(rca, (feature_s)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(rca, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(rca, (mapf_s)v.value, v.counter);
	else if(v.iskind<sitei>()) {
		pushvalue push_site(last_site);
		last_site = bsdata<sitei>::elements + v.value;
		for(auto ev : bsdata<sitei>::elements[v.value].landscape)
			create_landscape(rca, ev);
	} else if(v.iskind<monsteri>())
		place_monsters(rca, bsdata<monsteri>::elements[v.value], v.counter);
	else if(v.iskind<shapei>()) {
		if(!last_site || !test_counter(v))
			return;
		place_shape(bsdata<shapei>::elements[v.value],
			random(rca.shrink(5, 5)), last_site->floors, last_site->walls);
	}
}

static void add_area_sites(variant v) {
	if(!test_counter(v))
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
		create_landscape(locations.data[i], sites.data[i]);
}

void create_area(const char* id) {
	variant tile = id;
	if(!tile)
		return;
	locations.clear();
	sites.clear();
	create_landscape({0, 0, area.mps - 1, area.mps - 1}, tile);
	add_area_sites(tile);
	create_location_general();
	create_sites();
}