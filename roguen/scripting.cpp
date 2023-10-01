#include "areapiece.h"
#include "boost.h"
#include "creature.h"
#include "draw.h"
#include "draw_object.h"
#include "dialog.h"
#include "filter.h"
#include "game.h"
#include "itema.h"
#include "listcolumn.h"
#include "modifier.h"
#include "greatneed.h"
#include "pushvalue.h"
#include "race.h"
#include "resid.h"
#include "resource.h"
#include "script.h"
#include "site.h"
#include "siteskill.h"
#include "skilluse.h"
#include "textscript.h"
#include "trigger.h"
#include "triggern.h"
#include "indexa.h"

void add_need(int bonus);
void add_need_answers(int bonus);
void afterpaint_no_actions();
void apply_value(variant v);
void apply_ability(ability_s v, int counter);
void animate_figures();
bool check_activate(creature* player, point m, const featurei& ei);
void choose_limit(int counter);
void damage_equipment(int bonus, bool allow_save);
bool isfreeltsv(point m);
bool isfreecr(point m);
void make_game_map_screenshoot();
void visualize_images(res pid, point size, point offset);

itema				items;
indexa				indecies;
spella				allowed_spells;
creature			*player, *opponent, *enemy;
int					last_coins;
const char*			last_id;
static point		last_door;
locationi*			last_location;
quest*				last_quest;
rect				last_rect;
roomi*				last_room;
const sitei*		last_site;
greatneed*			last_need;
int					last_value, last_cap;
extern bool			show_floor_rect;
static fntestvariant last_allow_proc;

static adat<rect, 32> locations;
static adat<point, 512> points;
static rect			correct_conncetors;
static direction_s	last_direction;
static adat<variant, 32> sites;

void apply_ability(ability_s v, int counter);

static int random_value(int value) {
	if(!value)
		return 0;
	return xrand(value / 2, value);
}

static void show_debug_minimap() {
	auto pt = center(last_rect);
	draw::setcamera(m2s(pt));
	script_run("ExploreArea");
	script_run("ShowMinimap");
}

static void fix_yellow(const char* format, int value) {
	if(!value)
		return;
	player->fixvalue(str(format, value), value >= 0 ? ColorYellow : ColorRed);
}

static void fix_green(const char* format, int value) {
	if(!value)
		return;
	player->fixvalue(str(format, value), value >= 0 ? ColorGreen : ColorRed);
}

static void fix_green(int value) {
	player->fixvalue(value, ColorGreen, ColorRed);
}

static int add_green(int current, int bonus, const char* format, int minimum = 0, int maximum = 120) {
	auto i = current + bonus;
	if(i < minimum)
		i = minimum;
	if(i > maximum)
		i = maximum;
	fix_green(format, i - current);
	return i;
}

void damage_item(item& it) {
	auto name = it.getname();
	it.damage();
	if(it)
		player->act(getnm("YouDamageItem"), name);
	else
		player->act(getnm("YouBrokeItem"), name);
}

static int getrange(point m1, point m2) {
	auto dx = iabs(m1.x - m2.x);
	auto dy = iabs(m1.y - m2.y);
	return (dx > dy) ? dx : dy;
}

static bool isfreeft(point m) {
	if(!area->isvalid(m))
		return false;
	if(area->features[m])
		return false;
	if(area->iswall(m))
		return false;
	return area->features[m] == 0;
}

static void apply_script(const script* p, int bonus) {
	if(p)
		p->proc(bonus);
}

int getfloor() {
	if(last_site && last_site->floors)
		return last_site->floors;
	if(last_location && last_location->floors)
		return last_location->floors;
	return 0;
}

