#include "main.h"

BSDATA(sitegeni) = {
	{"GenerateBuilding", &sitei::building},
	{"GenerateCorridors", &sitei::corridors},
	{"GenerateDungeon", &sitei::dungeon},
	{"GenerateOutdoor", &sitei::outdoor},
	{"GenerateRoom", &sitei::room},
	{"GenerateVillage", &sitei::cityscape},
};
BSDATAF(sitegeni)

static adat<point> points;
static adat<rect, 32> locations;
static adat<variant, 32> sites;
static point last_door;
static direction_s last_direction;
static sitei* landscape;
static rect correct = {1, 1, area.mps - 2, area.mps - 2};

const auto minimum_corridor_lenght = 6;
const auto chance_create_door = 10;
const auto chance_corridor_content = 10;
const auto chance_hidden_door = 10;

static point random(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = xrand(rc.x1, rc.x2);
	short y = xrand(rc.y1, rc.y2);
	return {x, y};
}

static rect random(const rect& rca, point offset, point minimum, point maximum) {
	if(rca.x1 > rca.x2 || rca.y1 > rca.y2)
		return {-1000, -1000};
	auto w = rca.width() - offset.x * 2;
	auto h = rca.height() - offset.y * 2;
	rect rc;
	rc.x1 = rca.x1 + offset.x + xrand(0, w - maximum.x);
	rc.x2 = rc.x1 + xrand(minimum.x, maximum.x);
	rc.y1 = rca.y1 + offset.y + xrand(0, h - maximum.y);
	rc.y2 = rc.y1 + xrand(minimum.y, maximum.y);
	return rc;
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
	auto f2 = area.getfeature(to(i, d2));
	return f1 == v || f2 == v;
}

static void create_monster(const rect& rc, const char* id, bool hostile, int count) {
	for(auto i = 0; i < count; i++) {
		auto p = creature::create(random(rc), random_value(id));
		if(hostile)
			p->set(Enemy);
	}
}

static bool isvaliddoor(point m) {
	if(iswall(m, West, East) && !isoneof(m, North, South, Door) && !isonewall(m, North, South))
		return true;
	if(iswall(m, North, South) && !isoneof(m, West, East, Door) && !isonewall(m, West, East))
		return true;
	return false;
}

