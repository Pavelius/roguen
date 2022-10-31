#include "main.h"

areamap			area;
location		loc;
worldi			world;
gamei			game;
static char		console_text[4096];
stringbuilder	console(console_text);
point			gamei::start_village = {128, 128};

static void update_los() {
	point m;
	for(m.y = 0; m.y < area.mps; m.y++)
		for(m.x = 0; m.x < area.mps; m.x++) {
			area.remove(m, Visible);
			if(area.is(m, Darkened))
				area.remove(m, Explored);
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
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid())
			(e.*proc)();
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
			if((i % 5) == 0)
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

bool gamei::testcount(variant v) {
	if(v.counter < 0 && d100() >= -v.counter)
		return false;
	return true;
}