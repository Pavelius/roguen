#include "areapiece.h"
#include "bsreq.h"
#include "draw.h"
#include "game.h"
#include "greatneed.h"
#include "creature.h"
#include "hotkey.h"
#include "log.h"
#include "rand.h"
#include "siteskill.h"
#include "textscript.h"
#include "timer.h"

using namespace draw;

static void add_item(const char* id, int count = 1, bool identified = false) {
	item it;
	it.create(bsdata<itemi>::find(id), count);
	if(identified)
		it.setidentified(1);
	player->equip(it);
}

static void test_shapes() {
	static rect cv[] = {
		{4, 2, 17, 19}, // North
		{0, 3, 15, 18}, // East
		{2, 1, 17, 16}, // South
		{1, 0, 16, 15}, // West
	};
	auto p = bsdata<shapei>::find("SmallRoom");
	auto d = West;
	rect r = {0, 0, 20, 20};
	auto n = p->bounding(r, d);
}

static void main_start() {
	player = player_create({5, 5}, "Human", false);
	player->experience += 600;
	player->money += 500;
	add_item("LongBow");
	add_item("Arrow");
	add_item("Torch");
	add_item("Ration", 10);
	add_item("GrindStone", 1, true);
	add_item("HealingPotion", 5, true);
	add_item("PotionOfStrenght", 2, true);
	add_item("HandPick");
	add_item("Bandage", 3, true);
	add_item("TomeOfLight", 1, true);
	add_item("RodOfBurning", 1, true);
	add_item("ScrollOfGoldDetection", 1, true);
	player->wears[MeleeWeapon].createpower(100);
	player->wears[MeleeWeapon].setidentified(1);
	game.setowner(player);
	test_shapes();
	new_game();
}

int start_application(fnevent proc);

int main(int argc, char *argv[]) {
	auto seed = getcputime();
	//auto seed = 1386226015;
	initialize_strings();
	logv(str("Seed is %1i", seed));
	srand(seed);
	return start_application(main_start);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}