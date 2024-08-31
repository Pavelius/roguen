#include "areapiece.h"
#include "boost.h"
#include "creature.h"
#include "draw.h"
#include "draw_object.h"
#include "filter.h"
#include "functor.h"
#include "hotkey.h"
#include "game.h"
#include "math.h"
#include "itema.h"
#include "io_stream.h"
#include "listcolumn.h"
#include "markuse.h"
#include "modifier.h"
#include "greatneed.h"
#include "pushvalue.h"
#include "race.h"
#include "rand.h"
#include "resid.h"
#include "resource.h"
#include "script.h"
#include "site.h"
#include "siteskill.h"
#include "speech.h"
#include "talk.h"
#include "trigger.h"
#include "triggern.h"
#include "indexa.h"
#include "zcopy.h"
#include "widget.h"

struct speechi;

void add_need(int bonus);
void add_need_answers(int bonus);
void advance_value(variant v);
void animate_figures();
bool check_activate(creature* player, point m, const featurei& ei);
void damage_equipment(int bonus, bool allow_save);
bool isfreeltsv(point m);
bool isfreecr(point m);
void make_game_map_screenshoot();
void open_manual(const char* manual_header, const char* manual_content);
void visualize_images(res pid, point size, point offset);

int choose_indecies(const indexa& source, const char* header, bool cancel);
int choose_dialog(fnevent proc);

const char* getlog();

extern point		start_village;

itema				items;
indexa				indecies;
spella				allowed_spells;
creature			*player, *opponent;
int					last_coins;
const char*			last_id;
static point		last_door;
locationi*			last_location;
quest*				last_quest;
rect				last_rect;
const sitei*		last_site;
int					last_value;
listi*				last_craft_list;
static const spelli* last_spell_cast;
extern bool			show_floor_rect;
static fntestvariant last_allow_proc;
static int			effect_level;
static void*		specific_target;

static adat<rect, 64> locations;
static adat<point, 512> points;
static rect			correct_conncetors;
static direction_s	last_direction;
static adat<variant, 32> sites;

static int random_value(int value) {
	if(!value)
		return 0;
	return xrand(value / 2, value);
}

