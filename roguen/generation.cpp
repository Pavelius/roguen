#include "main.h"

static adat<rect, 32> locations;
static adat<variant, 32> sites;
static direction_s connectors_side[] = {North, East, West, South};

const auto minimum_corridor_lenght = 4;
const auto chance_create_door = 10;
const auto chance_corridor_content = 10;
const auto chance_line_corridor = 40;
const auto chance_hidden_door = 10;

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

static void create_road(const rect& rc) {
	area.set(rc, Rock);
}

static void create_city_level(const rect& rc, int level) {
	const int chance_special_area = 20;
	const int max_building_size = 5;
	auto w = rc.width();
	auto h = rc.height();
	if(d100() < chance_special_area &&
		w > max_building_size && w<max_building_size * 3 && h>max_building_size && h < max_building_size * 3) {
		return;
	}
	if(w <= max_building_size && h <= max_building_size) {
		if(w > 6 && h > 6)
			locations.add({rc.x1, rc.y1, rc.x2 - 3, rc.y2 - 3});
		return;
	}
	int m = xrand(30, 60);
	int r = -1;
	if(w / 3 >= h / 2)
		r = 0;
	else if(h / 3 >= w / 2)
		r = 1;
	if(r == -1)
		r = (d100() < 50) ? 0 : 1;
	if(r == 0) {
		int w1 = (w * m) / 100; // horizontal
		create_city_level({rc.x1, rc.y1, rc.x1 + w1, rc.y2}, level + 1);
		create_city_level({rc.x1 + w1 + 1, rc.y1, rc.x2, rc.y2}, level + 1);
		if(level <= 2) {
			auto r1 = rc;
			if(r1.y2 >= area.mps - 3)
				r1.y2 = area.mps - 1;
			if(r1.y1 <= 2)
				r1.y1 = 0;
			rect r2 = {r1.x1 + w1 - 2, r1.y1, r1.x1 + w1, r1.y2};
			create_road(r2);
			//for(int i = xrand(0, 4); i > 0; i--)
			//	loc.commoner(loc.getrand(r2));
		}
	} else {
		int h1 = (h * m) / 100; // vertial
		create_city_level({rc.x1, rc.y1, rc.x2, rc.y1 + h1}, level + 1);
		create_city_level({rc.x1, rc.y1 + h1 + 1, rc.x2, rc.y2}, level + 1);
		if(level <= 2) {
			auto r1 = rc;
			if(r1.x2 >= area.mps - 3)
				r1.x2 = area.mps - 1;
			if(r1.x1 <= 2)
				r1.x1 = 0;
			rect r2 = {r1.x1, r1.y1 + h1 - 2, r1.x2, r1.y1 + h1};
			create_road(r2);
			//for(int i = xrand(0, 4); i > 0; i--)
			//	loc.commoner(loc.getrand(r2));
		}
	}
}

static void create_location_general() {
	const auto parts = 4;
	const auto size = area.mps / parts;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = (i % parts) * size + 1;
		rc.y1 = (i / parts) * size + 1;
		rc.x2 = rc.x1 + size - 2;
		rc.y2 = rc.y1 + size - 2;
		locations.add(rc);
	}
}

static void create_dungeon_rooms() {
}

static bool isallowcorridor(point index, tile_s tile, const rect& correct, direction_s dir) {
	auto i0 = to(index, dir); // Forward
	auto i1 = to(i0, dir); // Forward - Forward
	if(!i0.in(correct))
		return false;
	if(area.is(i0, Explored))
		return true;
	if(area[i0] == tile)
		return false;
	if(area.is(i1, Explored))
		return true;
	if(area[i1] == tile)
		return false;
	auto d1 = round(dir, West);
	auto d2 = round(dir, East);
	auto i2 = to(i0, d1); // Forward - Left
	auto i3 = to(i0, d2); // Forward - Right
	if(area[i2] == tile || area[i3] == tile)
		return false;
	auto i4 = to(i1, d1); // Forward - Forward - Left
	auto i5 = to(i1, d2); // Forward - Forward - Right
	if(area[i4] == tile || area[i5] == tile)
		return false;
	return true;
}

static void create_corridor_content(point m) {
}

static void create_door(point m, bool hidden) {
}

