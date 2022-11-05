#include "main.h"

BSDATA(sitegeni) = {
	{"NoGenerate", &sitei::nogenerate},
	{"GenerateBuilding", &sitei::building},
	{"GenerateCorridors", &sitei::corridors},
	{"GenerateDungeon", &sitei::dungeon},
	{"GenerateOutdoor", &sitei::outdoor},
	{"GenerateRoom", &sitei::room},
	{"GenerateWalls", &sitei::fillwallsall},
	{"GenerateVillage", &sitei::cityscape},
};
BSDATAF(sitegeni)

static adat<point, 512> points;
static adat<rect, 32> locations;
static adat<variant, 32> sites;
static point last_door;
static direction_s last_direction;
static condition_s last_modifier;
static rect correct_conncetors;
static const sitegeni* last_method;

const auto chance_corridor_content = 10;
const auto chance_hidden_door = 10;

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

static bool isvaliddoornt(point m) {
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

void sitei::fillfloor() const {
	auto n = floors;
	if(!n && last_location->floors)
		n = last_location->floors;
	area.set(last_rect, n);
}

void sitei::fillwallsall() const {
	auto n = walls;
	if(!n && last_location->walls)
		n = last_location->walls;
	area.set(last_rect, n);
}

void sitei::fillwalls() const {
	area.horz(last_rect.x1, last_rect.y1, last_rect.x2, walls);
	area.vert(last_rect.x1, last_rect.y1, last_rect.y2, walls);
	area.horz(last_rect.x1, last_rect.y2, last_rect.x2, walls);
	area.vert(last_rect.x2, last_rect.y1, last_rect.y2, walls);
}

void sitei::building() const {
	static direction_s rdir[] = {North, South, West, East};
	const auto door = Door;
	// Walls and floors
	fillfloor();
	fillwalls();
	// Doors
	last_direction = maprnd(rdir);;
	last_door = area.getpoint(last_rect, last_direction);
	area.set(last_door, floors);
	area.set(last_door, door);
	auto m1 = to(last_door, last_direction);
	if(area.iswall(m1))
		area.set(m1, floors);
	area.set(m1, NoFeature);
	last_rect.offset(1, 1);
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

static void create_monster(variant v, int count) {
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
		creature::create(area.get(last_rect), single(v));
}

static void create_road(const rect& rc) {
	pushvalue push_rect(last_rect, rc);
	if(last_rect.width() > last_rect.height()) {
		if(last_rect.x1 <= 6)
			last_rect.x1 = 0;
		if(last_rect.x2 >= area.mps - 6)
			last_rect.x2 = area.mps - 1;
	} else {
		if(last_rect.y1 >= 1 && last_rect.y1 <= 6)
			last_rect.y1 = 0;
		if(last_rect.y2 >= area.mps - 6)
			last_rect.y2 = area.mps - 1;
	}
	area.set(last_rect, Rock);
	create_monster("RandomCommoner", xrand(3, 6));
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

static bool isallowconnector(point index, direction_s dir, bool deadend = false) {
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

static bool isallowcorridor(point index, direction_s dir, bool linkable = false) {
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
	if(linkable) {
		auto pr = roomi::find(game, index);
		if(pr)
			return true;
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

void place_item(point index, const itemi* pe) {
	if(!pe || pe == bsdata<itemi>::elements)
		return;
	if(area.iswall(index))
		return;
	item it; it.clear();
	it.create(pe);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	it.drop(index);
}

static void create_connector(point index, direction_s dir, tile_s wall, tile_s floor, int& count, bool linkable) {
	const auto chance_line_corridor = 60;
	const auto minimum_corridor_lenght = 4;
	count = 0;
	point start = {-1000, -1000};
	while(true) {
		if(!isallowcorridor(index, dir, linkable)) {
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
		create_connector(index, round(dir, d), wall, floor, next_count, linkable);
	if(!next_count)
		points.add(index);
}

static void create_corridor(const rect& rc, direction_s dir, tile_s wall, tile_s floor) {
	points.clear();
	auto index = area.getpoint(rc, dir);
	auto count = 0;
	create_connector(index, dir, wall, floor, count, false);
}

static int add_possible_door(point m, bool run) {
	if(!isvaliddoor(m))
		return 0;
	if(run)
		points.add(m);
	return 1;
}

static int add_possible_doors(const rect& rc, bool run) {
	auto result = 0;
	for(short x = rc.x1; x <= rc.x2; x++) {
		result += add_possible_door({x, (short)(rc.y1 - 1)}, run);
		result += add_possible_door({x, (short)(rc.y2 + 1)}, run);
	}
	for(short y = rc.y1; y <= rc.y2; y++) {
		result += add_possible_door({(short)(rc.x1 - 1), y}, run);
		result += add_possible_door({(short)(rc.x2 + 1), y}, run);
	}
	return result;
}

static void create_doors(const rect& rc, tile_s floor, tile_s wall) {
	points.clear();
	auto hidden_room = d100() < chance_hidden_door / 2;
	add_possible_doors(rc, true);
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

static void place_item(const itemi& e, int count) {
	if(count < 0) {
		if(d100() >= -count)
			return;
		count = 0;
	}
	if(count == 0)
		count = 1;
	for(auto i = 0; i < count; i++)
		place_item(area.get(last_rect), &e);
}

static void place_character(const racei& race, const classi& cls) {
	creature::create(area.get(last_rect), &race, &cls);
}

void create_landscape(variant v) {
	static racei* last_race;
	if(!v)
		return;
	else if(v.iskind<featurei>())
		area.set(last_rect, (feature_s)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(last_rect, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(last_rect, (mapf_s)v.value, v.counter);
	else if(v.iskind<sitei>()) {
		pushvalue push_site(last_site);
		pushvalue push_race(last_race);
		pushvalue push_rect(last_rect);
		pushvalue push_method(last_method);
		last_site = bsdata<sitei>::elements + v.value;
		if(last_site->local)
			last_method = last_site->local;
		if(last_method)
			(last_site->*last_method->proc)();
		if(!last_site->sites)
			add_room(last_site, last_rect);
		for(auto ev : bsdata<sitei>::elements[v.value].landscape)
			create_landscape(ev);
	} else if(v.iskind<monsteri>())
		create_monster(v, v.counter);
	else if(v.iskind<listi>()) {
		for(auto v : bsdata<listi>::elements[v.value].elements)
			create_landscape(v);
	} else if(v.iskind<shapei>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		if(!last_site)
			return;
		place_shape(bsdata<shapei>::elements[v.value],
			area.get(last_rect.shrink(4, 4)), last_site->floors, last_site->walls);
	} else if(v.iskind<racei>())
		last_race = bsdata<racei>::elements + v.value;
	else if(v.iskind<classi>()) {
		if(!last_race)
			return;
		place_character(*last_race, bsdata<classi>::elements[v.value]);
	} else if(v.iskind<itemi>())
		place_item(bsdata<itemi>::elements[v.value], v.counter);
	else if(v.iskind<script>()) {
		pushvalue push_result(last_variant, {});
		bsdata<script>::elements[v.value].run(v.counter);
		if(last_variant)
			create_landscape(last_variant);
	} else if(v.iskind<randomizeri>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			create_landscape(bsdata<randomizeri>::elements[v.value].random());
	}
}

static void add_area_sites(variant v) {
	if(!v)
		return;
	auto count = game.getcount(v);
	if(count <= 0)
		return;
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

static void create_sites() {
	if(sites.count < locations.count)
		locations.count = sites.count;
	if(sites.count > locations.count)
		sites.count = locations.count;
	for(size_t i = 0; i < locations.count; i++) {
		pushvalue push_rect(last_rect, locations.data[i]);
		create_landscape(sites.data[i]);
	}
}

static bool apply_landscape(geoposition geo, variant tile) {
	last_location = tile;
	if(!last_location)
		return false;
	areahead.setsite(last_location);
	areahead.darkness = areahead.getsite()->darkness;
	return true;
}

void sitei::cityscape() const {
	fillfloor();
	create_landscape(this);
	last_rect.offset(2, 2);
	create_city_level(last_rect, 0);
	sort_locations();
}

void sitei::outdoor() const {
	fillfloor();
	create_landscape(this);
	const auto parts = 4;
	const auto size = last_rect.width() / parts;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = last_rect.x1 + (i % parts) * size + 1;
		rc.y1 = last_rect.y1 + (i / parts) * size + 1;
		rc.x2 = rc.x1 + size - 2;
		rc.y2 = rc.y1 + size - 2;
		locations.add(rc);
	}
	shuffle_locations();
}

void sitei::room() const {
	fillfloor();
}

static rect bounding_locations() {
	rect rc = {1000, 1000, 0, 0};
	for(auto& e : locations) {
		auto pt = center(e);
		if(rc.x1 > pt.x)
			rc.x1 = pt.x;
		if(rc.y1 > pt.y)
			rc.y1 = pt.y;
		if(rc.x2 < pt.x)
			rc.x2 = pt.x;
		if(rc.y2 < pt.y)
			rc.y2 = pt.y;
	}
	return rc;
}

void sitei::dungeon() const {
	last_rect.offset(2, 2);
	auto parts = 4;
	if(last_rect.width() <= 48)
		parts = 3;
	if(last_rect.width() <= 32)
		parts = 2;
	auto size = last_rect.width() / parts;
	for(auto i = 0; i < parts * parts; i++) {
		rect rc;
		rc.x1 = last_rect.x1 + (i % parts) * size;
		rc.y1 = last_rect.y1 + (i / parts) * size;
		rc.x2 = rc.x1 + size;
		rc.y2 = rc.y1 + size;
		auto r1 = random(rc, {2, 2}, {5, 3}, {8, 6});
		locations.add(r1);
	}
	shuffle_locations();
	correct_conncetors = bounding_locations();
}

static direction_s findvalid(point i, tile_s t) {
	static direction_s source[] = {North, South, East, West};
	for(auto d : source) {
		auto i1 = to(i, d);
		if(!area.isvalid(i1))
			continue;
		if(area[i1] == t)
			return d;
	}
	return North;
}

static void additional_corridors(tile_s floor, tile_s wall) {
	for(auto& rc : locations) {
		//auto m = area.getpoint(rc, area.getmost(rc));
		//if(!isvaliddoor(m))
		//	continue;
		//int result_count = 0;
		//create_connector(m, findvalid(m, NoTile), wall, floor, result_count, true);
		//if(result_count)
		//	break;
	}
}

static void create_doors(tile_s floor, tile_s wall) {
	for(auto& rc : locations)
		create_doors(rc, floor, wall);
}

static void create_corridor_content(point i) {
	variant treasure = "RandomTreasure";
	if(last_location && last_location->loot && d100() < 40)
		treasure = randomizeri::random(last_location->loot);
	else if(last_dungeon && last_dungeon->modifier && last_dungeon->modifier->loot && d100() < 40)
		treasure = randomizeri::random(last_dungeon->modifier->loot);
	pushvalue push_rect(last_rect, {i.x, i.y, i.x, i.y});
	create_landscape(treasure);
}

static void create_corridor_contents() {
	zshuffle(points.data, points.count);
	auto count = xrand(10, 30);
	if(count > (int)points.count)
		count = points.count;
	for(auto i = 0; i < count; i++)
		create_corridor_content(points.data[i]);
}

void sitei::corridors() const {
	const auto& rc = locations[0];
	auto dir = area.getmost(rc);
	create_corridor(rc, dir, walls, floors);
	additional_corridors(floors, walls);
	area.change(NoTile, walls);
	create_corridor_contents();
	create_doors(floors, walls);
}

static void update_dungeon_rumor() {
	if(last_dungeon)
		return;
	auto i = area.find(StairsDown);
	if(!area.isvalid(i))
		return;
	// Add random rumor
	dungeon::add(game.position);
}

static void create_area(geoposition geo, variant tile) {
	areahead.clear();
	if(!apply_landscape(geo, tile))
		return;
	bsdata<itemground>::source.clear();
	area.clear();
	locations.clear();
	sites.clear();
	add_area_events(geo);
	add_area_sites(tile);
	rect all = {0, 0, area.mps - 1, area.mps - 1};
	pushvalue push_rect(last_rect, all);
	pushvalue push_method(last_method, last_location->local);
	if(last_location->global)
		(last_location->*last_location->global->proc)();
	else
		last_location->outdoor();
	last_rect = push_rect;
	create_sites();
	last_rect = push_rect;
	if(last_location->global_finish)
		(last_location->*last_location->global_finish->proc)();
	update_doors();
	update_dungeon_rumor();
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
	} else {
		if(last_dungeon) {
			auto chance_finale = level * 10;
			if(last_dungeon->final_level && d100() < (last_dungeon->final_level->chance_finale + chance_finale))
				rt = last_dungeon->final_level;
			else if(last_dungeon->level)
				rt = last_dungeon->level;
			else
				rt = single("DefaultDungeon");
		}
		if(!rt)
			rt = single("DefaultDungeon");
	}
	if(!rt)
		rt = single("LightForest");
	create_area(*this, rt);
}