static void normalize_bonus(int& bonus) {
	if(!bonus)
		bonus = 1;
	if(bonus == 100)
		bonus = last_value;
	else if(bonus == -100)
		bonus = -last_value;
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

static void clear_console() {
	if(player && player->ishuman())
		console.clear();
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
		player->act("YouDamageItem", 0, name);
	else
		player->act("YouBrokeItem", 0, name);
}

static int getrange(point m1, point m2) {
	auto dx = iabs(m1.x - m2.x);
	auto dy = iabs(m1.y - m2.y);
	return (dx > dy) ? dx : dy;
}

static bool isfreeft(point m) {
	if(!area->isvalid(m))
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
	// TODO: If site is rocky mountain, then floor is rock. When you digging, you may dig passable floor.
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
	script_run(ids(last_site->id, "Population"));
}

static void create_city_level(const rect& rc, int level) {
	const int min_building_size = 5;
	const int max_building_size = 8;
	auto w = rc.width();
	auto h = rc.height();
	if(w <= max_building_size + 1 || h <= max_building_size + 1) {
		auto x1 = rc.x1;
		auto y1 = rc.y1;
		if(w > h)
			w = h;
		if(h > w)
			h = w;
		if(h > max_building_size)
			h = max_building_size;
		if(h != rc.height())
			y1 += rand() % (rc.height() - h);
		if(w > max_building_size)
			w = max_building_size;
		if(w != rc.width())
			x1 += rand() % (rc.width() - w);
		locations.add({x1 + 1, y1 + 1, x1 + w - 1, y1 + h - 1});
		return;
	}
	auto m = xrand(40, 60);
	auto r = (d100() < 50) ? 0 : 1;
	if(w > h)
		r = 0;
	else if(h > w)
		r = 1;
	auto rd = 2;
	if(level == 2)
		rd = 1;
	else if(level > 2)
		rd = 0;
	if(r == 0) {
		auto w1 = (w * m) / 100; // horizontal
		if(w1 < min_building_size)
			w1 = min_building_size;
		create_city_level({rc.x1, rc.y1, rc.x1 + w1 - rd - 1, rc.y2}, level + 1);
		create_city_level({rc.x1 + w1, rc.y1, rc.x2, rc.y2}, level + 1);
		if(rd)
			create_road({rc.x1 + w1 - rd, rc.y1, rc.x1 + w1 - 1, rc.y2});
	} else {
		auto h1 = (h * m) / 100; // vertial
		if(h1 < min_building_size)
			h1 = min_building_size;
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
		auto pr = find_room(index);
		if(pr)
			return true;
	}
	if(!area->istile(to(index, round(dir, West)), 0))
		return false;
	if(!area->istile(to(index, round(dir, East)), 0))
		return false;
	return true;
}

static void lock_door(point m) {
	auto ph = area->getfeature(m).getlocked();
	if(ph) {
		area->setfeature(m, ph - bsdata<featurei>::elements);
		area->total.doors_locked++;
	}
}

static void mark_room(int bonus) {
	markuse_add(last_action, center(last_rect), bsid(player), 0);
}

static void lock_all_doors(int bonus) {
	for(point m(last_rect.x1, last_rect.y1 - 1); m.x < last_rect.x2; m.x++)
		lock_door(m);
	for(point m(last_rect.x1, last_rect.y2 + 1); m.x < last_rect.x2; m.x++)
		lock_door(m);
	for(point m(last_rect.x1 - 1, last_rect.y1); m.y < last_rect.y2; m.y++)
		lock_door(m);
	for(point m(last_rect.x2 + 1, last_rect.y1); m.y < last_rect.y2; m.y++)
		lock_door(m);
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
		if(ph) {
			area->setfeature(m, ph - bsdata<featurei>::elements);
			area->total.doors_locked++;
		}
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
	pushvalue push_room(last_room, find_room(center(rc)));
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
	auto p = add_room();
	p->clear();
	p->set(ps);
	p->rc = rc;
	p->param = rand();
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
		{1, 2},
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
	for(auto i = 0; i < count; i++) {
		auto p = player_create(area->get(last_rect), v, false);
		p->set(Local);
		if(p->isevil())
			area->total.monsters++;
	}
}

static int get_count(variant v, int minimal = 1) {
	auto count = v.counter;
	if(count < 0 && d100() >= -count)
		return -1;
	if(count < minimal)
		count = minimal;
	return count;
}

static void add_area_sites(variant v) {
	if(!v)
		return;
	auto count = get_count(v);
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
		treasure = random_value(last_location->getloot());
	else if(quest_modifier && quest_modifier->loot && d100() < 30)
		treasure = random_value(quest_modifier->getloot());
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
		auto range = getrange(area->position, start_village);
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
		if(game.minutes > 60 * 20)
			area->minutes = game.minutes - xrand(60 * 4, 60 * 18);
		create_random_area();
	}
	check_time_passed();
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
	pushvalue push(player, game.getowner());
	if(!player)
		return;
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos == ogp && is_ally(&e)) {
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

void enter_area(point m, int level, const featurei* feature, direction_s appear_side) {
	if(area)
		zcopy<timemanage>(*area, game);
	save_game("autosave");
	geoposition old_pos = game;
	remove_summoned(old_pos);
	game.position = m;
	game.level = level;
	ready_area(game);
	point start = {-1000, -1000};
	if(area) {
		if(feature)
			start = area->findfeature((unsigned char)bsid(feature));
		if(!game.isvalid(start))
			start = area->bordered(round(appear_side, South));
	}
	set_party_position(old_pos, game, start);
	update_ui();
	next_phase(play_game);
}

static const script* get_local_method() {
	if(last_site && last_site->local)
		return last_site->local;
	if(last_location && last_location->local)
		return last_location->local;
	return 0;
}

static void add_statistic(item& it) {
	switch(it.getmagic()) {
	case Artifact: area->total.artifacts++; break;
	case Blessed: area->total.blessed++; break;
	case Cursed: area->total.cursed++; break;
	default: break;
	}
}

static item craft_item(const itemi* pi, magicn magic) {
	item it;
	it.create(pi, 1);
	it.set(magic);
	it.setidentified(1);
	return it;
}

static void add_item(point index, const itemi* pe, int count = 1, int chance_power = 0) {
	if(!pe || pe == bsdata<itemi>::elements || count <= 0)
		return;
	if(area->iswall(index))
		return;
	item it; it.clear();
	it.create(pe, count);
	if(area->level)
		chance_power += iabs(area->level) * 5;
	it.createpower(chance_power);
	it.createmagical();
	add_statistic(it);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	it.drop(index);
}

static void add_room_item(const itemi* pe, int count = 1, int chance_power = 0) {
	auto room_id = area->rooms.indexof(last_room);
	if(room_id == -1)
		return;
	item it; it.clear();
	it.create(pe, count);
	if(area->level)
		chance_power += iabs(area->level) * 5;
	auto m = InRoomToBuySpecial;
	if(!count)
		m = InRoomToBuy;
	it.createpower(chance_power);
	it.createmagical();
	it.setidentified(1);
	add_statistic(it);
	it.drop({get_token(m), (short)room_id});
}

static void add_item(item it) {
	player->additem(it);
}

static void add_item(const itemi* pe, int count = 1) {
	if(!player || !pe || pe == bsdata<itemi>::elements || count <= 0)
		return;
	item it; it.clear();
	it.create(pe, count);
	it.createpower(0);
	it.createmagical();
	add_statistic(it);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	add_item(it);
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

template<> void ftscript<widget>(int value, int counter) {
	choose_dialog(bsdata<widget>::elements[value].proc);
}

template<> void ftscript<speechi>(int value, int counter) {
	if(player)
		player->say(speech_get(value));
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
	if(!count)
		count = get_deafault_count(bsdata<monsteri>::elements[value], game.level);
	place_creature(bsdata<monsteri>::elements + value, count);
}

template<> void ftscript<racei>(int value, int counter) {
	auto count = script_count(counter, 1);
	if(count <= 0)
		return;
	place_creature(bsdata<racei>::elements + value, count);
}

template<> void ftscript<shapei>(int value, int counter) {
	auto count = script_count(counter, 1);
	for(auto i = 0; i < count; i++)
		place_shape(bsdata<shapei>::elements[value], last_rect, getfloor(), getwall());
}

template<> void ftscript<itemi>(int value, int counter) {
	auto count_base = script_count(counter, 0);
	auto count = count_base;
	if(count < 0)
		return;
	else if(!count)
		count = 1;
	switch(modifier) {
	case InPlayerBackpack: add_item(bsdata<itemi>::elements + value, count); break;
	case InPosition: add_item(last_index, bsdata<itemi>::elements + value, count); break;
	case InRoomToBuy: add_room_item(bsdata<itemi>::elements + value, count_base); break;
	default:
		for(auto i = 0; i < count; i++)
			add_item(randomft(last_rect), bsdata<itemi>::elements + value);
		break;
	}
}

template<> void ftscript<sitei>(int value, int counter) {
	pushvalue push_rect(last_rect);
	pushvalue push_site(last_site);
	pushvalue push_room(last_room);
	last_site = bsdata<sitei>::elements + value;
	apply_script(get_local_method(), 0);
	last_room = add_room(last_site, last_rect);
	script_run(bsdata<sitei>::elements[value].landscape);
}

template<> void ftscript<locationi>(int value, int counter) {
	pushvalue push_rect(last_rect);
	script_run(bsdata<locationi>::elements[value].landscape);
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
	last_ability = (ability_s)value;
	if(counter < 0)
		return (counter + player->basic.abilities[value]) >= 0;
	return true;
}
template<> void ftscript<abilityi>(int value, int counter) {
	last_ability = (ability_s)value;
	player->basic.add((ability_s)value, counter);
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

static void move_left(int bonus) {
	move_step(West);
}

static void move_right(int bonus) {
	move_step(East);
}

static void move_up(int bonus) {
	move_step(North);
}

static void move_down(int bonus) {
	move_step(South);
}

static void move_up_left(int bonus) {
	move_step(NorthWest);
}

static void move_up_right(int bonus) {
	move_step(NorthEast);
}

static void move_down_left(int bonus) {
	move_step(SouthWest);
}

static void move_down_right(int bonus) {
	move_step(SouthEast);
}

static void* choose_answers(const char* header, const char* cancel) {
	return an.choose(header, cancel);
}

static void* choose_answers() {
	if(player->ishuman())
		return an.choose();
	else
		return an.random();
}

static bool have_targets() {
	return targets.getcount() != 0
		|| rooms.getcount() != 0
		|| items.getcount() != 0
		|| indecies.getcount() != 0;
}

bool apply_targets(const variants& conditions) {
	pushvalue push_index(last_index, player->getposition());
	indecies.clear();
	items.clear();
	rooms.clear();
	targets.clear();
	script_run(conditions);
	return have_targets();
}

bool allow_targets(const variants& conditions) {
	pushvalue push_index(last_index, player->getposition());
	indecies.clear();
	items.clear();
	rooms.clear();
	targets.clear();
	return script_allow(conditions);
}

static bool choose_indecies(indexa& indecies, const char* header) {
	auto index = choose_indecies(indecies, header, true);
	if(index == -1)
		return false;
	iswap(indecies.data[index], indecies.data[0]);
	return true;
}

static bool choose_indecies(creaturea& source, const char* header, int offset) {
	indexa indecies;
	for(size_t i = offset; i < source.getcount(); i++)
		indecies.add(((creature*)source.data[i])->getposition());
	if(!indecies)
		return true;
	auto index = choose_indecies(indecies, header, true);
	if(index == -1)
		return false;
	iswap(source.data[offset + index], source.data[offset]);
	return true;
}

static bool choose_target_interactive(const char* id, int offset = 0) {
	if(!id)
		return true;
	auto pn = getnme(ids(id, "Choose"));
	if(!pn)
		return true; // Get random target
	pushvalue push_width(window_width, 300);
	if(targets.getcount() > 1) {
		if(!choose_indecies(targets, pn, offset))
			return false;
	}
	if(rooms.getcount() > 1) {
		if(!rooms.chooseu(pn, getnm("Cancel"), offset))
			return false;
	}
	if(items.getcount() > 1) {
		if(!items.chooseu(pn, getnm("Cancel"), offset))
			return false;
	}
	if(indecies.getcount() > 1) {
		if(!choose_indecies(indecies, pn))
			return false;
	}
	return true;
}

static int get_target_count() {
	return targets.getcount()
		+ rooms.getcount()
		+ items.getcount()
		+ indecies.getcount();
}

template<> void ftscript<spelli>(int index, int value) {
	cast_spell(bsdata<spelli>::elements[index], 0, true);
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

static bool empthy_next_condition(int bonus) {
	next_script();
	return true;
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

static const char* item_weight(const void* object, stringbuilder& sb) {
	auto p = (item*)object;
	if(!(*p))
		return "";
	auto w = p->getweight();
	item_weight(sb, w, true);
	return sb.begin();
}

static const char* items_footer() {
	static char temp[260]; stringbuilder sb(temp);
	sb.add(getnm("EncumbranceTotal"));
	return temp;
}

static item* choose_wear() {
	static listcolumn columns[] = {
		{"Weight", 60, item_weight, true},
		{}};
	an.clear();
	for(auto& e : player->equipment()) {
		if(e)
			an.add(&e, e.getfullname());
		else
			an.add(&e, "-");
	}
	pushvalue push_columns(current_columns, columns);
	pushvalue push_footer(answers::footer, items_footer());
	return (item*)choose_answers(getnm("Inventory"), getnm("Cancel"));
}

static item* choose_items(itema& items, const char* header, bool autochoose = false) {
	static listcolumn columns[] = {
		{"Weight", 60, item_weight, true},
		{}};
	pushvalue push_columns(current_columns, columns);
	pushvalue push_footer(answers::footer, items_footer());
	return (item*)items.choose(header, getnm("Cancel"), autochoose);
}

static item* choose_item(const char* header_format = 0) {
	static listcolumn columns[] = {
		{"Weight", 60, item_weight, true},
		{}};
	pushvalue push_columns(current_columns, columns);
	pushvalue push_footer(answers::footer, items_footer());
	return (item*)choose_answers(header_format, getnm("Cancel"));
}

static item* choose_stuff(wear_s wear, const char* header_format = 0, fnvisible proc = 0) {
	if(!header_format)
		header_format = getnm(ids("Choose", bsdata<weari>::elements[wear].id));
	an.clear();
	char temp[512]; stringbuilder sb(temp);
	for(auto& e : player->backpack()) {
		if(!e)
			continue;
		if(wear && !e.is(wear))
			continue;
		if(proc && !proc(&e))
			continue;
		sb.clear();
		e.getinfo(sb);
		an.add(&e, temp);
	}
	sb.clear();
	sb.add(header_format);
	return choose_item(temp);
}

static listi* get_ingridient_list(const itemi* pi) {
	auto p = bsdata<listi>::find(ids(pi->id, "Ingridients"));
	if(!p && pi->unidentified)
		p = bsdata<listi>::find(ids(pi->unidentified, "Ingridients"));
	if(!p)
		p = bsdata<listi>::find(ids(bsdata<weari>::elements[pi->wear].id, "Ingridients"));
	return p;
}

static listi* get_ability_craft(ability_s v) {
	static listi* alchemy = bsdata<listi>::find("AlchemyCraft");
	switch(v) {
	case Alchemy: return alchemy;
	default: return 0;
	}
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
				if(!player->isallow(*ni))
					console.addn(getnm("YouCantWearItem"), ni->getname());
				else {
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

static int get_rate(ability_s a, int v) {
	switch(a) {
	case Strenght:
		if(v >= 80)
			return 6;
		else if(v >= 50)
			return 5;
		return 4;
	case Wits:
	case Dexterity:
		if(v >= 80)
			return 5;
		else if(v >= 50)
			return 4;
		return 3;
	default:
		if(v >= 90)
			return 4;
		else if(v >= 60)
			return 3;
		else if(v >= 30)
			return 2;
		return 1;
	}
}

static void add_raise(ability_s a) {
	auto pb = bsdata<abilityi>::begin() + a;
	auto v = player->basic.abilities[a];
	if(!v)
		return;
	auto r = get_rate(a, v);
	if(player->basic.abilities[SkillPoints] < r)
		return;
	an.add(pb, getnm("RaiseSkill"), pb->getname(), v + 1, r);
}

static void raise_ability_by_skill(ability_s a) {
	auto v = player->basic.abilities[a];
	auto r = get_rate(a, v);
	if(player->basic.abilities[SkillPoints] < r)
		return;
	player->basic.abilities[SkillPoints] -= r;
	player->basic.abilities[a]++;
	player->update();
}

static const char* skill_value(const void* object, stringbuilder& sb) {
	auto i = (ability_s)bsdata<abilityi>::source.indexof(object);
	auto v = player->basic.abilities[i];
	sb.add("%1i%%", v);
	return sb.begin();
}

static const char* skill_raise_cost(const void* object, stringbuilder& sb) {
	auto p = (abilityi*)object;
	auto i = (ability_s)bsdata<abilityi>::source.indexof(object);
	auto v = player->basic.abilities[i];
	auto r = get_rate(i, v);
	sb.add(getnm("RaiseSkill"), p->getname(), v + 1, r);
	return sb.begin();
}

static abilityi* choose_skill(listcolumn* columns, const char* header, int dialog_width) {
	an.clear();
	for(auto i = Strenght; i <= LastSkill; i = (ability_s)(i + 1)) {
		if(i >= DamageMelee && i <= EnemyAttacks)
			continue;
		if(!player->basic.abilities[i])
			continue;
		an.add(bsdata<abilityi>::elements + i, bsdata<abilityi>::elements[i].getname());
	}
	char temp[260]; stringbuilder sb(temp);
	sb.add(getnm(header), player->basic.abilities[SkillPoints]);
	pushvalue push_columns(current_columns, columns);
	pushvalue push_width(window_width, dialog_width);
	return (abilityi*)choose_answers(temp, getnm("Cancel"));
}

static void raise_skills(int bonus) {
	if(player->basic.abilities[SkillPoints] <= 0) {
		static listcolumn columns[] = {
			{"Value", 32, skill_value},
			{}};
		choose_skill(columns, "Skills", 248);
	} else {
		static listcolumn columns[] = {
			{"Value", 32, skill_value},
			{"Cost", 90, skill_raise_cost},
			{}};
		while(player->basic.abilities[SkillPoints] > 0) {
			auto p = choose_skill(columns, "RaiseSkillChoose", 280);
			if(!p)
				break;
			auto a = (ability_s)(p - bsdata<abilityi>::elements);
			raise_ability_by_skill(a);
		}
	}
}

static void debug_message(int bonus) {
	//console.addn("Object count [%1i]/[%2i].", free_objects_count(), bsdata<draw::object>::source.getcount());
	//auto m = player->getposition();
	//console.addn("Position %1i, %2i.", m.x, m.y);
	//auto f = area->features[m];
	//if(f)
	//	console.adds("Feature %1 (%2i).", bsdata<featurei>::elements[f].getname(), f);
	raise_skills(0);
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

static void chatting() {
	if(opponent->isgood() && player->isevil()) {
		opponent->speak("DoNotSpeakEvil");
		return;
	}
	if(opponent->getowner() == player) {
		opponent->speak("HelloBoss");
		return;
	}
	if(opponent->istired()) {
		opponent->speak("IAmTired");
		opponent->add(Mood, -1);
		return;
	}
	if(player->ishuman()) {
		auto monster = opponent->getmonster();
		if(monster) {
			if(talk_opponent(monster->id, 0))
				return;
		}
		auto room = opponent->getroom();
		if(room) {
			if(talk_opponent(room->geti().id, 0))
				return;
		}
		if(speech_need())
			return;
	}
	if(opponent->is(KnowRumor) && d100() < 70) {
		if(opponent->speechrumor()) {
			opponent->add(Mood, -xrand(2, 5));
			return;
		}
	}
	if(player->ishuman() && opponent->is(KnowLocation) && d100() < 30) {
		if(opponent->speechlocation()) {
			opponent->add(Mood, -xrand(2, 5));
			return;
		}
	}
	opponent->speak("HowYouAre");
	opponent->add(Mood, -xrand(2, 4));
}

static void chatting(int bonus) {
	chatting();
	pay_action();
	opponent->wait();
}

static void test_rumor(int bonus) {
	player->speechrumor();
}

static bool payment(creature* player, creature* keeper, const char* object_name, int coins, const char* confirm = 0) {
	if(confirm && player->ishuman()) {
		clear_console();
		player->actp(confirm, 0, object_name, coins);
		if(!yesno(0))
			return false;
	}
	auto allow_coins = player->getmoney();
	if(player->getmoney() < coins) {
		keeper->speak("NotEnoughCoins", 0, allow_coins, coins, coins - allow_coins);
		return false;
	}
	keeper->addcoins(coins);
	player->addcoins(-coins);
	keeper->speak("BuyExplicitItem", 0, coins, object_name);
	return true;
}

static bool selling(creature* player, creature* opponent, const char* object, int coins, const char* confirm = 0) {
	if(confirm && player->ishuman()) {
		clear_console();
		player->actp(confirm, 0, object, coins);
		if(!yesno(0))
			return false;
	}
	auto allow_coins = opponent->getmoney();
	if(opponent->getmoney() < coins) {
		opponent->speak("KeeperNotEnoughCoins", 0, allow_coins, coins, coins - allow_coins);
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
	auto p = choose_items(items, getnm("PickItem"), true);
	if(p) {
		//auto payment_cost = player->getpaymentcost();
		//if(payment_cost) {
		//	auto item_cost = p->getcostall() * payment_cost / 100;
		//	auto keeper = player->getroom()->getowner();
		//	if(!payment(player, keeper, p->getfullname(), item_cost))
		//		return;
		//}
		player->act("PickupItem", 0, p->getfullname());
		player->additem(*p);
		player->update();
	}
}

static void pickup_all(int bonus) {
	//if(player->getpaymentcost()) {
	//	player->actp("YouCantPickUpAllForCost");
	//	return;
	//}
	itema items;
	items.select(player->getposition());
	for(auto p : items) {
		player->act("PickupItem", 0, p->getfullname());
		player->additem(*p);
	}
	player->update();
}

static void drop_down(int bonus) {
	itema items;
	items.selectbackpack(player);
	if(!items)
		return;
	auto p = choose_items(items, getnm("DropItem"));
	if(p) {
		auto payment_cost = player->getsellingcost();
		if(payment_cost) {
			auto item_cost = p->getcostall() * payment_cost / 100;
			auto keeper = player->getroom()->getowner();
			if(!selling(player, keeper, p->getfullname(), item_cost))
				return;
		}
		player->act("DropdownItem", 0, p->getfullname());
		p->drop(player->getposition());
		player->update();
	}
}

static void use_item(int bonus) {
	auto p = choose_stuff(Backpack, "%UseItem", fntis<item, &item::isusable>);
	if(p)
		use_item(*p);
}

static void view_stuff(int bonus) {
	auto pi = choose_stuff(Backpack);
	if(!pi)
		return;
	char temp[1024]; stringbuilder sb(temp);
	pi->getexamine(sb);
	if(sb)
		player->say(temp);
}

static void explore_area(int bonus) {
	area->set({0, 0, area->mps, area->mps}, &areamap::setflag, Explored);
}

static void detect_all_items(int bonus) {
	for(auto& e : area->items) {
		if(!e)
			continue;
		if(!area->isvalid(e.position))
			continue; // Special room items
		area->setflag(e.position, Explored);
		area->setflag(e.position, Visible);
	}
}

static void detect_items(wear_s v) {
	for(auto& e : area->items) {
		if(!e || !e.is(v))
			continue;
		if(!area->isvalid(e.position))
			continue;
		area->setflag(e.position, Explored);
		area->setflag(e.position, Visible);
	}
}

static void detect_items(feat_s v) {
	for(auto& e : area->items) {
		if(!e || !e.is(v))
			continue;
		area->setflag(e.position, Explored);
		area->setflag(e.position, Visible);
	}
}

static void detect_area_items(variant v) {
	if(v.iskind<weari>())
		detect_items((wear_s)v.value);
	else if(v.iskind<feati>())
		detect_items((feat_s)v.value);
	else if(v.iskind<listi>()) {
		for(auto ev : bsdata<listi>::elements[v.value].elements)
			detect_area_items(ev);
	} else if(v.iskind<randomizeri>()) {
		for(auto ev : bsdata<randomizeri>::elements[v.value].chance)
			detect_area_items(ev);
	}
}

static void detect_items(int bonus) {
	auto v = next_script();
	detect_area_items(v);
}

static void some_coins(int bonus) {
	bonus = script_count(bonus);
	if(bonus <= 0)
		return;
	player->money += xrand(bonus * 50, bonus * 100);
}

static void test_enemy_change(int bonus) {
	if(!opponent)
		player->actp("YouDontSeeAnyEnemy");
	game.setowner(opponent);
	player->wait();
}

static void test_arena(int bonus) {
	an.clear();
	auto count = 0;
	for(auto& e : bsdata<monsteri>()) {
		if(e.islower(Reputation, -30))
			an.add(&e, e.getname());
	}
	pushvalue push_column(answers::column_count);
	answers::column_count = 3;
	auto pm = (monsteri*)choose_answers(getnm("ChooseMonsterToFight"), getnm("Cancel"));
	if(!pm)
		return;
	auto m = player->getposition();
	auto p = player_create(m.to(3, 0), pm, false);
	make_hostile(p, player);
	p->wait();
	pay_action();
}

static void toggle_floor_rect(int bonus) {
	show_floor_rect = !show_floor_rect;
}

static void show_logs(int bonus) {
	open_manual(getnm("GameLogs"), getlog());
}

static void show_images(int bonus) {
	static res source[] = {res::Monsters, res::Items};
	an.clear();
	for(auto id : source)
		an.add((void*)id, bsdata<resource>::elements[(int)id].name);
	clear_console();
	player->actp("ChooseImageSet");
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

static bool is_transfer_coins(int bonus) {
	if(bonus > 0)
		return player->money >= bonus;
	else
		return opponent->money >= -bonus;
}

static void transfer_coins(int bonus) {
	if(bonus > 0) {
		if(bonus > player->money)
			bonus = player->money;
	} else {
		if(-bonus > opponent->money)
			bonus = -opponent->money;
	}
	last_value = bonus;
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
	pushvalue push_width(window_width, 248);
	return allowed_spells.choose(getnm("ChooseSpell"), getnm("Cancel"), player);
}

static void cast_spell(int bonus) {
	pushvalue push(last_spell);
	last_spell = choose_spell(bonus);
	last_spell_cast = last_spell;
	if(last_spell)
		cast_spell(*last_spell);
}

static void cast_last_spell(int bonus) {
	if(!last_spell_cast)
		return;
	if(!allowed_spells.have((void*)last_spell_cast)) {
		last_spell_cast = 0;
		return;
	}
	cast_spell(*last_spell_cast);
}

static void heal_player(int bonus) {
	player->abilities[Hits] = add_green(player->get(Hits), bonus, "%Heal%+1i", 0, player->getmaximum(Hits));
	if(!(*player))
		player->kill();
}

static void harm_player(int bonus) {
	player->damage(bonus);
}

static bool is_heal_wounded(int bonus) {
	return player->abilities[Blooded] > 0;
}
static void heal_wounded(int bonus) {
	if(bonus >= 100)
		player->abilities[Blooded] = 0;
	else
		player->abilities[Blooded] -= bonus;
}

static bool is_heal_poison(int bonus) {
	return player->abilities[Poison] > 0;
}
static void heal_poison(int bonus) {
	if(bonus >= 100)
		player->abilities[Poison] = 0;
	else
		player->abilities[Poison] -= bonus;
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

static void win_game(int bonus) {
	auto pn = bonus ? str("WinGame%1i", bonus) : "WinGame";
	dialog_message(getnm(pn));
	next_phase(main_menu);
}

static void lose_game(int bonus) {
	auto pn = bonus ? str("LoseGame%1i", bonus) : "LoseGame";
	dialog_message(getnm(pn));
	next_phase(main_menu);
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

static void apply_action(int bonus) {
	if(!last_action)
		return;
	if(last_action->tool)
		last_action->usetool();
	if(!player->speak("Apply", last_action->id))
		player->act("Apply", last_action->id);
	apply_targets(last_action->effect);
	pay_action();
}

static void jump_to_site(int bonus) {
	if(!player->ishuman())
		player->act("YouSundellyDisappear");
	auto m = area->getfree(last_index, 8, isfreecr);
	player->place(m);
	if(!player->ishuman())
		player->act("YouSundellyAppear");
	else
		area->setlos(m, player->getlos(), isfreeltsv);
	player->fixteleport(player->ishuman());
}

static void wait_hour(int bonus) {
	if(bonus < 0)
		return;
	normalize_bonus(bonus);
	player->waitseconds(xrand(bonus * 600 / 2, bonus * 600));
}

static void apply_fail_roll() {
	if(last_action) {
		auto p = bsdata<listi>::find(ids(last_action->id, "Fail"));
		if(p)
			script_run(p->elements);
	}
}

static void roll_value(int bonus) {
	if(!player)
		return;
	auto p = bsdata<abilityi>::elements + last_ability;
	if(!player->roll(last_ability, bonus)) {
#ifdef _DEBUG
		player->logs(getnm("YouFailRoll"), p->getname(), last_roll_result, player->get(last_ability) + bonus);
#endif // _DEBUG
		script_stop();
		apply_fail_roll();
	} else {
#ifdef _DEBUG
		player->logs(getnm("YouMakeRoll"), p->getname(), last_roll_result, player->get(last_ability) + bonus);
#endif // _DEBUG
	}
}

static void roll_action(int bonus) {
	if(!last_action)
		return;
	last_ability = last_action->skill;
	roll_value(last_action->bonus + bonus);
	if(last_roll_successed) {
		if(!player->speak("Success", last_action->id))
			player->act("Success", last_action->id);
	} else {
		if(!player->speak("Fail", last_action->id))
			player->act("Fail", last_action->id);
	}
}

static void gain_experience(int bonus) {
	auto value = bonus * 100;
	fix_yellow("%Experience%+1i", value);
	player->experience += value;
}

static bool learn_value(variant v, const char* action) {
	if(v.iskind<feati>()) {
		if(v.counter >= 0) {
			if(!player->feats.is(v.value))
				player->feats.set(v.value);
			else
				return false;
		} else {
			if(player->feats.is(v.value))
				player->feats.remove(v.value);
			else
				return false;
		}
	} else if(v.iskind<abilityi>()) {
		if(v.counter >= 0) {
			if(player->basic.abilities[v.value] + v.counter < 100)
				ftscript<abilityi>(v.value, v.counter);
			else
				return false;
		} else
			ftscript<abilityi>(v.value, v.counter);
	} else if(v.iskind<spelli>()) {
		auto p = bsdata<spelli>::elements + v.value;
		if(v.counter >= 0) {
			if(!player->known_spell(v.value) && player->basic.abilities[Mana] < p->mana)
				player->learn_spell(v.value);
			else
				return false;
		} else {
			if(player->known_spell(v.value))
				player->forget_spell(v.value);
			else
				return false;
		}
	} else
		return false;
	auto id = v.getid();
	auto name = getnm(id);
	if(!player->act(action, id)) {
		if(!player->act(action, bsdata<varianti>::elements[v.type].id, name))
			player->act(action, "Default", name);
	}
	return true;
}

static const listi* chance_ill(const item* pi) {
	if(!pi)
		return 0;
	auto& ei = last_item->geti();
	if(!ei.cursed)
		return 0;
	int chance = last_item->geti().chance_ill;
	switch(pi->getmagic()) {
	case Artifact: chance -= 40; break;
	case Cursed: chance += 40; break;
	case Blessed: chance -= 10; break;
	}
	if(chance < 0)
		chance = 0;
	else if(chance > 90)
		chance = 90;
	if(d100() < chance)
		return ei.cursed;
	return 0;
}

static void roll_for_effect(int bonus) {
	roll_value(0);
	if(script_stopped()) {
		auto ill_effect = chance_ill(last_item);
		if(ill_effect) {
			auto number_effects = ill_effect->elements.size();
			if(number_effects) {
				auto value = ill_effect->elements.begin()[rand() % number_effects];
				if(learn_value(value, "LearnBad"))
					return;
			}
		}
		player->act("FailLearn");
	} else {
		auto number_effects = script_end - script_begin;
		if(number_effects) {
			auto value = script_begin[rand() % number_effects];
			if(!learn_value(value, "Learn")) {
				gain_experience(xrand(4, 10));
				player->act("GainExperienceLearn");
			}
		}
	}
}

static void roll_learning(int bonus) {
	auto push_ability = last_ability;
	last_ability = Literacy;
	roll_for_effect(bonus);
	player->wait(xrand(600, 1200));
	last_ability = push_ability;
}

static void random_chance(int bonus) {
	if(d100() >= bonus)
		script_stop();
}

static void ability_exchange(int bonus) {
	switch(rand() % 4) {
	case 1: iswap(player->basic.abilities[Strenght], player->basic.abilities[Wits]); break;
	case 2: iswap(player->basic.abilities[Strenght], player->basic.abilities[Dexterity]); break;
	default: iswap(player->basic.abilities[Wits], player->basic.abilities[Dexterity]); break;
	}

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

static void destroy_wall(int bonus) {
	point m = center(last_rect);
	auto& ei = bsdata<tilei>::elements[area->tiles[m]];
	if(ei.tile) {
		visualize_activity(m);
		area->settile(m, ei.tile);
	}
}

static void identify_item(int bonus) {
	last_item->setidentified(bonus);
}

static void curse_item(int bonus) {
	if(bonus >= 0)
		last_item->set(Cursed);
	else if(last_item->getmagic() == Cursed)
		last_item->set(Mundane);
}

static void damage_item(int bonus) {
	last_item->damage(bonus);
}

static void random_ability(int bonus) {
	static ability_s source[] = {Strenght, Dexterity, Wits};
	ftscript<abilityi>(maprnd(source), bonus);
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
	auto damage = xrand(bonus / 2, bonus);
	player->fixeffect("AcidSplash");
	player->damage(damage);
	player->add(Corrosion, 1);
	damage_equipment(bonus, true);
}

static void fire_harm(int bonus) {
	if(player->resist(FireResistance, FireImmunity))
		return;
	player->fixeffect("FireSplash");
	player->add(Burning, 1);
	auto damage = xrand(bonus / 2, bonus) - imax(player->get(Armor) - 2, 0);
	if(damage <= 0)
		return;
	player->damage(damage);
	damage_backpack_item(Scroll, 50);
	damage_backpack_item(Tome, 40);
}

static void cold_harm(int bonus) {
	if(player->resist(ColdResistance, ColdImmunity))
		return;
	player->fixeffect("IceSplash");
	player->add(Freezing, 2);
	auto damage = bonus - player->get(Armor);
	if(damage <= 0)
		return;
	player->damage(damage);
	damage_backpack_item(Potion, 20);
}

static bool is_full(int bonus) {
	script_stop();
	return have_targets();
}

static bool is_targets(int bonus) {
	script_stop();
	return targets.getcount() > 0;
}

static void for_each_opponent(int bonus) {
	if(!targets) {
		script_fail = true;
		script_stop();
		return;
	}
	pushvalue push(opponent);
	pushvalue push_rect(last_rect);
	pushvalue push_index(last_index);
	variants commands; commands.set(script_begin, script_end - script_begin);
	creaturea source(targets);
	for(auto p : source) {
		opponent = p;
		auto pt = p->getposition();
		last_rect = pt.rectangle();
		last_index = pt;
		script_run(commands);
	}
	script_stop();
}

static void for_each_creature(int bonus) {
	if(!targets) {
		script_fail = true;
		script_stop();
		return;
	}
	pushvalue push(player);
	pushvalue push_rect(last_rect);
	pushvalue push_index(last_index);
	variants commands; commands.set(script_begin, script_end - script_begin);
	creaturea source(targets);
	for(auto p : source) {
		player = p;
		auto pt = p->getposition();
		last_rect = {pt.x, pt.y, pt.x, pt.y};
		last_index = pt;
		script_run(commands);
	}
	script_stop();
}

static void for_each_item(int bonus) {
	if(!items) {
		script_fail = true;
		script_stop();
		return;
	}
	pushvalue push(last_item);
	variants commands; commands.set(script_begin, script_end - script_begin);
	itema source(items);
	for(auto p : source) {
		last_item = p;
		script_run(commands);
	}
	script_stop();
}

static bool is_features(int bonus) {
	script_stop();
	return indecies.getcount() > 0;
}

static bool is_items(int bonus) {
	script_stop();
	return items.getcount() > 0;
}

static void for_each_feature(int bonus) {
	if(!indecies) {
		script_fail = true;
		script_stop();
		return;
	}
	auto push_source(indecies);
	auto push_rect = last_rect;
	auto push_index = last_index;
	variants commands; commands.set(script_begin, script_end - script_begin);
	for(auto p : indecies) {
		last_rect = {p.x, p.y, p.x, p.y};
		last_index = p;
		script_run(commands);
	}
	script_stop();
	last_index = push_index;
	last_rect = push_rect;
	indecies = push_source;
}

static bool is_room(int bonus) {
	script_stop();
	return rooms.getcount() > 0;
}

static void for_each_room(int bonus) {
	if(!rooms) {
		script_fail = true;
		script_stop();
		return;
	}
	rooma push_source(rooms);
	pushvalue push(last_room);
	pushvalue push_rect(last_rect);
	pushvalue push_index(last_index);
	variants commands; commands.set(script_begin, script_end - script_begin);
	for(auto p : rooms) {
		last_room = p;
		last_rect = p->rc;
		last_index = center(last_rect);
		script_run(commands);
	}
	script_stop();
	rooms = push_source;
}

static void gain_coins(int bonus) {
	auto value = bonus * 10;
	fix_yellow("%1i %Coins", value);
	player->money += value;
}

static void empthy_script(int bonus) {
}

static void opponent_next(int bonus) {
	auto push_player = player;
	auto push_opponent = opponent;
	iswap(player, opponent);
	auto v = next_script();
	script_run(v);
	opponent = push_opponent;
	player = push_player;
}

static void add_anger(int bonus) {
	if(!bonus)
		bonus = 1;
	if(bonus > 0)
		bonus = xrand(bonus, bonus * 3);
	player->add(Mood, -bonus);
}

static bool have_object(variant v) {
	if(v.iskind<itemi>())
		return player->useitem(bsdata<itemi>::begin() + v.value, false);
	else if(v.iskind<abilityi>())
		return player->abilities[v.value] >= v.counter;
	else if(v.iskind<feati>())
		return player->is((feat_s)v.value);
	else if(v.iskind<listi>()) {
		for(auto v : bsdata<listi>::elements[v.value].elements) {
			if(have_object(v))
				return true;
		}
	} else if(v.iskind<randomizeri>()) {
		for(auto v : bsdata<randomizeri>::elements[v.value].chance) {
			if(have_object(v))
				return true;
		}
	}
	return false;
}

static bool is_have_next(int bonus) {
	return have_object(*script_begin++) == (bonus >= 0);
}

static void have_next(int bonus) {
	if(!is_have_next(bonus))
		script_stop();
}

static bool is_animal(int bonus) {
	return !player->canspeak();
}

static const char* get_header_id() {
	if(last_action)
		return last_action->getid();
	if(last_spell)
		return last_spell->getid();
	if(last_item)
		return last_item->geti().id;
	return "Common";
}

static void set_magic_effect(magicn v) {
	switch(v) {
	case Artifact: effect_level = 10; break;
	case Blessed: effect_level = 4; break;
	default: effect_level = 1; break;
	}
}

static void check_script_targets() {
	if(!have_targets()) {
		script_stop();
		player->actp("YouDontValidTargets");
	}
}

static void choose_limit(int counter) {
	counter = script_count(counter, 1);
	if(counter <= 0)
		return;
	if(targets.getcount() > (size_t)counter)
		targets.count = counter;
	if(items.getcount() > (size_t)counter)
		items.count = counter;
	if(rooms.getcount() > (size_t)counter)
		rooms.count = counter;
	if(indecies.getcount() > (size_t)counter)
		indecies.count = counter;
}

static bool choose_specific_target() {
	if(!specific_target)
		return false;
	auto i = targets.find(specific_target);
	if(i != -1) {
		iswap(targets.data[0], targets.data[i]);
		return true;
	}
	i = rooms.find(specific_target);
	if(i != -1) {
		iswap(rooms.data[0], rooms.data[i]);
		return true;
	}
	i = items.find(specific_target);
	if(i != -1) {
		iswap(items.data[0], items.data[i]);
		return true;
	}
	return false;
}

static void choose_target(int bonus) {
	if(!choose_specific_target()) {
		if(player->ishuman())
			choose_target_interactive(get_header_id());
	}
	choose_limit(1);
	check_script_targets();
}

static void choose_random(int bonus) {
	if(items)
		items.shuffle();
	if(rooms)
		rooms.shuffle();
	if(targets)
		targets.shuffle();
	if(indecies)
		indecies.shuffle();
	choose_limit(bonus);
}

static void enchant_minutes(int count, int koeff, const char* format) {
	auto v = *script_begin++;
	auto minutes = count * koeff;
	if(minutes <= 0)
		return;
	auto stop_time = game.getminutes() + minutes;
	if(format) {
		char temp[260]; stringbuilder sb(temp);
		sb.addv(v.getname(), 0);
		sb.adds("%1i %-2", count, getnm(str_count(format, count)));
		player->fixvalue(temp, ColorGreen);
	}
	add_boost(player, v, stop_time);
}

static void enchant_minutes(int bonus) {
	enchant_minutes(script_count(bonus), 1, "Minute");
}

static void enchant_hours(int bonus) {
	enchant_minutes(script_count(bonus), 60, "Hour");
}

static void enchant_days(int bonus) {
	enchant_minutes(script_count(bonus), 24 * 60, "Day");
}

static void make_alarm(int range) {
	auto start = player->getposition();
	for(auto& e : bsdata<creature>()) {
		if(!e || !e.ispresent() || e.moveorder.x >= 0)
			continue;
		if(e.ishuman())
			continue;
		if(!e.isevil() && !e.isgood())
			continue;
		if(start.range(e.getposition()) > range)
			continue;
		e.moveorder = start;
	}
}

static void make_noise(int bonus) {
	make_alarm(1000);
}

static void steal_opponent_coins(int bonus) {
	auto coins = opponent->money;
	if(!coins)
		return;
	auto maximum = xrand((last_value + 1) * 50, (last_value + 1) * 100);
	if(coins > maximum)
		coins = maximum;
	player->money += coins;
	opponent->money -= coins;
	gain_experience((coins + 99) / 100);
}

static void add_reputation(int bonus) {
	player->add(Reputation, bonus);
}

static void add_mana(int bonus) {
	player->add(Mana, bonus, 0, player->basic.abilities[Mana]);
}

static void create_hero(int bonus) {
	bonus = script_count(bonus);
	auto random_race = bsdata<randomizeri>::find("RandomRace");
	if(!random_race)
		return;
	for(auto i = 0; i < bonus; i++) {
		auto v = single(random_race);
		player_create(area->get(last_rect), v, false);
	}
}

static void test_manual(int bonus) {
}

static void repeat_use_item(int bonus) {
	auto& v = player->wears[last_player_used_wear];
	if(!v)
		return;
	use_item(v);
}

static bool isf(unsigned source, int v) {
	return (source & (1 << v)) != 0;
}

static void consume_ingridients(const itemi* pi, magicn magic, int& count, bool run) {
	if(count <= 0)
		return;
	for(auto& e : player->backpack()) {
		if(&e.geti() != pi)
			continue;
		if(!e.isidentified())
			continue;
		if(e.getmagic() != magic)
			continue;
		auto count_allow = e.getcount();
		if(count_allow >= count) {
			if(run)
				e.setcount(count_allow - count);
			count = 0;
		} else {
			count -= count_allow;
			if(run)
				e.setcount(0);
		}
		if(!count)
			break;
	}
}

static bool consume_ingridients(const itemi* pi, int count, bool run) {
	if(count <= 0)
		return true;
	consume_ingridients(pi, Mundane, count, run);
	consume_ingridients(pi, Blessed, count, run);
	consume_ingridients(pi, Artifact, count, run);
	return count == 0;
}

static bool consume_ingridients(const listi* source, bool run) {
	if(!source)
		return false;
	for(auto v : source->elements) {
		if(v.iskind<itemi>()) {
			if(!consume_ingridients(bsdata<itemi>::elements + v.value, v.counter, run))
				return false;
		}
	}
	return true;
}

static void select_craft_items(crafti* craft, unsigned allowed_items) {
	auto index = 0;
	an.clear();
	for(auto v : craft->elements) {
		if(!isf(allowed_items, index++))
			continue;
		if(v.iskind<itemi>()) {
			auto pi = bsdata<itemi>::elements + v.value;
			auto ingridients = get_ingridient_list(pi);
			if(!consume_ingridients(ingridients, false))
				continue;
			an.add(pi, v.getname());
		}
	}
}

static void use_craft(int bonus) {
	auto craft = bsdata<crafti>::elements + last_craft;
	auto known_items = player->receipts[last_craft];
	select_craft_items(craft, known_items);
	if(!an) {
		if(!known_items)
			player->actp("NoCraftItems", craft->id);
		else
			player->actp("NoIngridients", craft->id);
		script_fail = true;
		return;
	}
	auto pi = (itemi*)choose_answers(getnm(craft->id), getnm("Cancel"));
	if(!pi)
		return;
	if(!consume_ingridients(get_ingridient_list(pi), true))
		return;
	auto skill_value = player->get(craft->skill);
	auto magic = Mundane;
	if(skill_value < 30) {
		if(!player->roll(craft->skill, 50))
			magic = Cursed;
	} else {
		if(player->roll(craft->skill, -30)) {
			magic = Blessed;
			if(skill_value > 90) {
				if(player->roll(craft->skill, -90))
					magic = Artifact;
			}
		}
	}
	add_item(craft_item(pi, magic));
}

static bool is_trade_items(int bonus) {
	auto room = player->getroom();
	if(!room)
		return false;
	auto keeper = room->getowner();
	if(!keeper)
		return false;
	if(keeper == player)
		return false;
	if(player->getposition().range(keeper->getposition()) > 1)
		return false;
	auto index = room->getsellitems();
	auto index_special = room->getspecialsellitems();
	for(auto& e : area->items) {
		if(!e)
			continue;
		if((e.position == index) || (e.position == index_special))
			return true;
	}
	return false;
}

static void make_trade(int bonus) {
	auto room = player->getroom();
	if(!room)
		return;
	pushvalue push_opponent(opponent);
	opponent = room->getowner();
	if(!opponent)
		return;
	auto index = room->getsellitems();
	auto index_special = room->getspecialsellitems();
	auto payment_cost = player->getpaymentcost();
	an.clear();
	auto format = getnm("AskBuyItem");
	for(auto& e : area->items) {
		if(!e)
			continue;
		if(e.position == index)
			an.add(&e, format, e.getname(), e.getcost(payment_cost));
		else if(e.position == index_special)
			an.add(&e, format, e.getname(), e.getcost(payment_cost));
	}
	if(!an) {
		opponent->speak("NoBuyItem");
		return;
	}
	clear_console();
	opponent->speak("BuyItem");
	an.add(0, getnm("NoWantTrade"));
	auto pv = choose_answers();
	if(!pv)
		return;
	if(area->items.have(pv)) {
		auto pi = (itemground*)pv;
		auto cost = pi->getcost(payment_cost);
		if(payment(player, opponent, pi->getname(), cost)) {
			if(pi->position.x == get_token(InRoomToBuySpecial)) {
				item it = *pi;
				it.setcount(1);
				player->additem(it);
				if(!it)
					pi->setcount(pi->getcount() - 1);
			} else {
				item it = *pi; // Make copy. Original don't change.
				player->additem(it);
			}
		}
	}
}

static void add_items(collectiona& result, point index) {
	for(auto& e : area->items) {
		if(!e || e.position != index)
			continue;
		result.add((void*)&e.geti());
	}
}

static void make_selling(int bonus) {
	auto room = player->getroom();
	if(!room)
		return;
	pushvalue push_opponent(opponent);
	opponent = room->getowner();
	if(!opponent)
		return;
	collectiona items_unlimined, items;
	add_items(items_unlimined, room->getsellitems());
	add_items(items, room->getspecialsellitems());
	auto payment_cost = player->getsellingcost();
	an.clear();
	auto format = getnm("AskSellItem");
	for(auto& e : player->backpack()) {
		if(!e)
			continue;
		auto pi = (void*)&e.geti();
		if(items_unlimined.find(pi) == -1 && items.find(pi) == -1)
			continue;
		auto cost = e.getcost(payment_cost);
		if(!cost)
			continue;
		an.add(&e, format, e.getname(), cost);
	}
	if(!an) {
		opponent->speak("NoSellItem");
		return;
	}
	clear_console();
	opponent->speak("SellItem");
	an.add(0, getnm("NoWantTrade"));
	auto pv = choose_answers();
	if(!pv)
		return;
	if(player->iswear(pv)) {
		auto pi = (item*)pv;
		auto cost = pi->getcost(payment_cost);
		if(selling(player, opponent, pi->getname(), cost, "WantSellItem")) {
			if(items.find((void*)&pi->geti()) != -1) {
				item it = *pi; it.setcount(1);
				it.drop(room->getspecialsellitems());
			}
			pi->setcount(pi->getcount() - 1);
		}
	}
}

static int find_craft_index(variant v) {
	auto index = 0;
	for(auto e : bsdata<crafti>::elements[last_craft].elements) {
		if(e.issame(v))
			return index;
		index++;
	}
	return -1;
}

static void add_drunk(int bonus) {
	if(bonus > 0) {
		if(player->is(PoisonImmunity))
			return;
		if(player->is(PoisonResistance))
			bonus = bonus / 2;
	}
	player->add(Drunk, bonus, 0);
}

static void add_craft(int bonus) {
	player->receipts[last_craft] |= (1 << bonus);
}

static bool prohibited_action_effect(const char* id) {
	for(auto p : creatures) {
		if(!(*p))
			continue;
		if(!p->isevil() && !p->isgood())
			continue;
		if(p == player || p->ishuman())
			continue;
		if(p->getenemy())
			continue;
		p->add(Mood, -xrand(3, 6));
		if(p->abilities[Mood] < 0 && d100() < -p->abilities[Mood] * 2)
			make_hostile(p, player);
		else {
			if(!p->speak(id, "ProhibitedAction"))
				p->speak("ProhibitedAction");
		}
		return true;
	}
	return false;
}

static void prohibited_action(int bonus) {
	if(!last_action)
		return;
	if(prohibited_action_effect(last_action->id))
		script_stop();
}

static void charm_opponent(int bonus) {
	if(bonus>=0)
		opponent->setcharmer(player);
	else
		opponent->setcharmer(0);
}

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
	{"AddCraft", add_craft},
	{"AddDrunk", add_drunk},
	{"AddDungeonRumor", add_dungeon_rumor},
	{"AddNeed", add_need},
	{"AddNeedAnswers", add_need_answers},
	{"AddReputation", add_reputation},
	{"AddMana", add_mana},
	{"ApplyAction", apply_action},
	{"AnimalInt", empthy_script, is_animal},
	{"Anger", add_anger},
	{"CastSpell", cast_spell},
	{"CastLastSpell", cast_last_spell},
	{"Chance", random_chance},
	{"CharmOpponent", charm_opponent},
	{"Chatting", chatting},
	{"ChooseTarget", choose_target, is_full},
	{"ChooseRandom", choose_random, is_full},
	{"ChooseLimit", choose_limit, is_full},
	{"ColdHarm", cold_harm},
	{"CreateHero", create_hero},
	{"CurseItem", curse_item},
	{"Damage", damage_all},
	{"DamageItem", damage_item},
	{"DebugMessage", debug_message},
	{"DestroyFeature", destroy_feature},
	{"DestroyWall", destroy_wall},
	{"DetectItems", detect_items, empthy_next_condition},
	{"DropDown", drop_down},
	{"EnchantMinutes", enchant_minutes, empthy_next_condition},
	{"EnchantHours", enchant_hours, empthy_next_condition},
	{"EnchantDays", enchant_days, empthy_next_condition},
	{"ExploreArea", explore_area},
	{"ProhibitedAction", prohibited_action},
	{"FireHarm", fire_harm},
	{"ForEachCreature", for_each_creature, is_targets},
	{"ForEachFeature", for_each_feature, is_features},
	{"ForEachItem", for_each_item, is_items},
	{"ForEachOpponent", for_each_opponent, is_targets},
	{"ForEachRoom", for_each_room, is_room},
	{"GainCoins", gain_coins},
	{"GainExperience", gain_experience},
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
	{"Harm", harm_player},
	{"HaveNext", have_next, is_have_next},
	{"Heal", heal_player},
	{"HealAll", heal_all},
	{"HealPoison", heal_poison, is_heal_poison},
	{"HealWounded", heal_wounded, is_heal_wounded},
	{"IdentifyItem", identify_item},
	{"Inventory", inventory},
	{"JumpToSite", jump_to_site},
	{"LoseGame", lose_game},
	{"LockAllDoors", lock_all_doors},
	{"MakeNoise", make_noise},
	{"MakeScreenshoot", make_screenshoot},
	{"MakeSelling", make_selling, is_trade_items},
	{"MakeTrade", make_trade, is_trade_items},
	{"MarkRoom", mark_room},
	{"MoveDown", move_down},
	{"MoveDownLeft", move_down_left},
	{"MoveDownRight", move_down_right},
	{"MoveLeft", move_left},
	{"MoveRight", move_right},
	{"MoveUp", move_up},
	{"MoveUpRight", move_up_right},
	{"MoveUpLeft", move_up_left},
	{"Offset", set_offset},
	{"OpenNearestDoor", open_nearest_door},
	{"Opponent", opponent_next},
	{"PickUp", pickup},
	{"PickUpAll", pickup_all},
	{"QuestGuardian", quest_guardian},
	{"QuestLandscape", quest_landscape},
	{"QuestMinion", quest_minion},
	{"QuestReward", quest_reward},
	{"RaiseSkills", raise_skills},
	{"RandomAbility", random_ability},
	{"RangeAttack", attack_range},
	{"RepairItem", repair_item},
	{"RemoveFeature", remove_feature},
	{"RepeatUseItem", repeat_use_item},
	{"Roll", roll_value},
	{"RollAction", roll_action},
	{"RollLearning", roll_learning},
	{"ShowImages", show_images},
	{"ShowLogs", show_logs},
	{"SiteFloor", site_floor},
	{"SiteWall", site_wall},
	{"SomeCoins", some_coins},
	{"StealOpponentCoins", steal_opponent_coins},
	{"TransferCoins", transfer_coins, is_transfer_coins},
	{"TestArena", test_arena},
	{"TestEnemyChange", test_enemy_change},
	{"TestRumor", test_rumor},
	{"TestManual", test_manual},
	{"ThrownAttack", attack_thrown},
	{"ToggleFloorRect", toggle_floor_rect},
	{"ViewStuff", view_stuff},
	{"WaitHour", wait_hour},
	{"WinGame", win_game},
	{"Wounded", standart_filter, is_wounded},
	{"UseItem", use_item},
	{"UseCraft", use_craft},
};
BSDATAF(script);