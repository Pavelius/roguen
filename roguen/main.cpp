#include "bsreq.h"
#include "draw.h"
#include "main.h"
#include "hotkey.h"

static_assert(sizeof(item) == sizeof(int), "Struct item greater tha integer");

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

void initialize_script();

static void initializating() {
	initialize_script();
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Monsters.txt");
	bsreq::read("rules/Advancement.txt");
	hotkey::initialize();
	readl("Chats", speech::read);
	readl("NameCharacters", charname::read);
	shapei::read("rules/Shapes.txt");
	bsreq::read("rules/Sites.txt");
	talki::read();
#ifdef _DEBUG
	main_util();
#endif
}

static void equip_item(const char* id) {
	item it; it.create(id);
	player->equip(it);
	if(it)
		player->additem(it);
}

static void main_start() {
	player = creature::create({5, 5}, "Human", "Fighter", true);
	player->set(Ally);
	equip_item("LongBow");
	equip_item("Arrow");
	equip_item("Torch");
	game.setowner(player);
	game.newgame();
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	auto seed = getcputime();
	//auto seed = 96115171;
	actable::logv(str("Seed is %1i", seed), 0, 0, false);
	srand(seed);
	//srand(213);
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}