static void update_doors() {
	point i;
	for(i.y = 0; i.y < area.mps; i.y++) {
		for(i.x = 0; i.x < area.mps; i.x++) {
			if(bsdata<featurei>::elements[area.features[i]].is(BetweenWalls)) {
				if(isvaliddoor(i))
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

static void shuffle_locations() {
	zshuffle(locations.data, locations.count);
}

static void sort_locations() {
	qsort(locations.data, locations.count, sizeof(locations.data[0]), compare_locations);
}

static void remove_trail_locations(size_t count) {
	if(locations.count >= count)
		locations.count -= count;
	else
		locations.count = 0;
}

static roomi* add_room(const sitei* ps, const rect& rc) {
	auto p = new roomi();
	p->clear();
	p->setsite(ps);
	p->rc = rc;
	return p;
}

void sitei::fillfloor(const rect & rc) const {
	area.set(rc, floors);
}

void sitei::fillwalls(const rect & rc) const {
	area.horz(rc.x1, rc.y1, rc.x2, walls);
	area.vert(rc.x1, rc.y1, rc.y2, walls);
	area.horz(rc.x1, rc.y2, rc.x2, walls);
	area.vert(rc.x2, rc.y1, rc.y2, walls);
}

void sitei::building(const rect & rc) const {
	static direction_s rdir[] = {North, South, West, East};
	const auto door = Door;
	// Walls and floors
	fillfloor(rc);
	fillwalls(rc);
	// Doors
	last_direction = maprnd(rdir);;
	last_door = area.getpoint(rc, last_direction);
	area.set(last_door, floors);
	area.set(last_door, door);
	auto m1 = to(last_door, last_direction);
	if(area.iswall(m1))
		area.set(m1, floors);
	area.set(m1, NoFeature);
	add_room(this, rc);
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
	rect r1 = rc;
	if(r1.width() > r1.height()) {
		if(r1.x1 <= 6)
			r1.x1 = 0;
		if(r1.x2 >= area.mps - 6)
			r1.x2 = area.mps - 1;
	} else {
		if(r1.y1 >= 1 && r1.y1 <= 6)
			r1.y1 = 0;
		if(r1.y2 >= area.mps - 6)
			r1.y2 = area.mps - 1;
	}
	area.set(r1, Rock);
	create_monster(r1, "RandomCommoner", false, xrand(3, 6));
}

static void show_debug_minimap() {
	script::run("ExploreArea");
	script::run("ShowMinimap");
}

static void create_city_level(const rect& rc, int level) {
	const int min_building_size = 8;
	const int max_building_size = 16;
	auto w = rc.width();
	auto h = rc.height();
	if(w <= max_building_size && h <= max_building_size) {
		if(w >= min_building_size && h >= min_building_size)
			locations.add({rc.x1 + 1, rc.y1 + 1, rc.x2 - 1, rc.y2 - 1});
		return;
	}
	auto m = xrand(40, 70);
	auto r = (d100() < 50) ? 0 : 1;
	if(w / 3 >= h / 2)
		r = 0;
	else if(h / 3 >= w / 2)
		r = 1;
	auto rd = 2;
	if(level > 2)
		rd = 0;
	if(r == 0) {
		auto w1 = (w * m) / 100; // horizontal
		create_city_level({rc.x1, rc.y1, rc.x1 + w1 - rd - 1, rc.y2}, level + 1);
		create_city_level({rc.x1 + w1, rc.y1, rc.x2, rc.y2}, level + 1);
		if(rd)
			create_road({rc.x1 + w1 - rd, rc.y1, rc.x1 + w1 - 1, rc.y2});
	} else {
		auto h1 = (h * m) / 100; // vertial
		create_city_level({rc.x1, rc.y1, rc.x2, rc.y1 + h1 - rd - 1}, level + 1);
		create_city_level({rc.x1, rc.y1 + h1, rc.x2, rc.y2}, level + 1);
		if(rd)
			create_road({rc.x1, rc.y1 + h1 - rd, rc.x2, rc.y1 + h1 - 1});
	}
}

static bool isallowcorridor(point index, direction_s dir, bool deadend = false) {
	index = to(index, dir); // Forward
	if(!area.is(index, NoTile))
		return false;
	if(!area.is(to(index, round(dir, West)), NoTile))
		return false;
	if(!area.is(to(index, round(dir, East)), NoTile))
		return false;
	index = to(index, dir);
	if(deadend) {
		if(!area.is(index, NoTile))
			return false;
	}
	if(!area.is(to(index, round(dir, West)), NoTile))
		return false;
	if(!area.is(to(index, round(dir, East)), NoTile))
		return false;
	return true;
}

static void create_corridor_content(point m) {
}

static void create_door(point m, tile_s floor, tile_s wall, bool hidden) {
	area.set(m, floor);
	area.set(m, Door);
}

static void create_connector(point index, direction_s dir, tile_s wall, tile_s floor) {
	const auto chance_line_corridor = 60;
	auto count = 0;
	point start = {0, 0};
	while(true) {
		if(!isallowcorridor(index, dir)) {
			if(!start)
				return;
			break;
		}
		auto i0 = to(index, dir);
		area[i0] = floor;
		if(!start) {
			start = i0;
			//if(d100() < chance_create_door)
			//	create_door(i0, d100() < chance_hidden_door);
		} else {
			if(d100() < chance_corridor_content)
				create_corridor_content(i0);
		}
		index = i0;
		if(count >= minimum_corridor_lenght && d100() >= chance_line_corridor)
			break;
		count++;
	}
	//show_debug_minimap();
	direction_s rnd[] = {West, East, North};
	zshuffle(rnd, 3);
	for(auto d : rnd)
		create_connector(index, round(dir, d), wall, floor);
}

static void create_corridor(const rect& rc, direction_s dir, tile_s wall, tile_s floor) {
	auto index = area.getpoint(rc, dir);
	create_connector(index, dir, wall, floor);
}

static void add_possible_door(point m) {
	if(!isvaliddoor(m))
		return;
	points.add(m);
}

static void create_doors(const rect& rc, tile_s floor, tile_s wall) {
	points.clear();
	auto hidden_room = d100() < chance_hidden_door / 2;
	for(short x = rc.x1; x <= rc.x2; x++) {
		add_possible_door({x, (short)(rc.y1 - 1)});
		add_possible_door({x, (short)(rc.y2 + 1)});
	}
	for(short y = rc.y1; y <= rc.y2; y++) {
		add_possible_door({(short)(rc.x1 - 1), y});
		add_possible_door({(short)(rc.x2 + 1), y});
	}
	zshuffle(points.data, points.count);
	auto door_count = xrand(2, 5);
	for(auto m : points) {
		if(!isvaliddoor(m))
			continue;
		if(--door_count < 0)
			break;
		create_door(m, floor, wall, hidden_room);
	}
}

static void place_monsters(const rect & rca, const monsteri & e, int count, bool hostile) {
	if(count < 0)
		count = 0;
	if(count == 0) {
		if(game.isoutdoor())
			count = e.getbase().appear_outdoor.roll();
		else
			count = e.getbase().appear.roll();
	}
	for(auto i = 0; i < count; i++) {
		auto p = creature::create(random(rca), &e);
		if(hostile)
			p->set(Enemy);
	}
}

static void place_character(const rect & rca, const racei& race, const classi& cls) {
	creature::create(random(rca), &race, &cls);
}

static bool test_counter(variant v) {
	if(v.counter < 0 && d100() >= -v.counter)
		return false;
	return true;
}

static void create_landscape(const rect & rca, variant v) {
	static sitei* last_site;
	static racei* last_race;
	if(v.iskind<featurei>())
		area.set(rca, (feature_s)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(rca, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(rca, (mapf_s)v.value, v.counter);
	else if(v.iskind<sitei>()) {
		pushvalue push_site(last_site);
		last_site = bsdata<sitei>::elements + v.value;
		if(last_site->local && last_site->local->proc)
			(last_site->*last_site->local->proc)(rca);
		for(auto ev : bsdata<sitei>::elements[v.value].landscape) {
			rect rc = rca;
			rc.offset(last_site->offset.x, last_site->offset.y);
			create_landscape(rc, ev);
		}
	} else if(v.iskind<monsteri>()) {
		bool hostile = false;
		if(bsdata<monsteri>::elements[v.value].friendly < -20)
			hostile = true;
		place_monsters(rca, bsdata<monsteri>::elements[v.value], v.counter, hostile);
	} else if(v.iskind<shapei>()) {
		if(!last_site || !test_counter(v))
			return;
		place_shape(bsdata<shapei>::elements[v.value],
			random(rca.shrink(4, 4)), last_site->floors, last_site->walls);
	} else if(v.iskind<racei>())
		last_race = bsdata<racei>::elements + v.value;
	else if(v.iskind<classi>()) {
		if(!last_race)
			return;
		place_character(rca, *last_race, bsdata<classi>::elements[v.value]);
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
		} else {
			auto count = v.counter;
			if(!count)
				count = 1;
			for(auto i = 0; i < count; i++)
				sites.add(v);
		}
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

static bool apply_landscape(variant tile) {
	landscape = tile;
	return landscape != 0;
}

void sitei::cityscape(const rect& rca) const {
	fillfloor(rca);
	create_landscape(rca, this);
	rect r1 = rca; r1.offset(offset.x, offset.y);
	create_city_level(r1, 0);
	sort_locations();
}

void sitei::outdoor(const rect& rca) const {
	fillfloor(rca);
	create_landscape(rca, this);
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
	shuffle_locations();
}

void sitei::room(const rect & rc) const {
	fillfloor(rc);
	add_room(this, rc);
}

void sitei::dungeon(const rect& rca) const {
	const auto parts = 4;
	const auto size = area.mps / parts;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = (i % parts) * size;
		rc.y1 = (i / parts) * size;
		rc.x2 = rc.x1 + size;
		rc.y2 = rc.y1 + size;
		auto r1 = random(rc, offset, {5, 5}, {10, 10});
		locations.add(r1);
	}
	sort_locations();
	remove_trail_locations(2);
	shuffle_locations();
}

direction_s getmost(const rect& rc) {
	auto x1 = rc.x1;
	auto x2 = area.mps - 1 - rc.x2;
	auto y1 = rc.y1;
	auto y2 = area.mps - 1 - rc.y2;
	if(imax(x1, x2) > imax(y1, y2)) {
		if(x1 > x2)
			return West;
		return East;
	} else {
		if(y1 > y2)
			return North;
		return South;
	}
}

static void create_doors(tile_s floor, tile_s wall) {
	for(auto& rc : locations)
		create_doors(rc, floor, wall);
}

void sitei::corridors(const rect& rca) const {
	static direction_s connectors_side[] = {North, East, West, South};
	const auto& rc = locations[0];
	auto dir = getmost(rc);
	create_corridor(rc, dir, walls, floors);
	area.change(NoTile, walls);
	create_doors(floors, walls);
}

void create_area(variant tile) {
	static rect all = {0, 0, area.mps - 1, area.mps - 1};
	if(!apply_landscape(tile))
		return;
	bsdata<itemground>::source.clear();
	bsdata<creature>::source.clear();
	bsdata<boosti>::source.clear();
	loc.clear();
	area.clear();
	locations.clear();
	sites.clear();
	loc.settile(landscape->id);
	loc.darkness = landscape->darkness;
	add_area_sites(tile);
	if(landscape->global)
		(landscape->*landscape->global->proc)(all);
	else
		landscape->outdoor(all);
	create_sites();
	if(landscape->global_finish)
		(landscape->*landscape->global_finish->proc)(all);
	update_doors();
}