static void create_connector(point index, direction_s dir, const rect& correct) {
	point start = {-1000, -1000};
	auto tile = DungeonFloor;
	auto floor = DungeonFloor;
	auto count = 0;
	while(true) {
		if(!isallowcorridor(index, tile, correct, dir))
			break;
		auto i0 = to(index, dir); // Forward
		if(area.is(i0, Explored))
			return;
		if(area[i0] == tile)
			return;
		area[i0] = floor;
		if(!start) {
			start = i0;
			if(d100() < chance_create_door)
				create_door(i0, d100() < chance_hidden_door);
		} else {
			if(d100() < chance_corridor_content)
				create_corridor_content(i0);
		}
		index = i0;
		if(count >= minimum_corridor_lenght && d100() >= chance_line_corridor)
			break;
		count++;
	}
	//show_minimap_step(index);
	direction_s rnd[] = {West, East, North};
	zshuffle(rnd, 3);
	for(auto e : rnd) {
		auto d1 = round(dir, e);
		if(!isallowcorridor(index, tile, correct, d1))
			continue;
		create_connector(index, d1, correct);
	}
}

static void create_corridor(int x, int y, int w, int h, direction_s dir, const rect& correct) {
	//switch(dir) {
	//case North: create_connector(area.get(x + rand() % w, y), North, correct); break;
	//case West: create_connector(area.get(x, y + rand() % h), West, correct); break;
	//case South: create_connector(area.get(x + rand() % w, y + h - 1), South, correct); break;
	//default: create_connector(area.get(x + w - 1, y + rand() % h), East, correct); break;
	//}
}

static void create_room_door(point m, bool hidden) {
	if(area.getfeature(m) == Door) {
		if(hidden)
			area.set(m, Explored);
		return;
	}
	if(area[m] == DungeonFloor)
		create_door(m, hidden);
}

static void create_doors(const rect& rc) {
	auto hidden_room = d100() < chance_hidden_door / 2;
	for(short x = rc.x1; x <= rc.x2; x++) {
		create_room_door({x, (short)rc.y1}, hidden_room);
		create_room_door({x, (short)rc.y2}, hidden_room);
	}
	for(short y = rc.y1; y <= rc.y2; y++) {
		create_room_door({(short)rc.x1, y}, hidden_room);
		create_room_door({(short)rc.x2, y}, hidden_room);
	}
}

static void create_dungeon_content(const rect& rc) {
	create_dungeon_rooms();
	for(auto& e : locations) {
		direction_s side[4]; memcpy(side, connectors_side, sizeof(connectors_side));
		zshuffle(side, sizeof(side) / sizeof(side[0]));
		create_corridor(e.x1, e.y1, e.width(), e.height(), side[0], rc);
		if(d100() < 50)
			create_corridor(e.x1, e.y1, e.width(), e.height(), side[1], rc);
		if(d100() < 10)
			create_corridor(e.x1, e.y1, e.width(), e.height(), side[2], rc);
	}
	for(auto& e : locations) {
		auto r1 = e;
		r1.offset(-1, -1);
		create_doors(r1);
	}
}

static void sort_locations() {
	qsort(locations.data, locations.count, sizeof(locations.data[0]), compare_locations);
}

static void shuffle_locations() {
	zshuffle(locations.data, locations.count);
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
			random(rca.shrink(4, 4)), last_site->floors, last_site->walls);
	} else if(v.iskind<itemi>()) {
		item it; it.clear();
		it.create(bsdata<itemi>::elements + v.value);
		it.drop(random(rca));
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

static void create_floor(variant tile) {
	if(tile.iskind<sitei>()) {
		auto& ei = bsdata<sitei>::elements[tile.value];
		if(ei.floors)
			area.set({0, 0, area.mps - 1, area.mps - 1}, ei.floors);
	}
}

void create_area(const char* id) {
	variant tile = id;
	if(!tile)
		return;
	bsdata<itemground>::source.clear();
	bsdata<creature>::source.clear();
	bsdata<boosti>::source.clear();
	area.clear();
	locations.clear();
	sites.clear();
	create_floor(tile);
	create_landscape({0, 0, area.mps - 1, area.mps - 1}, tile);
	add_area_sites(tile);
	create_location_general();
	//create_city_level({0, 0, area.mps - 1, area.mps - 1}, 0);
	shuffle_locations();
	create_sites();
}