static int getwall() {
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

static point randomft(const rect& rc) {
	return area->getfree(randomr(rc), 10, isfreeft);
}

static bool iswall(point i, direction_s d1, direction_s d2) {
	return area->iswall(i, d1)
		&& area->iswall(i, d2);
}

static bool isonewall(point i, direction_s d1, direction_s d2) {
	return area->iswall(i, d1)
		|| area->iswall(i, d2);
}

static bool isoneofdoor(point i, direction_s d1, direction_s d2) {
	if(area->getfeature(to(i, d1)).is(BetweenWalls))
		return true;
	if(area->getfeature(to(i, d2)).is(BetweenWalls))
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
	for(i.y = 0; i.y < area->mps; i.y++) {
		for(i.x = 0; i.x < area->mps; i.x++) {
			if(bsdata<featurei>::elements[int(area->features[i])].is(BetweenWalls)) {
				if(isvaliddoor(i))
					continue;
				area->setfeature(i, 0);
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
		if(last_rect.x2 >= area->mps - 6)
			last_rect.x2 = area->mps - 1;
	} else {
		if(last_rect.y1 >= 1 && last_rect.y1 <= 6)
			last_rect.y1 = 0;
		if(last_rect.y2 >= area->mps - 6)
			last_rect.y2 = area->mps - 1;
	}
	area->set(last_rect, &areamap::settile, road.value);
	script_run("RandomCommoner");
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

static bool allow_connector(point index, direction_s dir, bool deadend = false) {
	index = to(index, dir); // Forward
	if(!index.in(correct_conncetors))
		return false;
	if(!area->istile(index, 0))
		return false;
	if(!area->istile(to(index, round(dir, West)), 0))
		return false;
	if(!area->istile(to(index, round(dir, East)), 0))
		return false;
	index = to(index, dir);
	if(deadend) {
		if(!area->istile(index, 0))
			return false;
	}
	if(!area->istile(to(index, round(dir, West)), 0))
		return false;
	if(!area->istile(to(index, round(dir, East)), 0))
		return false;
	return true;
}

static bool allow_corridor(point index, direction_s dir, bool linkable = false) {
	index = to(index, dir); // Forward
	if(!index.in(correct_conncetors))
		return false;
	if(!area->istile(index, 0))
		return false;
	if(!area->istile(to(index, round(dir, West)), 0))
		return false;
	if(!area->istile(to(index, round(dir, East)), 0))
		return false;
	index = to(index, dir);
	if(linkable) {
		auto pr = roomi::find(index);
		if(pr)
			return true;
	}
	if(!area->istile(to(index, round(dir, West)), 0))
		return false;
	if(!area->istile(to(index, round(dir, East)), 0))
		return false;
	return true;
}

static void create_door(point m, int floor, int wall, bool hidden, bool locked, bool stuck) {
	const auto& door = bsdata<featurei>::elements[getdoor()];
	if(!door.isvisible())
		return;
	if(hidden) {
		area->settile(m, wall);
		auto ph = door.gethidden();
		if(ph)
			area->setfeature(m, ph - bsdata<featurei>::elements);
		area->total.doors_hidden++;
	} else if(locked) {
		area->settile(m, floor);
		auto ph = door.getlocked();
		if(ph)
			area->setfeature(m, ph - bsdata<featurei>::elements);
		area->total.doors_locked++;
	} else if(stuck) {
		area->settile(m, floor);
		auto ph = door.getstuck();
		if(ph)
			area->setfeature(m, ph - bsdata<featurei>::elements);
		area->total.doors_stuck++;
	} else {
		area->settile(m, floor);
		area->setfeature(m, &door - bsdata<featurei>::elements);
	}
	area->total.doors++;
}

static void create_connector(point index, direction_s dir, int wall, int floor, int& count, bool linkable) {
	const auto chance_line_corridor = 60;
	const auto minimum_corridor_lenght = 4;
	count = 0;
	point start = {-1000, -1000};
	while(true) {
		if(!allow_corridor(index, dir, linkable)) {
			if(!area->isvalid(start))
				return;
			break;
		}
		auto i0 = to(index, dir);
		area->tiles[i0] = floor;
		if(!area->isvalid(start))
			start = i0;
		index = i0;
		if(count >= minimum_corridor_lenght && d100() >= chance_line_corridor)
			break;
		count++;
	}
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
	auto index = area->getpoint(rc, correct_conncetors, dir);
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
	pushvalue push_room(last_room, roomi::find(center(rc)));
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
		area->total.rooms_hidden++;
}

static void create_doors(int floor, int wall) {
	for(auto& rc : locations)
		create_doors(rc, floor, wall);
}

static roomi* add_room(const sitei* ps, const rect& rc) {
	auto p = roomi::add();
	p->clear();
	p->set(ps);
	p->rc = rc;
	return p;
}

static void place_shape_ex(const shapei& e, point m, direction_s d, char symbol, int wall) {
	auto c = m;
	for(m.y = 0; m.y < e.size.y; m.y++) {
		for(m.x = 0; m.x < e.size.x; m.x++) {
			if(e[m] != symbol)
				continue;
			auto pm = e.translate(c, m, d);
			area->settile(pm, wall);
		}
	}
}

static void place_shape(const shapei& e, point m, direction_s d, int floor, int wall) {
	place_shape_ex(e, m, d, 'X', wall);
	place_shape_ex(e, m, d, '.', floor);
	place_shape_ex(e, m, d, '0', floor);
	place_shape_ex(e, m, d, '1', floor);
}

static void place_shape(const shapei& e, point m, int floor, int walls) {
	static direction_s direction[] = {North, South, West, East};
	place_shape(e, m, maprnd(direction), floor, walls);
}

static void place_shape(const shapei& e, rect rc, int floor, int walls) {
	if(rc.width() >= 3 && rc.height() >= 3)
		rc.offset(1);
	else if(rc.width() >= 6 && rc.height() >= 6)
		rc.offset(2);
	place_shape(e, area->get(rc), floor, walls);
}

int get_deafault_count(const monsteri& e, int area_level) {
	static dice source[] = {
		{4, 12},
		{3, 8},
		{2, 6},
		{2, 4},
		{1, 2}, // 0
		{1},
	};
	if(e.unique)
		return 1;
	auto level_creature = e.abilities[Level];
	if(area_level <= 1)
		area_level = 1;
	auto n = 4 + (level_creature - area_level);
	auto d = maptbl(source, n);
	return d.roll();
}

static void place_creature(variant v, int count) {
	if(count <= 0) {
		if(v.iskind<monsteri>())
			count = get_deafault_count(bsdata<monsteri>::elements[v.value], game.level);
		else
			count = xrand(2, 5);
		if(!count)
			count = 1;
	}
	for(auto i = 0; i < count; i++) {
		auto p = creature::create(area->get(last_rect), v);
		p->set(Local);
		if(p->is(Enemy))
			area->total.monsters++;
	}
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
		script_run(sites.data[i]);
	}
}

static bool apply_location(geoposition geo, variant tile) {
	last_location = tile;
	if(!last_location)
		return false;
	area->setloc(last_location);
	area->darkness = last_location->darkness;
	last_site = last_location;
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

static void create_corridor_loot(point i) {
	variant treasure = "RandomLoot";
	locationi* quest_modifier = (last_quest && last_quest->modifier) ? (locationi*)last_quest->modifier : 0;
	if(last_location && last_location->loot && d100() < 40)
		treasure = randomizeri::random(last_location->getloot());
	else if(quest_modifier && quest_modifier->loot && d100() < 30)
		treasure = randomizeri::random(quest_modifier->getloot());
	pushvalue push_rect(last_rect, {i.x, i.y, i.x, i.y});
	script_run(treasure);
	area->total.loots++;
}

static void create_wandered_monster(point i) {
	pushvalue push_rect(last_rect, {i.x, i.y, i.x, i.y});
	variant monster = "RandomCaveMonster";
	if(last_quest && last_quest->problem.iskind<monsteri>()) {
		monsteri* pm = last_quest->problem;
		monster = pm->ally();
	}
	place_creature(single(monster), 1);
}

static void create_trap(point i) {
	auto pm = bsdata<randomizeri>::find("RandomTraps");
	if(!pm)
		return;
	auto v = pm->random();
	area->setfeature(i, v.value);
	area->setflag(i, Hidden);
	area->total.traps++;
}

static bool ispassage(point m) {
	return !area->iswall(m) && !area->features[m]
		&& ((area->iswall(m, North) && area->iswall(m, South) && !area->iswall(m, East) && !area->iswall(m, West))
			|| (area->iswall(m, East) && area->iswall(m, West) && !area->iswall(m, North) && !area->iswall(m, South)));
}

static void create_corridor_traps(size_t maximum_count) {
	if(!maximum_count)
		return;
	indexa points;
	points.select(ispassage, true, 16);
	points.shuffle();
	if(points.count > maximum_count)
		points.count = maximum_count;
	for(auto m : points)
		create_trap(m);
}

static void create_corridor_contents() {
	zshuffle(points.data, points.count);
	auto count = xrand(10, 18);
	if(count > (int)points.count)
		count = points.count;
	for(auto i = 0; i < count; i++) {
		if(d100() < 30)
			create_wandered_monster(points.data[i]);
		else
			create_corridor_loot(points.data[i]);
	}
}

static void fillfloor() {
	area->set(last_rect, &areamap::settile, getfloor());
}

static void fillwallsall() {
	area->set(last_rect, &areamap::settile, getwall());
}

static void fillwalls() {
	area->horz(last_rect.x1, last_rect.y1, last_rect.x2, &areamap::settile, last_site->walls);
	area->vert(last_rect.x1, last_rect.y1, last_rect.y2, &areamap::settile, last_site->walls);
	area->horz(last_rect.x1, last_rect.y2, last_rect.x2, &areamap::settile, last_site->walls);
	area->vert(last_rect.x2, last_rect.y1, last_rect.y2, &areamap::settile, last_site->walls);
}

static void generate_building(int bonus) {
	static direction_s rdir[] = {North, South, West, East};
	auto door = getdoor();
	fillfloor();
	fillwalls();
	pushvalue push_direction(last_direction, maprnd(rdir));
	last_door = area->getpoint(last_rect, last_direction);
	area->settile(last_door, last_site->floors);
	area->setfeature(last_door, door);
	auto m1 = to(last_door, last_direction);
	if(area->iswall(m1))
		area->settile(m1, last_site->floors);
	area->setfeature(m1, 0);
	last_rect.offset(1, 1);
}

static void generate_cityscape(int bonus) {
	fillfloor();
	script_run(last_site);
	last_rect.offset(last_location->offset, last_location->offset);
	create_city_level(last_rect, 0);
	sort_locations();
}

static void generate_outdoor(int bonus) {
	fillfloor();
	script_run(last_site);
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

static void generate_dungeon(int bonus) {
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
		auto r1 = area->get(rc, {2, 2}, {4, 3}, {7, 5});
		locations.add(r1);
	}
	shuffle_locations();
	correct_conncetors = bounding_locations();
}

static void generate_room_record(int bonus) {
	area->total.rooms++;
}

static void generate_room(int bonus) {
	fillfloor();
	generate_room_record(bonus);
}

static void generate_cave(int bonus) {
	auto s1 = bsdata<shapei>::find("CircleRoom");
	auto s2 = bsdata<shapei>::find("SmallCircleRoom");
	if(!s1 || !s2)
		return;
	auto floor = getfloor();
	auto pt = center(last_rect);
	place_shape(*s1, pt, floor, floor);
	//show_debug_minimap();
	auto maximum = xrand(2, 4);
	for(auto i = 0; i < maximum; i++)
		place_shape(*s2, last_rect, floor, floor);
	generate_room_record(bonus);
}

static void generate_walls(int bonus) {
	fillwallsall();
}

static void generate_empthy_space(int bonus) {
	area->change(0, last_site->walls);
}

static void generate_corridors(int bonus) {
	const auto& rc = locations[0];
	auto dir = area->getmost(rc);
	create_corridor(rc, dir, last_site->walls, last_site->floors);
	generate_empthy_space(bonus);
	create_corridor_contents();
	create_doors(last_site->floors, last_site->walls);
	create_corridor_traps(random_value(last_location->traps_count));
}

static void create_area(geoposition geo, variant tile) {
	pushvalue push_site(last_site);
	if(!apply_location(geo, tile))
		return;
	area->areamap::clear();
	locations.clear();
	sites.clear();
	add_area_events(geo);
	add_area_sites(tile);
	rect all = {0, 0, area->mps - 1, area->mps - 1};
	pushvalue push_rect(last_rect, all);
	apply_script(last_location->global, 0);
	last_rect = push_rect;
	create_sites();
	last_rect = push_rect;
	apply_script(last_location->global_finish, 0);
	update_doors();
}

static void create_random_area() {
	variant rt;
	last_quest = quest::find(area->position);
	if(area->level == 0) {
		auto range = getrange(area->position, game.start_village);
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
			auto chance_finale = area->level * 10;
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
	create_area(*area, rt);
}

static areapiece* find_areapiece(geoposition geo) {
	for(auto& e : bsdata<areapiece>()) {
		if(e == geo)
			return &e;
	}
	return 0;
}

static void ready_area(geoposition geo) {
	area = find_areapiece(geo);
	if(!area) {
		area = bsdata<areapiece>::add();
		area->clear();
		area->position = geo.position;
		area->level = geo.level;
		create_random_area();
	}
}

static void update_ui() {
	draw::removeobjects(bsdata<creature>::source);
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid())
			e.fixappear();
	}
}

static void set_party_position(geoposition ogp, geoposition ngp, point m) {
	if(!area->isvalid(m))
		return;
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos == ogp && e.is(Ally)) {
			e.place(m);
			e.worldpos = ngp;
		}
	}
}

static void remove_summoned(geoposition geo) {
	for(auto& e : bsdata<creature>()) {
		if(!e.isvalid() || e.worldpos != geo)
			continue;
		if(e.is(Summoned)) {
			e.unlink();
			e.clear();
		}
	}
}

void gamei::enter(point m, int level, const featurei* feature, direction_s appear_side) {
	write("autosave");
	geoposition old_pos = *this;
	remove_summoned(old_pos);
	this->position = m;
	this->level = level;
	ready_area(*this);
	point start = {-1000, -1000};
	if(feature)
		start = area->findfeature((unsigned char)bsid(feature));
	if(!isvalid(start))
		start = area->bordered(round(appear_side, South));
	set_party_position(old_pos, *this, start);
	update_ui();
	draw::setnext(play);
}

static const script* get_local_method() {
	if(last_site && last_site->local)
		return last_site->local;
	if(last_location && last_location->local)
		return last_location->local;
	return 0;
}

static void apply_magical(item& it) {
	if(!it.getpower())
		return;
	static magic_s chance_special[20] = {
		Mundane, Mundane, Mundane, Mundane, Mundane,
		Mundane, Mundane, Mundane, Mundane, Mundane,
		Mundane, Mundane, Mundane, Blessed, Blessed,
		Blessed, Cursed, Cursed, Cursed, Artifact
	};
	it.set(maprnd(chance_special));
	switch(it.getmagic()) {
	case Artifact: area->total.artifacts++; break;
	case Blessed: area->total.blessed++; break;
	case Cursed: area->total.cursed++; break;
	default: break;
	}
}

static void place_item(point index, const itemi* pe) {
	if(!pe || pe == bsdata<itemi>::elements)
		return;
	if(area->iswall(index))
		return;
	item it; it.clear();
	it.create(pe);
	auto chance_power = 0;
	if(area->level)
		chance_power += iabs(area->level) * 5;
	it.createpower(chance_power);
	apply_magical(it);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	it.drop(index);
}

static void place_item(const itemi* pe) {
	if(!player || !pe || pe == bsdata<itemi>::elements)
		return;
	item it; it.clear();
	it.create(pe);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	player->additem(it);
}

static void visualize_activity(point m) {
	if(!area->is(m, Visible))
		return;
	movable::fixeffect(m2s(m), "SearchVisual");
}

static int random(const rect& rc, int v) {
	if(v <= -100)
		return 0;
	if(v < 0)
		v = (rc.width() + 1) * (rc.height() + 1) * (-v) / 100;
	if(v <= 0)
		v = 1;
	return v;
}

static void placement(rect rc, areamap::fnset proc, int v, int random_count) {
	if(random_count <= -100)
		area->set(rc, proc, v);
	else {
		random_count = random(rc, random_count);
		while(random_count > 0) {
			auto m = randomft(rc);
			(area->*proc)(m, v);
			random_count--;
		}
	}
}

template<> void ftscript<dialogi>(int value, int counter) {
	bsdata<dialogi>::elements[value].open();
}

template<> void ftscript<featurei>(int value, int counter) {
	placement(last_rect, &areamap::setfeature, value, counter);
}

template<> void ftscript<tilei>(int value, int counter) {
	area->set(last_rect, &areamap::settile, value, counter);
}

template<> void ftscript<areafi>(int value, int counter) {
	area->set(last_rect, &areamap::setflag, value, counter);
}

template<> bool fttest<monsteri>(int value, int counter) {
	if(!counter)
		return player->iskind(bsdata<monsteri>::elements + value);
	return true;
}
template<> void ftscript<monsteri>(int value, int counter) {
	auto count = script_count(counter, 0);
	if(count < 0)
		return;
	if(count == 0)
		count = get_deafault_count(bsdata<monsteri>::elements[value], game.level);
	if(!count)
		count = 1;
	place_creature(bsdata<monsteri>::elements + value, count);
}

template<> void ftscript<racei>(int value, int counter) {
	auto count = script_count(counter, 1);
	if(count <= 0)
		return;
	place_creature(bsdata<racei>::elements + value, count);
}

template<> void ftscript<classi>(int value, int counter) {
	auto count = script_count(counter, 1);
	for(auto i = 0; i < count; i++)
		creature::create(area->get(last_rect), single("RandomRace"), bsdata<classi>::elements + value, (rand() % 2) != 0);
}

template<> void ftscript<shapei>(int value, int counter) {
	auto count = script_count(counter, 1);
	for(auto i = 0; i < count; i++)
		place_shape(bsdata<shapei>::elements[value], last_rect, getfloor(), getwall());
}

template<> void ftscript<itemi>(int value, int counter) {
	auto count = script_count(counter, 1);
	switch(modifier) {
	case InPlayerBackpack:
		for(auto i = 0; i < count; i++)
			place_item(bsdata<itemi>::elements + value);
		break;
	default:
		for(auto i = 0; i < count; i++)
			place_item(randomft(last_rect), bsdata<itemi>::elements + value);
		break;
	}
}

template<> void ftscript<sitei>(int value, int counter) {
	pushvalue push_rect(last_rect);
	pushvalue push_site(last_site);
	last_site = bsdata<sitei>::elements + value;
	apply_script(get_local_method(), 0);
	add_room(last_site, last_rect);
	script_run(bsdata<sitei>::elements[value].landscape);
}

template<> void ftscript<locationi>(int value, int counter) {
	pushvalue push_rect(last_rect);
	script_run(bsdata<locationi>::elements[value].landscape);
}

template<> void ftscript<speech>(int value, int counter) {
	auto count = script_count(counter, 1);
	if(count <= 0)
		return;
	if(player)
		player->speech(bsdata<speech>::elements[value].id);
}

template<> bool fttest<needni>(int value, int counter) {
	if(counter <= 0)
		return last_need && last_need->is((needn)value);
	return true;
}
template<> void ftscript<needni>(int value, int counter) {
	if(!last_need)
		return;
	if(counter >= 0)
		last_need->set((needn)value);
	else
		last_need->remove((needn)value);
}

template<> bool fttest<abilityi>(int value, int counter) {
	if(counter < 0)
		return player->get((ability_s)value) >= -counter;
	return true;
}
template<> void ftscript<abilityi>(int value, int counter) {
	apply_ability((ability_s)value, counter);
}

template<> bool fttest<feati>(int value, int counter) {
	if(counter <= 0)
		return player->is((feat_s)value);
	return true;
}
template<> void ftscript<feati>(int value, int counter) {
	if(counter >= 0)
		player->feats.set(value);
	else
		player->feats.remove(value);
}

//template<> bool fttest<conditioni>(int value, int counter) {
//	return player->is((condition_s)value);
//}

static void move_left(int bonus) {
	player->movestep(West);
}

static void move_right(int bonus) {
	player->movestep(East);
}

static void move_up(int bonus) {
	player->movestep(North);
}

static void move_down(int bonus) {
	player->movestep(South);
}

static void move_up_left(int bonus) {
	player->movestep(NorthWest);
}

static void move_up_right(int bonus) {
	player->movestep(NorthEast);
}

static void move_down_left(int bonus) {
	player->movestep(SouthWest);
}

static void move_down_right(int bonus) {
	player->movestep(SouthEast);
}

bool choose_targets(const variants& conditions) {
	pushvalue push_index(last_index, player->getposition());
	indecies.clear();
	items.clear();
	rooms.clear();
	targets.clear();
	script_run(conditions);
	return targets.getcount() != 0
		|| rooms.getcount() != 0
		|| items.getcount() != 0
		|| indecies.getcount() != 0;
}

static void apply_target_effect(const variants& effect) {
	if(!effect)
		return;
	pushvalue push_opponent(opponent, player);
	pushvalue push_player(player);
	pushvalue push_modifier(modifier);
	pushvalue push_item(last_item);
	pushvalue push_value(last_room);
	pushvalue push_rect(last_rect);
	pushvalue push_index(last_index, player->getposition());
	for(auto p : targets) {
		player = p;
		script_run(effect);
	}
	for(auto p : indecies) {
		last_rect = {p.x, p.y, p.x, p.y};
		last_index = p;
		script_run(effect);
	}
	for(auto p : rooms) {
		last_room = p;
		last_rect = p->rc;
		last_index = center(last_rect);
		script_run(effect);
	}
	for(auto p : items) {
		last_item = p;
		script_run(effect);
	}
}

static bool choose_target_interactive(const char* id) {
	if(!id)
		return true;
	auto pn = getdescription(str("%1Choose", id));
	if(!pn)
		return true;
	pushvalue push_finish(draw::pfinish, afterpaint_no_actions);
	pushvalue push_width(window_width, 300);
	if(targets) {
		if(!targets.chooseu(pn, getnm("Cancel")))
			return false;
	}
	if(rooms) {
		if(!rooms.chooseu(pn, getnm("Cancel")))
			return false;
	}
	if(items) {
		if(!items.chooseu(pn, getnm("Cancel")))
			return false;
	}
	return true;
}

static void action_text(const creature* player, const char* id, const char* action) {
	if(player->canspeak()) {
		auto pn = player->getspeech(str("%1%2Speech", id, action), false);
		if(pn) {
			player->say(pn);
			return;
		}
	}
	auto pn = getdescription(str("%1%2", id, action));
	if(pn)
		player->act(pn);
}

static int get_target_count() {
	return targets.getcount()
		+ rooms.getcount()
		+ items.getcount()
		+ indecies.getcount();
}

static bool bound_targets(const char* id, int multi_targets, bool interactive, bool force_choose = false) {
	pushvalue push_interactive(answers::interactive, interactive);
	unsigned target_count = 1 + multi_targets;
	if(!multi_targets) {
		if(!force_choose)
			force_choose = get_target_count() > 1;
		if(force_choose && !choose_target_interactive(id))
			return false;
	}
	choose_limit(target_count);
	return true;
}

void apply_spell(const spelli& ei, int level) {
	pushvalue push_value(last_value, ei.getcount(level));
	if(ei.duration) {
		auto minutes = bsdata<durationi>::elements[ei.duration].get(level);
		auto stop_time = game.getminutes() + minutes;
		player->fixvalue(str("%1 %2i %-Minutes", ei.getname(), minutes), ColorGreen);
		for(auto v : ei.effect)
			boosti::add(player, v, stop_time);
	} else
		player->apply(ei.effect);
}

template<> void ftscript<spelli>(int index, int value) {
	apply_spell(bsdata<spelli>::elements[index], value);
}

bool spelli::apply(int level, int targets_count, bool interactive, bool silent) const {
	if(!bound_targets(id, targets_count, interactive))
		return false;
	if(!silent)
		action_text(player, id, "Casting");
	if(::targets.getcount()) {
		pushvalue push_player(player), push_opponent(opponent, player);
		for(auto p : (::targets)) {
			player = p;
			apply_spell(*this, level);
		}
	} else
		apply_target_effect(effect);
	if(summon)
		player->summon(player->getposition(), summon, getcount(level), level);
	return true;
}

void creature::cast(const spelli& e, int level, int mana) {
	if(get(Mana) < mana) {
		actp(getnm("NotEnoughtMana"));
		return;
	}
	if(!e.summon && !choose_targets(e.targets)) {
		actp(getnm("YouDontValidTargets"));
		return;
	}
	e.apply(level, 0, ishuman(), false);
	add(Mana, -mana);
	update();
	wait();
}

static void remove_feature(int bonus) {
	if(bonus > 0) {
		if(d100() >= bonus)
			return;
	}
	area->setfeature(last_index, 0);
}

static const char* random_herbs(point m) {
	auto& ei = bsdata<tilei>::elements[area->tiles[m]];
	if(ei.is(Undeground))
		return "UndegroundHerbs";
	return "GrassHerbs";
}

static featurei* herbs_base(featurei* p) {
	while(p->power) {
		auto p1 = p->getactivatefrom();
		if(!p1)
			break;
		p = p1;
	}
	return p;
}

static void gather_item(variant v, int bonus) {
	if(v.iskind<itemi>()) {
		item it; it.create(bsdata<itemi>::elements + v.value, v.counter);
		player->act(getnm("YouGatherItems"), it.getfullname());
		player->additem(it);
	}
}

static void gather_next_item(int bonus) {
	auto v = single(*script_begin++);
	gather_item(v, bonus);
}

static bool iskind(variant v, const char* id) {
	auto pi = bsdata<listi>::find(id);
	if(pi)
		return pi->is(v);
	auto pr = bsdata<randomizeri>::find(id);
	if(pr)
		return pr->is(v);
	return true;
}

static bool is_locked_door(int bonus) {
	auto pf = bsdata<featurei>::elements + area->features[last_index];
	if(!pf->activate_item)
		return false;
	return true;
}

static const char* item_weight(const void* object, stringbuilder& sb) {
	auto p = (item*)object;
	if(!(*p))
		return "";
	auto w = p->getweight();
	sb.add("%1i.%2i%3i %Kg", w / 100, (w / 10) % 10, w % 10);
	return sb.begin();
}

static item* choose_wear() {
	answers an;
	for(auto& e : player->equipment()) {
		if(e)
			an.add(&e, e.getfullname());
		else
			an.add(&e, "-");
	}
	static listcolumn columns[] = {
		{"Weight", 60, item_weight, true},
		{}};
	pushvalue push_columns(current_columns, columns);
	return (item*)an.choose(getnm("Inventory"), getnm("Cancel"));
}

static item* choose_stuff(wear_s wear) {
	answers an;
	char temp[512]; stringbuilder sb(temp);
	for(auto& e : player->backpack()) {
		if(!e)
			continue;
		if(wear && !e.is(wear))
			continue;
		sb.clear();
		e.getinfo(sb);
		an.add(&e, temp);
	}
	sb.clear();
	sb.add("%Choose %-1", bsdata<weari>::elements[wear].getname());
	return (item*)an.choose(temp, getnm("Cancel"));
}

static creature* getowner(const item* pi) {
	auto i = bsdata<creature>::source.indexof(pi);
	if(i == -1)
		return 0;
	return bsdata<creature>::elements + i;
}

static void inventory(int bonus) {
	while(true) {
		auto pi = choose_wear();
		if(!pi)
			break;
		auto owner = getowner(pi);
		if(!owner)
			break;
		if((*pi)) {
			if(player->canremove(*pi)) {
				player->additem(*pi);
				player->update();
			}
		} else {
			auto ni = choose_stuff(owner->getwearslot(pi));
			if(ni) {
				if(!player->isallow(*ni)) {
					console.addn(getnm("YouCantWearItem"), ni->getname());
					draw::pause();
				} else {
					iswap(*ni, *pi);
					player->update();
				}
			}
		}
	}
}

static void make_screenshoot(int bonus) {
	make_game_map_screenshoot();
}

static int free_objects_count() {
	auto result = 0;
	for(auto& e : bsdata<draw::object>()) {
		if(!e)
			continue;
		result++;
	}
	return result;
}

static void debug_message(int bonus) {
	//dialog_message(getdescription("LoseGame1"));
	//player->speech(ids("PickPockets", "Speech"));
	console.addn("Object count [%1i]/[%2i].", free_objects_count(), bsdata<draw::object>::source.getcount());
	auto m = player->getposition();
	console.addn("Position %1i, %2i.", m.x, m.y);
	auto f = area->features[m];
	if(f)
		console.adds("Feature %1 (%2i).", bsdata<featurei>::elements[f].getname(), f);
	//draw::pause();
}

static void open_locked_door(int bonus) {
	area->setactivate(last_index);
}

static void open_nearest_door(int bonus) {
	indexa source;
	source.select(player->getposition(), 1);
	for(auto i : source) {
		auto& ei = area->getfeature(i);
		if(!ei.isvisible())
			continue;
		check_activate(player, i, ei);
	}
}

static void chat_someone() {
	auto monster = player->getmonster();
	if(monster) {
		if(opponent->talk(monster->id))
			return;
	}
	auto room = player->getroom();
	if(room) {
		if(opponent->talk(room->geti().id))
			return;
	}
	if(player->speechneed())
		return;
	if(player->is(KnowRumor) && d100() < 70) {
		if(player->speechrumor())
			return;
	}
	if(player->is(KnowLocation) && !opponent->is(Local) && d100() < 30) {
		if(player->speechlocation())
			return;
	}
	player->speech("HowYouAre");
}

static void char_opponent(int bonus) {
	chat_someone();
	player->wait();
	opponent->wait();
}

static void apply_action(const char* id, const variants& targets, const variants& effects) {
	if(!choose_targets(targets)) {
		player->actp(getdescription(str("%1Fail", id)));
		return;
	}
	if(!bound_targets(id, 0, player->ishuman(), false))
		return;
	apply_target_effect(effects);
}

static void test_rumor(int bonus) {
	player->speechrumor();
}

static bool payment(creature* player, creature* keeper, const char* object, int coins) {
	if(player->ishuman()) {
		if(answers::console)
			answers::console->clear();
		player->actp(getnm("WantBuyItem"), object, coins);
		if(!draw::yesno(0))
			return false;
	}
	auto allow_coins = player->getmoney();
	if(player->getmoney() < coins) {
		keeper->speech("NotEnoughCoins", allow_coins, coins, coins - allow_coins);
		return false;
	}
	keeper->addcoins(coins);
	player->addcoins(-coins);
	return true;
}

static bool selling(creature* player, creature* opponent, const char* object, int coins) {
	if(player->ishuman()) {
		if(answers::console)
			answers::console->clear();
		player->actp(getnm("WantSellItem"), object, coins);
		if(!draw::yesno(0))
			return false;
	}
	auto allow_coins = opponent->getmoney();
	if(opponent->getmoney() < coins) {
		opponent->speech("KeeperNotEnoughCoins", allow_coins, coins, coins - allow_coins);
		return false;
	}
	player->addcoins(coins);
	opponent->addcoins(-coins);
	return true;
}

static void pickup(int bonus) {
	itema items;
	items.select(player->getposition());
	if(!items)
		return;
	auto p = items.choose(getnm("PickItem"), getnm("Cancel"));
	if(p) {
		auto payment_cost = player->getpaymentcost();
		if(payment_cost) {
			auto item_cost = p->getcostall() * payment_cost / 100;
			auto keeper = player->getroom()->getowner();
			if(!payment(player, keeper, p->getfullname(), item_cost))
				return;
		}
		player->act(getnm("PickupItem"), p->getfullname());
		player->additem(*p);
	}
}

static void pickup_all(int bonus) {
	if(player->getpaymentcost()) {
		player->actp("YouCantPickUpAllForCost");
		return;
	}
	itema items;
	items.select(player->getposition());
	for(auto p : items) {
		player->act(getnm("PickupItem"), p->getfullname());
		player->additem(*p);
	}
}

static void dropdown(int bonus) {
	itema items;
	items.selectbackpack(player);
	if(!items)
		return;
	auto p = items.choose(getnm("DropItem"), getnm("Cancel"));
	if(p) {
		auto payment_cost = player->getsellingcost();
		if(payment_cost) {
			auto item_cost = p->getcostall() * payment_cost / 100;
			auto keeper = player->getroom()->getowner();
			if(!selling(player, keeper, p->getfullname(), item_cost))
				return;
		}
		player->act(getnm("DropdownItem"), p->getfullname());
		p->drop(player->getposition());
	}
}

static void use_item(int bonus) {
	itema items;
	items.selectbackpack(player);
	items.match(fntis<item, &item::isusable>, true);
	if(!items)
		return;
	auto p = items.choose(getnm("UseItem"), getnm("Cancel"), false);
	if(p)
		player->use(*p);
}

static void use_magic_item_power(int bonus) {
	itema items;
	items.selectbackpack(player);
	items.match(fntis<item, &item::ischarges>, true);
	if(!items)
		return;
	auto p = items.choose(getnm("UseZapItem"), getnm("Cancel"), false);
	if(p)
		player->use(*p);
}

static void view_stuff(int bonus) {
	choose_stuff(Backpack);
}

static void explore_area(int bonus) {
	area->set({0, 0, area->mps, area->mps}, &areamap::setflag, Explored);
}

static void test_arena(int bonus) {
	answers an;
	auto count = 0;
	for(auto& e : bsdata<monsteri>()) {
		if(e.friendly <= -5)
			an.add(&e, e.getname());
	}
	pushvalue push_column(answers::column_count);
	answers::column_count = 3;
	auto pm = (monsteri*)an.choose(getnm("ChooseMonsterToFight"), getnm("Cancel"));
	if(!pm)
		return;
	auto m = player->getposition();
	auto p = creature::create(m.to(3, 0), pm);
	p->set(Enemy);
	p->wait();
	player->wait();
}

static void toggle_floor_rect(int bonus) {
	show_floor_rect = !show_floor_rect;
}

static void range_attack(int bonud) {
	if(!player->canshoot(true))
		return;
	if(!enemy) {
		player->actp(getnm("YouDontSeeAnyEnemy"));
		return;
	}
	if(enemy) {
		player->setdirection(area->getdirection(player->getposition(), enemy->getposition()));
		player->attackrange(*enemy);
		player->wait();
	}
}

static void thrown_attack(int bonud) {
	if(!player->canthrown(true))
		return;
	if(!enemy) {
		player->actp(getnm("YouDontSeeAnyEnemy"));
		return;
	}
	if(enemy) {
		player->setdirection(area->getdirection(player->getposition(), enemy->getposition()));
		player->attackthrown(*enemy);
		player->wait();
	}
}

static void show_images(int bonus) {
	static res source[] = {res::Monsters, res::Items};
	answers an;
	for(auto id : source)
		an.add((void*)id, bsdata<resource>::elements[(int)id].name);
	console.clear();
	player->act(getnm("ChooseImageSet"));
	auto id = (res)(int)an.choose();
	switch(id) {
	case res::Items:
		visualize_images(id, {64, 64}, {64 / 2, 64 / 2});
		break;
	default:
		visualize_images(id, {80, 90}, {80 / 2, 90});
		break;
	}
}

static void transfer_coins(int bonus) {
	if(bonus > 0) {
		if(bonus > player->money)
			bonus = player->money;
	} else {
		if(-bonus > opponent->money)
			bonus = -opponent->money;
	}
	player->addcoins(-bonus);
	opponent->addcoins(bonus);
}

static void quest_minion(int bonus) {
	if(!last_quest)
		return;
	monsteri* pm = last_quest->problem;
	if(pm)
		script_run(pm->ally(), bonus);
}

static void quest_guardian(int bonus) {
	if(!last_quest)
		return;
	monsteri* pm = last_quest->problem;
	if(pm) {
		area->total.boss++;
		script_run((monsteri*)last_quest->problem, bonus);
	}
}

static void quest_reward(int bonus) {
	if(last_quest && last_quest->reward) {
		variant v = last_quest->reward;
		v.counter = bonus;
		script_run(v);
	}
}

static void quest_landscape(int bonus) {
	if(!last_quest)
		return;
	locationi* pm = last_quest->modifier;
	if(pm)
		script_run(pm->landscape);
}

static void site_floor(int bonus) {
	if(last_site && last_site->floors)
		script_run(bsdata<tilei>::elements + last_site->floors, bonus);
	else if(last_location && last_location->floors)
		script_run(bsdata<tilei>::elements + last_location->floors, bonus);
}

static void site_wall(int bonus) {
	if(last_site && last_site->walls)
		script_run(bsdata<tilei>::elements + last_site->walls, bonus);
	else if(last_location && last_location->walls)
		script_run(bsdata<tilei>::elements + last_location->walls, bonus);
}

static const spelli* choose_spell(int bonus) {
	pushvalue push_width(window_width, 300);
	return allowed_spells.choose(getnm("ChooseSpell"), getnm("Cancel"), player);
}

static void cast_spell(int bonus) {
	auto p = choose_spell(bonus);
	if(p)
		player->cast(*p);
}

static void heal_player(int bonus) {
	player->abilities[Hits] = add_green(player->get(Hits), bonus, "%Heal%+1i", 0, player->getmaximum(Hits));
	if(!(*player))
		player->kill();
}

static void heal_all(int bonus) {
	heal_player(1000);
}

static void damage_all(int bonus) {
	player->abilities[DamageMelee] += bonus;
	player->abilities[DamageRanged] += bonus;
}

static void set_offset(int bonus) {
	if(bonus > last_rect.width() / 2)
		bonus = last_rect.width() / 2;
	if(bonus > last_rect.height() / 2)
		bonus = last_rect.height() / 2;
	last_rect.offset(bonus);
}

static void trigger_text(int bonus) {
	if(!last_site)
		return;
}

static void win_game(int bonus) {
	auto pn = bonus ? str("WinGame%1i", bonus) : "WinGame";
	dialog_message(getdescription(pn));
	game.next(game.mainmenu);
}

static void lose_game(int bonus) {
	auto pn = bonus ? str("LoseGame%1i", bonus) : "LoseGame";
	dialog_message(getdescription(pn));
	game.next(game.mainmenu);
}

static void add_dungeon_rumor(int bonus) {
	quest::add(KillBossQuest, game.position);
}

static void repair_item(int bonus) {
	if(!last_item)
		return;
	if(bonus > 0)
		last_item->setborken(0);
	else if(bonus < 0)
		last_item->setborken(3);
}

static bool roll_skill() {
	auto skill = player->get(last_action->skill) + last_action->bonus;
	auto result = d100();
	if(game.getowner() == player || player->is(Visible))
		player->logs(getnm("YouRollSkill"), getnm(last_action->id), skill, result);
	return result < skill;
}

static void fix_action(const char* suffix) {
	auto id = ids(last_action->id, suffix, "Action");
	auto p = getdescription(id);
	if(p)
		player->act(p);
}

static void speech_action() {
	if(!player->canspeak())
		return;
	auto id = ids(last_action->id, "Speech");
	if(bsdata<speech>::find(id))
		player->speech(id);
}

static void apply_action(int bonus) {
	if(!last_action)
		return;
	if(!choose_targets(last_action->targets))
		return;
	if(!bound_targets(last_action->id, 0, player->ishuman()))
		return;
	speech_action();
	apply_target_effect(last_action->effect);
	player->wait();
}

static void jump_to_site(int bonus) {
	if(!last_room)
		return;
	if(!player->ishuman())
		player->act(getnm("YouSundellyDisappear"));
	auto m = area->getfree(center(last_room->rc), 8, isfreecr);
	player->place(m);
	if(!player->ishuman())
		player->act(getnm("YouSundellyAppear"));
	else
		area->setlos(m, player->getlos(), isfreeltsv);
	player->fixteleport(player->ishuman());
}

static void wait_hour(int bonus) {
	player->wait(6 * 60 * 24);
}

static void roll_value(int bonus) {
	if(!player)
		return;
	if(!player->roll(last_ability, bonus)) {
		player->logs(getnm("YouMakeRoll"), last_value, player->get(last_ability) + bonus, bonus);
		script_stop();
	} else
		player->logs(getnm("YouFailRoll"), last_value, player->get(last_ability) + bonus, bonus);
}

static void roll_action(int bonus) {
	if(!last_action)
		return;
	last_ability = last_action->skill;
	roll_value(last_action->bonus + bonus);
	last_action->fixuse();
}

static void fail_roll_value(int bonus) {
	if(!player)
		return;
	if(!player->roll(last_ability, bonus))
		player->logs(getnm("YouMakeRoll"), last_value, player->get(last_ability) + bonus, bonus);
	else {
		player->logs(getnm("YouFailRoll"), last_value, player->get(last_ability) + bonus, bonus);
		script_stop();
	}
}

static void fail_roll_action(int bonus) {
	if(!last_action)
		return;
	last_ability = last_action->skill;
	roll_value(last_action->bonus + bonus);
}

static void random_chance(int bonus) {
	if(d100() >= bonus)
		script_stop();
}

static void select_raw_abilities() {
	auto p = raw_abilities;
	for(auto i = Strenght; i <= Wits; i = (ability_s)(i + 1))
		*p++ = i;
}

static int compare_player_ability(const void* p1, const void* p2) {
	auto a1 = *((ability_s*)p1);
	auto a2 = *((ability_s*)p2);
	auto v1 = player->get(a1);
	auto v2 = player->get(a2);
	return v1 - v2;
}

static void sort_raw_ability_topmost() {
	qsort(raw_abilities, sizeof(raw_abilities) / sizeof(raw_abilities[0]), sizeof(raw_abilities[0]), compare_player_ability);
}

static void ability_exchange(int bonus) {
	select_raw_abilities();
	sort_raw_ability_topmost();
	iswap(player->abilities[raw_abilities[1]], player->abilities[raw_abilities[2]]);
}

static void activate_feature(int bonus) {
	point m = center(last_rect);
	visualize_activity(m);
	area->setactivate(m);
}

static void destroy_feature(int bonus) {
	point m = center(last_rect);
	visualize_activity(m);
	area->setfeature(m, 0);
}

static bool feature_match(variant f, variant v);

static bool feature_match_one_of(variant f, const variants& source) {
	pushvalue push_begin(script_begin, source.begin());
	pushvalue push_end(script_end, source.end());
	while(script_begin < script_end) {
		if(feature_match(f, *script_begin++))
			return true;
	}
	return false;
}

static bool feature_match(variant f, variant v) {
	if(v.iskind<listi>())
		return feature_match_one_of(f, bsdata<listi>::elements[v.value].elements);
	else if(v.iskind<randomizeri>())
		return feature_match_one_of(f, bsdata<randomizeri>::elements[v.value].chance);
	else if(v.iskind<featurei>())
		return v.value == f.value;
	return false;
}

static bool feature_match(variant f, const variants& source) {
	pushvalue push_begin(script_begin, source.begin());
	pushvalue push_end(script_end, source.end());
	while(script_begin < script_end) {
		if(!feature_match(f, *script_begin++))
			return false;
	}
	return true;
}

static bool feature_match_next_allow(int bonus) {
	variant f = bsdata<featurei>::elements + area->features[last_index];
	return feature_match(f, *script_begin++);
}
static void feature_match_next(int bonus) {
	if(!feature_match_next_allow(bonus))
		script_stop();
}

static void identify_item(int bonus) {
	last_item->setidentified(bonus);
}

static void random_ability(int bonus) {
	static ability_s source[] = {Strenght, Dexterity, Wits};
	apply_ability(maprnd(source), bonus);
}

static void filter_allies(int bonus) {
	if(player->is(Ally))
		targets.match(Ally, true);
	else if(player->is(Enemy))
		targets.match(Enemy, true);
	else {
		targets.match(Enemy, false);
		targets.match(Ally, false);
	}
}

static void apply_filter(collectiona& source, variant v, void** filter_object) {
	auto pb = source.begin();
	auto push = *filter_object;
	if(v.counter >= 0) {
		for(auto p : source) {
			*filter_object = p;
			if(script_allow(v))
				*pb++ = p;
		}
	} else {
		for(auto p : source) {
			*filter_object = p;
			if(!script_allow(v))
				*pb++ = p;
		}
	}
	*filter_object = push;
	source.count = pb - source.begin();
}

static void filter_next(int bonus) {
	auto v = *script_begin++;
	if(targets)
		apply_filter(targets, v, (void**)&player);
	if(items)
		apply_filter(items, v, (void**)&last_item);
	if(rooms)
		apply_filter(rooms, v, (void**)&last_room);
}

static bool is_wounded(int bonus) {
	auto n = player->get(Hits);
	auto m = player->basic.abilities[Hits];
	return n > 0 && n < m;
}

static void standart_filter(int bonus) {
}

static void acid_harm(int bonus) {
	if(player->resist(AcidResistance, AcidImmunity))
		return;
	player->fixeffect("AcidSplash");
	player->damage(xrand(bonus / 2, bonus));
	player->add(Corrosion, 1);
	damage_equipment(bonus, true);
}

static void gain_experience(int bonus) {
	auto value = bonus * 100;
	fix_yellow("%Experience%+1i", value);
	player->experience += value;
}

static void gain_coins(int bonus) {
	auto value = bonus * 10;
	fix_yellow("%1i %Coins", value);
	player->money += value;
}

static void gain_satiation(int bonus) {
	player->satiation += bonus * 10;
}

static bool is_random(int bonus) {
	return d100() < 30;
}

static void empthy_script(int bonus) {
}

static bool is_npc(int bonus) {
	return player->is(Local) != 0;
}

static bool is_animal(int bonus) {
	return !player->canspeak();
}

static void need_help_info(stringbuilder& sb) {
	if(!last_need)
		return;
	auto pn = getdescription(last_need->geti().getid());
	if(!pn)
		return;
	sb.add(pn, game.timeleft(last_need->deadline));
}

static const char* visualize_progress(int score) {
	if(score == 0)
		return "NoAnyProgress";
	else if(score < 40)
		return "LittleProgress";
	else if(score < 70)
		return "HalfwayProgress";
	else
		return "AlmostFinishProgress";
}

static void actual_need_state(stringbuilder& sb) {
	if(!last_need)
		return;
	sb.add(getnm("VisualizeProgress"), getnm(visualize_progress(last_need->score)), game.timeleft(last_need->deadline));
}

static void list_of_feats(stringbuilder& sb) {
	for(auto i = (feat_s)0; i <= Blooding; i = (feat_s)(i + 1)) {
		if(player->is(i))
			sb.addn(bsdata<feati>::elements[i].getname());
	}
}

static void list_of_skills(stringbuilder& sb) {
	for(auto i = FirstSkill; i <= LastSkill; i = (ability_s)(i + 1)) {
		auto v = player->get(i);
		if(v)
			sb.addn("%1\t%2i%%", bsdata<abilityi>::elements[i].getname(), v);
	}
}

BSDATA(textscript) = {
	{"ActualNeedState", actual_need_state},
	{"ListOfFeats", list_of_feats},
	{"ListOfSkills", list_of_skills},
	{"NeedHelpIntro", need_help_info},
};
BSDATAF(textscript)
BSDATA(triggerni) = {
	{"WhenCreatureP1EnterSiteP2"},
	{"WhenCreatureP1Dead"},
	{"WhenCreatureP1InSiteP2UpdateAbilities"},
	{"EverySeveralDays"},
	{"EverySeveralDaysForP1"},
};
assert_enum(triggerni, EverySeveralDaysForP1)
BSDATA(script) = {
	{"AbilityExchange", ability_exchange},
	{"AcidHarm", acid_harm},
	{"Activate", activate_feature},
	{"AddDungeonRumor", add_dungeon_rumor},
	{"AddNeed", add_need},
	{"AddNeedAnswers", add_need_answers},
	{"ApplyAction", apply_action},
	{"AnimalInt", empthy_script, is_animal},
	{"CastSpell", cast_spell},
	{"Chance", random_chance},
	{"ChatOpponent", char_opponent},
	{"Damage", damage_all},
	{"DebugMessage", debug_message},
	{"DestroyFeature", destroy_feature},
	{"DropDown", dropdown},
	{"ExploreArea", explore_area},
	{"FailRollAction", fail_roll_action},
	{"FeatureMatchNext", feature_match_next, feature_match_next_allow},
	{"Filter", filter_next},
	{"GainCoins", gain_coins},
	{"GainExperience", gain_experience},
	{"GainSatiation", gain_satiation},
	{"GatherNextItem", gather_next_item},
	{"GenerateBuilding", generate_building},
	{"GenerateCave", generate_cave},
	{"GenerateCorridors", generate_corridors},
	{"GenerateDungeon", generate_dungeon},
	{"GenerateEmphtySpace", generate_empthy_space},
	{"GenerateOutdoor", generate_outdoor},
	{"GenerateRoom", generate_room},
	{"GenerateRoomNoFloor", generate_room_record},
	{"GenerateWalls", generate_walls},
	{"GenerateVillage", generate_cityscape},
	{"Heal", heal_player},
	{"HealAll", heal_all},
	{"IdentifyItem", identify_item},
	{"Inventory", inventory},
	{"JumpToSite", jump_to_site},
	{"LoseGame", lose_game},
	{"MakeScreenshoot", make_screenshoot},
	{"MoveDown", move_down},
	{"MoveDownLeft", move_down_left},
	{"MoveDownRight", move_down_right},
	{"MoveLeft", move_left},
	{"MoveRight", move_right},
	{"MoveUp", move_up},
	{"MoveUpRight", move_up_right},
	{"MoveUpLeft", move_up_left},
	{"NPC", empthy_script, is_npc},
	{"Offset", set_offset},
	{"OpenLockedDoor", open_locked_door, is_locked_door},
	{"OpenNearestDoor", open_nearest_door},
	{"PickUp", pickup},
	{"PickUpAll", pickup_all},
	{"QuestGuardian", quest_guardian},
	{"QuestLandscape", quest_landscape},
	{"QuestMinion", quest_minion},
	{"QuestReward", quest_reward},
	{"Random", empthy_script, is_random},
	{"RandomAbility", random_ability},
	{"RangeAttack", range_attack},
	{"RepairItem", repair_item},
	{"RemoveFeature", remove_feature},
	{"Roll", roll_value},
	{"RollAction", roll_action},
	{"ShowImages", show_images},
	{"SiteFloor", site_floor},
	{"SiteWall", site_wall},
	{"TransferCoins", transfer_coins},
	{"TestArena", test_arena},
	{"TestRumor", test_rumor},
	{"ThrownAttack", thrown_attack},
	{"ToggleFloorRect", toggle_floor_rect},
	{"TriggerText", trigger_text},
	{"ViewStuff", view_stuff},
	{"WaitHour", wait_hour},
	{"WinGame", win_game},
	{"Wounded", standart_filter, is_wounded},
	{"UseItem", use_item},
};
BSDATAF(script)