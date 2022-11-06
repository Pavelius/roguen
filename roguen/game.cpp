#include "main.h"

areamap			area;
areaheadi		areahead;
gamei			game;
static char		console_text[4096];
stringbuilder	console(console_text);
point			gamei::start_village = {128, 128};

static bool isfreelt(point m) {
	if(area.is(m, Darkened))
		return false;
	return area.isfree(m);
}

static void update_los() {
	point m;
	for(m.y = 0; m.y < area.mps; m.y++)
		for(m.x = 0; m.x < area.mps; m.x++) {
			area.remove(m, Visible);
			if(area.is(m, Darkened))
				area.remove(m, Explored);
		}
	auto human = game.getowner();
	if(human) {
		auto i = human->getposition();
		if(area.isvalid(i))
			area.setlos(i, human->getlos(), isfreelt);
	}
}

static void decoy_food() {
}

static void growth_plant() {
}

static bool checkalive() {
	if(!player || !(*player))
		return false;
	return true;
}

int gamei::getrange(point m1, point m2) {
	auto dx = iabs(m1.x - m2.x);
	auto dy = iabs(m1.y - m2.y);
	return (dx > dy) ? dx : dy;
}

void gamei::all(creature::fnupdate proc) {
	if(draw::isnext())
		return;
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid())
			(e.*proc)();
		if(draw::isnext())
			break;
	}
}

void gamei::passminute() {
	minutes++;
	boosti::updateall();
	all(&creature::restoration);
	while(restore_half_turn < minutes) {
		all(&creature::every5minutes);
		restore_half_turn += 5;
		area.removechance(Iced, 20);
	}
	while(restore_turn < minutes) {
		all(&creature::every10minutes);
		restore_turn += 10;
	}
	while(restore_hour < minutes) {
		all(&creature::every1hour);
		restore_hour += 60;
	}
	while(restore_day_part < minutes) {
		decoy_food();
		restore_day_part += 60 * 4;
	}
	while(restore_day < minutes) {
		growth_plant();
		restore_day += 60 * 24;
	}
}

void gamei::playminute() {
	const int moves_per_minute = 6 * 4;
	bool need_continue = true;
	while(need_continue) {
		need_continue = true;
		for(auto i = 0; i < moves_per_minute; i++) {
			update_los();
			all(&creature::makemove);
			if(!game.getowner() || draw::isnext()) {
				need_continue = false;
				break;
			}
		}
		passminute();
	}
}

void gamei::play() {
	while(checkalive() && !draw::isnext())
		game.playminute();
}

void gamei::endgame() {
	actable::actv(console, getnm("PlayerKilled"), 0, 0, 0);
	actable::pressspace();
	writelog();
}

int gamei::getcount(variant v, int minimal) {
	auto count = v.counter;
	if(count < 0 && d100() >= -count)
		return -1;
	if(count < minimal)
		count = minimal;
	return count;
}

int gamei::getpositivecount(variant v) {
	if(v.counter < 1)
		return 1;
	return v.counter;
}

void gamei::setup_globals() {
	for(auto& e : bsdata<globali>())
		globals[bsid(&e)] = e.current;
}

void gamei::setup_rumors(int count) {
	point pt;
	dungeon::add(start_village);
	const int r = 6;
	for(auto i = 0; i < count; i++) {
		pt.x = xrand(start_village.x - r, start_village.x + r);
		pt.y = xrand(start_village.x - r, start_village.x + r);
		dungeon::add(pt);
	}
}

void gamei::randomworld() {
	clear();
	setup_globals();
	setup_rumors(xrand(4, 7));
}

void gamei::set(const globali& e, int v) {
	if(v < e.minimum)
		v = e.minimum;
	if(v > e.maximum)
		v = e.maximum;
	auto cv = get(e);
	if(cv == v)
		return;
	globals[bsid(&e)] = v;
	if(v == e.minimum)
		runscript(e.fail);
	else if(v==e.maximum)
		runscript(e.effect);
}