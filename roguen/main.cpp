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

static void equip_item(const char* id, int count = 1) {
	item it;
	it.create(id, count);
	player->equip(it);
	if(it)
		player->additem(it);
}

static void add_item(const char* id, int count = 1, bool identified = false) {
	item it;
	it.create(id, count);
	if(identified)
		it.setidentified(1);
	player->additem(it);
}

static void main_start() {
	player = player_create({5, 5}, "HightElf", true);
	player->set(Ally);
	player->experience += 600;
	equip_item("LongBow");
	equip_item("Arrow");
	add_item("Torch");
	add_item("Ration", 10);
	add_item("HealingPotion", 5, true);
	add_item("PotionOfStrenght", 2, true);
	add_item("RingOfLevitation", 1, true);
	add_item("RingOfRegeneration");
	add_item("HandPick");
	add_item("PotionOfLearning", 5, true);
	add_item("Bandage", 3, true);
	add_item("TomeOfLight", 1, true);
	add_item("RodOfBurning", 1, true);
	add_item("ScrollOfGoldDetection", 1, true);
	player->add(Herbalism, 25);
	player->wears[MeleeWeapon].createpower(100);
	player->wears[MeleeWeapon].setidentified(1);
	//player->wears[Torso].createpower(100);
	//player->wears[Torso].setidentified(1);
	//player->wears[Torso].set(Blessed);
	game.setowner(player);
	new_game();
}

int start_application(fnevent proc);

int main(int argc, char *argv[]) {
	auto seed = getcputime();
	initialize_strings();
	answers::console = &console;
	logv(str("Seed is %1i", seed));
	srand(seed);
	return start_application(main_start);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}