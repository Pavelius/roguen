#include "areamap.h"
#include "direction.h"
#include "creature.h"
#include "game.h"
#include "pushvalue.h"
#include "randomizer.h"
#include "script.h"
#include "site.h"

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

extern areamap area;
extern areaheadi areahead;
extern rect last_rect;
extern locationi* last_location;
extern const sitei* last_site;
extern roomi* last_room;

static adat<rect, 32> locations;
static adat<point, 512> points;

static point last_door;
static rect correct_conncetors;
static direction_s last_direction;
static adat<variant, 32> sites;

static int getrange(point m1, point m2) {
	auto dx = iabs(m1.x - m2.x);
	auto dy = iabs(m1.y - m2.y);
	return (dx > dy) ? dx : dy;
}

int getfloor() {
	if(last_site && last_site->floors)
		return last_site->floors;
	if(last_location && last_location->floors)
		return last_location->floors;
	return 0;
}

int getwall() {
	if(last_site && last_site->walls)
		return last_site->walls;
	if(last_location && last_location->walls)
		return last_location->walls;
	return 0;
}

static int getdoor() {
	if(last_site && last_site->doors)
		return last_site->doors;
	if(last_location && last_location->doors)
		return last_location->doors;
	return 0;
}

static int get_chance_hidden_doors() {
	if(last_site && last_site->chance_hidden_doors)
		return last_site->chance_hidden_doors;
	if(last_location && last_location->chance_hidden_doors)
		return last_location->chance_hidden_doors;
	return 10;
}

static int get_chance_locked_doors() {
	if(last_site && last_site->chance_locked_doors)
		return last_site->chance_locked_doors;
	if(last_location && last_location->chance_locked_doors)
		return last_location->chance_locked_doors;
	return 0;
}

static int get_chance_stuck_doors() {
	if(last_site && last_site->chance_stuck_doors)
		return last_site->chance_stuck_doors;
	if(last_location && last_location->chance_stuck_doors)
		return last_location->chance_stuck_doors;
	return 5;
}

static int get_doors_count() {
	if(last_site && last_site->doors_count)
		return last_site->doors_count;
	if(last_location && last_location->doors_count)
		return last_location->doors_count;
	return xrand(1, 4);
}

static bool iswall(point i, direction_s d1, direction_s d2) {
	return area.iswall(i, d1)
		&& area.iswall(i, d2);
}

static bool isonewall(point i, direction_s d1, direction_s d2) {
	return area.iswall(i, d1)
		|| area.iswall(i, d2);
}

static bool isoneofdoor(point i, direction_s d1, direction_s d2) {
	if(area.getfeature(to(i, d1)).is(BetweenWalls))
		return true;
	if(area.getfeature(to(i, d2)).is(BetweenWalls))
		return true;
	return false;
}

static bool isvaliddoor(point m) {
	if(iswall(m, West, East) && !isoneofdoor(m, North, South) && !isonewall(m, North, South))
		return true;
	if(iswall(m, North, South) && !isoneofdoor(m, West, East) && !isonewall(m, West, East))
		return true;
	return false;
}

static bool isvaliddoornt(point m) {
	if(iswall(m, West, East) && !isoneofdoor(m, North, South) && !isonewall(m, North, South))
		return true;
	if(iswall(m, North, South) && !isoneofdoor(m, West, East) && !isonewall(m, West, East))
		return true;
	return false;
}

