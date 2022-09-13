#include "main.h"

areamap			area;
static char		console_text[4096];
stringbuilder	console(console_text);
gamei			game;

static void update_los() {
	if(!player)
		return;
	// Set fog of war
	auto max_count = mps * mps;
	for(auto i = 0; i < max_count; i++)
		area.remove(i, Visible);
	//loc.setlos(player->getposition(), player->getlos());
}

static void decoy_food() {
}

static void growth_plant() {
}

static bool checkalive() {
	if(!player || !player->isalive())
		return false;
	return true;
}

void gamei::all(creature::fnupdate proc) {
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		(e.*proc)();
	}
}

void gamei::passminute() {
	minutes++;
	//boost_update();
	all(&creature::restoration);
	while(restore_half_turn < minutes) {
		all(&creature::checkpoison);
		restore_half_turn += 5;
		area.removechance(Iced, 20);
	}
	while(restore_turn < minutes) {
		all(&creature::checkmood);
		restore_turn += 10;
	}
	while(restore_hour < minutes) {
		all(&creature::checksick);
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
			if(draw::isnext()) {
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