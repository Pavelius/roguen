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

static adat<point, 512> points;
static adat<rect, 32> locations;
static adat<variant, 32> sites;
static point last_door;
static direction_s last_direction;
static sitei* landscape;
static dungeon* last_dungeon;
static condition_s last_modifier;
static rect correct_conncetors;

const auto chance_corridor_content = 10;
const auto chance_hidden_door = 10;

static point random(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	short x = xrand(rc.x1, rc.x2);
	short y = xrand(rc.y1, rc.y2);
	return {x, y};
}

static rect center_rect(const rect& rc) {
	if(rc.x1 > rc.x2 || rc.y1 > rc.y2)
		return {-1000, -1000};
	auto x = rc.x1 + rc.width() / 2;
	auto y = rc.y1 + rc.height() / 2;
	return {x, y, x, y};
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
	*static_cast<geoposition*>(p) = game;
	p->rc = rc;
	return p;
}

void sitei::fillfloor(rect & rc) const {
	area.set(rc, floors);
}

void sitei::fillwalls(rect & rc) const {
	area.horz(rc.x1, rc.y1, rc.x2, walls);
	area.vert(rc.x1, rc.y1, rc.y2, walls);
	area.horz(rc.x1, rc.y2, rc.x2, walls);
	area.vert(rc.x2, rc.y1, rc.y2, walls);
}

void sitei::building(rect& rc) const {
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
	rc.offset(1, 1);
}