static void update_doors() {
	point i;
	for(i.y = 0; i.y < area.mps; i.y++) {
		for(i.x = 0; i.x < area.mps; i.x++) {
			if(bsdata<featurei>::elements[int(area.features[i])].is(BetweenWalls)) {
				if(isvaliddoor(i))
					continue;
				area.setfeature(i, 0);
			}
		}
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

static void create_road(const rect& rc) {
	pushvalue push_rect(last_rect, rc);
	variant road = "Rock";
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
	area.set(last_rect, &areamap::settile, road.value);
	variant v = "RandomCommoner";
	v.counter = xrand(3, 6);
	script::run(v);
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
	if(!area.istile(index, 0))
		return false;
	if(!area.istile(to(index, round(dir, West)), 0))
		return false;
	if(!area.istile(to(index, round(dir, East)), 0))
		return false;
	index = to(index, dir);
	if(deadend) {
		if(!area.istile(index, 0))
			return false;
	}
	if(!area.istile(to(index, round(dir, West)), 0))
		return false;
	if(!area.istile(to(index, round(dir, East)), 0))
		return false;
	return true;
}

static bool isallowcorridor(point index, direction_s dir, bool linkable = false) {
	index = to(index, dir); // Forward
	if(!index.in(correct_conncetors))
		return false;
	if(!area.istile(index, 0))
		return false;
	if(!area.istile(to(index, round(dir, West)), 0))
		return false;
	if(!area.istile(to(index, round(dir, East)), 0))
		return false;
	index = to(index, dir);
	if(linkable) {
		auto pr = roomi::find(areahead, index);
		if(pr)
			return true;
	}
	if(!area.istile(to(index, round(dir, West)), 0))
		return false;
	if(!area.istile(to(index, round(dir, East)), 0))
		return false;
	return true;
}

static void create_door(point m, int floor, int wall, bool hidden, bool locked, bool stuck) {
	const auto& door = bsdata<featurei>::elements[getdoor()];
	if(!door.isvisible())
		return;
	if(hidden) {
		area.settile(m, wall);
		auto ph = door.gethidden();
		if(ph)
			area.setfeature(m, ph - bsdata<featurei>::elements);
		areahead.total.doors_hidden++;
	} else if(locked) {
		area.settile(m, floor);
		auto ph = door.getlocked();
		if(ph)
			area.setfeature(m, ph - bsdata<featurei>::elements);
		areahead.total.doors_locked++;
	} else if(stuck) {
		area.settile(m, floor);
		auto ph = door.getstuck();
		if(ph)
			area.setfeature(m, ph - bsdata<featurei>::elements);
		areahead.total.doors_stuck++;
	} else {
		area.settile(m, floor);
		area.setfeature(m, &door - bsdata<featurei>::elements);
	}
	areahead.total.doors++;
}

static void create_connector(point index, direction_s dir, int wall, int floor, int& count, bool linkable) {
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
		area.tiles[i0] = floor;
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

static void create_corridor(const rect& rc, direction_s dir, int wall, int floor) {
	points.clear();
	auto index = area.getpoint(rc, correct_conncetors, dir);
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

static void create_doors(const rect& rc, int floor, int wall) {
	pushvalue push_room(last_room, roomi::find(areahead, center(rc)));
	pushvalue push_site(last_site, last_room ? &last_room->geti() : 0);
	auto door_count = get_doors_count();
	auto chance_hidden_doors = get_chance_hidden_doors();
	auto chance_locked_doors = get_chance_locked_doors();
	auto chance_stuck_doors = get_chance_stuck_doors();
	points.clear();
	auto hidden_room = d100() < chance_hidden_doors;
	auto locked_room = d100() < chance_locked_doors;
	auto stuck_room = d100() < chance_stuck_doors;
	add_possible_doors(rc, true);
	zshuffle(points.data, points.count);
	for(auto m : points) {
		if(!isvaliddoor(m))
			continue;
		if(--door_count < 0)
			break;
		create_door(m, floor, wall, hidden_room, locked_room, stuck_room);
	}
	if(last_room && hidden_room)
		areahead.total.rooms_hidden++;
}

static direction_s findvalid(point i, int t) {
	static direction_s source[] = {North, South, East, West};
	for(auto d : source) {
		auto i1 = to(i, d);
		if(!area.isvalid(i1))
			continue;
		if(area.tiles[i1] == t)
			return d;
	}
	return North;
}

static void create_doors(int floor, int wall) {
	for(auto& rc : locations)
		create_doors(rc, floor, wall);
}

roomi* add_room(const sitei* ps, const rect& rc) {
	auto p = roomi::add();
	p->clear();
	p->set(ps);
	*static_cast<geoposition*>(p) = areahead;
	p->rc = rc;
	return p;
}

static void place_shape(const shapei& e, point m, direction_s d, int floor, int wall) {
	auto c = e.center(m);
	for(m.y = 0; m.y < e.size.y; m.y++) {
		for(m.x = 0; m.x < e.size.x; m.x++) {
			auto pm = e.translate(c, m, d);
			auto sm = e[m];
			switch(sm) {
			case ' ':
				break;
			case '.':
				area.settile(pm, floor);
				break;
			case 'X':
				area.settile(pm, wall);
				break;
			case '0':
				area.settile(pm, floor);
				break;
			default:
				break;
			}
		}
	}
}

void place_shape(const shapei& e, point m, int floor, int walls) {
	static direction_s direction[] = {North, South, West, East};
	place_shape(e, m, maprnd(direction), floor, walls);
}

static void show_debug_minimap() {
	script::run("ExploreArea");
	script::run("ShowMinimap");
}

static void add_area_sites(variant v) {
	if(!v)
		return;
	auto count = game.getcount(v);
	if(count <= 0)
		return;
	if(v.iskind<locationi>()) {
		auto& ei = bsdata<locationi>::elements[v.value];
		for(auto ev : ei.sites)
			add_area_sites(ev);
	} else if(v.iskind<sitei>()) {
		auto& ei = bsdata<sitei>::elements[v.value];
		for(auto i = 0; i < count; i++)
			sites.add(v.nocounter());
	} else if(v.iskind<randomizeri>()) {
		for(auto i = 0; i < count; i++)
			add_area_sites(single(v));
	}
}

static void add_area_events(geoposition v) {
	if(!last_quest)
		return;
	if(v.level == 0) {
		auto v = bsdata<sitei>::find(str("%1Entrance", last_quest->level.getid()));
		if(v)
			add_area_sites(v);
	} else
		add_area_sites(last_quest->modifier);
}

static void create_sites() {
	if(sites.count < locations.count)
		locations.count = sites.count;
	if(sites.count > locations.count)
		sites.count = locations.count;
	for(size_t i = 0; i < locations.count; i++) {
		pushvalue push_rect(last_rect, locations.data[i]);
		script::run(sites.data[i]);
	}
}

static bool apply_location(geoposition geo, variant tile) {
	last_location = tile;
	if(!last_location)
		return false;
	areahead.setloc(last_location);
	areahead.darkness = last_location->darkness;
	return true;
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

static void create_corridor_content(point i) {
	variant treasure = "RandomLoot";
	locationi* quest_modifier = (last_quest && last_quest->modifier) ? (locationi*)last_quest->modifier : 0;
	if(last_location && last_location->loot && d100() < 40)
		treasure = randomizeri::random(last_location->loot);
	else if(quest_modifier && quest_modifier->loot && d100() < 40)
		treasure = randomizeri::random(quest_modifier->loot);
	pushvalue push_rect(last_rect, {i.x, i.y, i.x, i.y});
	script::run(treasure);
	areahead.total.loots++;
}

static void create_corridor_contents() {
	zshuffle(points.data, points.count);
	auto count = xrand(10, 30);
	if(count > (int)points.count)
		count = points.count;
	for(auto i = 0; i < count; i++)
		create_corridor_content(points.data[i]);
}

static void create_area(geoposition geo, variant tile) {
	pushvalue push_site(last_site, (const sitei*)0);
	if(!apply_location(geo, tile))
		return;
	bsdata<itemground>::source.clear();
	area.clear();
	locations.clear();
	sites.clear();
	add_area_events(geo);
	add_area_sites(tile);
	rect all = {0, 0, area.mps - 1, area.mps - 1};
	pushvalue push_rect(last_rect, all);
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
}

void sitei::fillfloor() const {
	area.set(last_rect, &areamap::settile, getfloor());
}

void sitei::fillwallsall() const {
	area.set(last_rect, &areamap::settile, getwall());
}

void sitei::fillwalls() const {
	area.horz(last_rect.x1, last_rect.y1, last_rect.x2, &areamap::settile, walls);
	area.vert(last_rect.x1, last_rect.y1, last_rect.y2, &areamap::settile, walls);
	area.horz(last_rect.x1, last_rect.y2, last_rect.x2, &areamap::settile, walls);
	area.vert(last_rect.x2, last_rect.y1, last_rect.y2, &areamap::settile, walls);
}

void sitei::building() const {
	static direction_s rdir[] = {North, South, West, East};
	auto door = getdoor();
	// Walls and floors
	fillfloor();
	fillwalls();
	// Doors
	last_direction = maprnd(rdir);;
	last_door = area.getpoint(last_rect, last_direction);
	area.settile(last_door, floors);
	area.setfeature(last_door, door);
	auto m1 = to(last_door, last_direction);
	if(area.iswall(m1))
		area.settile(m1, floors);
	area.setfeature(m1, 0);
	last_rect.offset(1, 1);
}

void sitei::cityscape() const {
	fillfloor();
	script::run(this);
	last_rect.offset(last_location->offset, last_location->offset);
	create_city_level(last_rect, 0);
	sort_locations();
}

void sitei::outdoor() const {
	fillfloor();
	script::run(this);
	last_rect.offset(last_location->offset, last_location->offset);
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

void sitei::dungeon() const {
	last_rect.offset(last_location->offset, last_location->offset);
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
		auto r1 = area.get(rc, {2, 2}, {5, 3}, {8, 6});
		locations.add(r1);
	}
	shuffle_locations();
	correct_conncetors = bounding_locations();
}

void sitei::room() const {
	fillfloor();
	areahead.total.rooms++;
}

void sitei::corridors() const {
	const auto& rc = locations[0];
	auto dir = area.getmost(rc);
	create_corridor(rc, dir, walls, floors);
	area.change(0, walls);
	create_corridor_contents();
	create_doors(floors, walls);
}

void areaheadi::createarea(point start_village) {
	variant rt;
	last_quest = quest::find(position);
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
		if(last_quest) {
			auto chance_finale = level * 10;
			auto default_level = last_quest->level;
			auto final_level = bsdata<locationi>::find(str("%1Final", default_level.getid()));
			if(final_level && d100() < (final_level->chance_finale + chance_finale))
				rt = final_level;
			else if(last_quest->level)
				rt = last_quest->level;
		}
		if(!rt)
			rt = single("DefaultDungeon");
	}
	if(!rt)
		rt = single("LightForest");
	create_area(*this, rt);
}