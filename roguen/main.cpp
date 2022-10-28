#include "bsreq.h"
#include "draw.h"
#include "main.h"
#include "hotkey.h"
#include "variantc.h"

static_assert(sizeof(item) == sizeof(int), "Struct item greater tha integer");

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

static void initializating() {
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Monsters.txt");
	bsreq::read("rules/Advancement.txt");
	hotkey::initialize();
	readl("Chats", speech::read);
	readl("NameCharacters", charname::read);
	shapei::read("rules/Shapes.txt");
	bsreq::read("rules/Sites.txt");
#ifdef _DEBUG
	main_util();
#endif
}

static void equip_item(const char* id) {
	item it;
	it.create(id);
	player->equip(it);
}

void show_worldmap();

static void main_start() {
	//world.clear();
	//world.generate({world.mps / 2, world.mps / 2}, 1);
	//show_worldmap();
	geomark::create({128, 128}, "DungeonEntrance", "EvilSite");
	player = creature::create({5, 5}, "HightElf", "Fighter");
	player->set(Ally);
	equip_item("LongBow");
	equip_item("Arrow");
	game.setowner(player);
	game.newgame();
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	srand(getcputime());
	//srand(213);
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}