static void place_shape(const shapei& e, point m, direction_s d, tile_s floor, tile_s wall) {
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

static void create_monster(const rect& rca, variant v, int count) {
	if(count < 0) {
		if(d100() >= (-count))
			return;
		count = 0;
	}
	if(count == 0) {
		if(v.iskind<monsteri>())
			count = bsdata<monsteri>::elements[v.value].appear.roll();
		else
			count = xrand(2, 5);
		if(!count)
			count = 1;
	}
	for(auto i = 0; i < count; i++)
		creature::create(random(rca), single(v));
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
	create_monster(r1, "RandomCommoner", xrand(3, 6));
}

static void show_debug_minimap() {
	script::run("ExploreArea");
	script::run("ShowMinimap");
}

static void create_city_level(const rect& rc, int level) {
	const int min_building_size = 7;
	const int max_building_size = 12;
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
	if(!index.in(correct_conncetors))
		return false;
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

static void create_door(point m, tile_s floor, tile_s wall, bool hidden) {
	area.set(m, floor);
	area.set(m, Door);
}

static void place_item(point index, const itemi* pe) {
	if(!pe || pe == bsdata<itemi>::elements)
		return;
	item it; it.clear();
	it.create(pe);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	it.drop(index);
}

static void create_connector(point index, direction_s dir, tile_s wall, tile_s floor, int& count) {
	const auto chance_line_corridor = 60;
	const auto minimum_corridor_lenght = 6;
	count = 0;
	point start = {-1000, -1000};
	while(true) {
		if(!isallowcorridor(index, dir)) {
			if(!area.isvalid(start))
				return;
			break;
		}
		auto i0 = to(index, dir);
		area[i0] = floor;
		if(!area.isvalid(start))
			start = i0;
		index = i0;
		if(count >= minimum_corridor_lenght && d100() >= chance_line_corridor)
			break;
		count++;
	}
	//show_debug_minimap();
	direction_s rnd[] = {West, East, North};
	zshuffle(rnd, 3);
	auto next_count = 0;
	for(auto d : rnd)
		create_connector(index, round(dir, d), wall, floor, next_count);
	if(!next_count)
		points.add(index);
}

static void create_corridor(const rect& rc, direction_s dir, tile_s wall, tile_s floor) {
	points.clear();
	auto index = area.getpoint(rc, dir);
	auto count = 0;
	create_connector(index, dir, wall, floor, count);
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

static void place_items(const rect& rca, const itemi& e, int count) {
	if(&e == bsdata<itemi>::elements)
		return;
	if(count < 0)
		count = 0;
	for(auto i = 0; i < count; i++) {
		item it; it.clear();
		it.create(&e);
		it.drop(random(rca));
	}
}

static void place_character(const rect& rca, const racei& race, const classi& cls) {
	creature::create(random(rca), &race, &cls);
}

static bool test_counter(variant v) {
	if(v.counter < 0 && d100() >= -v.counter)
		return false;
	return true;
}

static void create_landscape(const rect& rca, variant v) {
	static sitei* last_site;
	static racei* last_race;
	v = single(v);
	if(!v)
		return;
	if(v.iskind<featurei>())
		area.set(rca, (feature_s)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(rca, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(rca, (mapf_s)v.value, v.counter);
	else if(v.iskind<sitei>()) {
		pushvalue push_site(last_site);
		pushvalue push_race(last_race);
		last_site = bsdata<sitei>::elements + v.value;
		rect rc = rca;
		if(last_site->local && last_site->local->proc)
			(last_site->*last_site->local->proc)(rc);
		rc.offset(last_site->offset.x, last_site->offset.y);
		if(!last_site->sites)
			add_room(last_site, rc);
		for(auto ev : bsdata<sitei>::elements[v.value].landscape)
			create_landscape(rc, ev);
	} else if(v.iskind<monsteri>())
		create_monster(rca, v, v.counter);
	else if(v.iskind<listi>()) {
		for(auto v : bsdata<listi>::elements[v.value].elements)
			create_landscape(rca, v);
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
	} else if(v.iskind<itemi>())
		place_items(rca, bsdata<itemi>::elements[v.value], v.counter);
}

static void add_area_sites(variant v) {
	if(!v)
		return;
	if(!test_counter(v))
		return;
	auto count = v.counter;
	if(!count)
		count = 1;
	if(v.iskind<sitei>()) {
		auto& ei = bsdata<sitei>::elements[v.value];
		if(ei.sites) {
			for(auto ev : ei.sites)
				add_area_sites(ev);
		} else {
			for(auto i = 0; i < count; i++)
				sites.add(v.nocounter());
		}
	} else if(v.iskind<randomizeri>()) {
		for(auto i = 0; i < count; i++)
			add_area_sites(single(v));
	} else if(v.iskind<monsteri>())
		sites.add(v);
}

static void add_area_events(geoposition v) {
	if(!last_dungeon)
		return;
	if(v.level == 0)
		add_area_sites(last_dungeon->entrance);
	else
		add_area_sites(last_dungeon->modifier);
}

static void add_area_landscapes(const rect rca) {
	if(!last_dungeon)
		return;
	for(auto v : last_dungeon->modifier->landscape)
		create_landscape(rca, v);
}

static void create_sites() {
	if(sites.count < locations.count)
		locations.count = sites.count;
	if(sites.count > locations.count)
		sites.count = locations.count;
	for(size_t i = 0; i < locations.count; i++) {
		add_area_landscapes(locations.data[i]);
		create_landscape(locations.data[i], sites.data[i]);
	}
}

static bool apply_landscape(variant tile) {
	landscape = tile;
	return landscape != 0;
}

void sitei::cityscape(rect& rca) const {
	fillfloor(rca);
	create_landscape(rca, this);
	rca.offset(offset.x, offset.y);
	create_city_level(rca, 0);
	sort_locations();
}

void sitei::outdoor(rect& rca) const {
	fillfloor(rca);
	create_landscape(rca, this);
	rca.offset(offset.x, offset.y);
	const auto parts = 4;
	const auto size = rca.width() / parts;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = rca.x1 + (i % parts) * size + 1;
		rc.y1 = rca.y1 + (i / parts) * size + 1;
		rc.x2 = rc.x1 + size - 2;
		rc.y2 = rc.y1 + size - 2;
		locations.add(rc);
	}
	shuffle_locations();
}

void sitei::room(rect& rc) const {
	fillfloor(rc);
}

static rect bounding_locations() {
	rect rc = {1000, 1000, 0, 0};
	for(auto& e : locations) {
		if(rc.x1 > e.x1)
			rc.x1 = e.x1;
		if(rc.y1 > e.y1)
			rc.y1 = e.y1;
		if(rc.x2 < e.x2)
			rc.x2 = e.x2;
		if(rc.y2 < e.y2)
			rc.y2 = e.y2;
	}
	return rc;
}

void sitei::dungeon(rect& rca) const {
	rca.offset(offset.x, offset.y);
	auto parts = 4;
	if(rca.width() <= 48)
		parts = 3;
	auto size = rca.width() / parts;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = rca.x1 + (i % parts) * size;
		rc.y1 = rca.y1 + (i / parts) * size;
		rc.x2 = rc.x1 + size;
		rc.y2 = rc.y1 + size;
		auto r1 = random(rc, {2, 2}, {5, 3}, {8, 6});
		locations.add(r1);
	}
	sort_locations();
	remove_trail_locations(xrand(1, 3));
	shuffle_locations();
	correct_conncetors = bounding_locations();
	correct_conncetors.offset(6, 4);
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

static void create_corridor_content(point i) {
	place_item(i, bsdata<itemi>::find("SP"));
}

static void create_corridor_contents() {
	zshuffle(points.data, points.count);
	auto count = xrand(10, 30);
	if(count > (int)points.count)
		count = points.count;
	for(auto i = 0; i < count; i++)
		create_corridor_content(points.data[i]);
}

void sitei::corridors(rect& rca) const {
	static direction_s connectors_side[] = {North, East, West, South};
	const auto& rc = locations[0];
	auto dir = getmost(rc);
	create_corridor(rc, dir, walls, floors);
	area.change(NoTile, walls);
	create_corridor_contents();
	create_doors(floors, walls);
}

static void create_area(geoposition geo, variant tile) {
	if(!apply_landscape(tile))
		return;
	bsdata<itemground>::source.clear();
	loc.clear();
	area.clear();
	locations.clear();
	sites.clear();
	loc.setsite(landscape);
	loc.darkness = landscape->darkness;
	add_area_events(geo);
	add_area_sites(tile);
	rect all = {0, 0, area.mps - 1, area.mps - 1};
	if(landscape->global)
		(landscape->*landscape->global->proc)(all);
	else
		landscape->outdoor(all);
	create_sites();
	if(landscape->global_finish)
		(landscape->*landscape->global_finish->proc)(all);
	update_doors();
}

variant get_dungeon_site(point v) {
	if(!last_dungeon || !last_dungeon->level)
		return single("DefaultDungeon");
	return last_dungeon->level;
}

void gamei::createarea() {
	variant rt;
	last_dungeon = dungeon::find(position);
	if(level == 0) {
		auto range = getrange(position, start_village);
		if(range == 0)
			rt = single("StartVillage");
		else if(range <= 2)
			rt = single("RandomNearestOverlandTiles");
		else if(range <= 5)
			rt = single("RandomOverlandTiles");
		else if(range <= 9)
			rt = single("RandomFarOverlandTiles");
		else
			rt = single("RandomUnknownOverlandTiles");
	} else
		rt = get_dungeon_site(position);
	if(!rt)
		rt = single("LightForest");
	create_area(*this, rt);
}