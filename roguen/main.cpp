#include "bsreq.h"
#include "charname.h"
#include "draw.h"
#include "greatneed.h"
#include "main.h"
#include "hotkey.h"

static_assert(sizeof(item) == sizeof(int), "Struct item greater tha integer");
void dialog_message(const char* format);

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

static void initializating() {
	bsreq::read("rules/Tiles.txt");
	bsreq::read("rules/Features.txt");
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Monsters.txt");
	bsreq::read("rules/Advancement.txt");
	hotkey::initialize();
	readl("Chats", speech::read);
	readl("NameCharacters", charname::read);
	shapei::read("rules/Shapes.txt");
	bsreq::read("rules/Spells.txt");
	bsreq::read("rules/Sites.txt");
	talki::read();
	readurl("modules", "*.txt", bsreq::read);
	check_need_loading();
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

static creature* find_monster_id(const char* id) {
	variant v = bsdata<monsteri>::find(id);
	if(!v)
		return 0;
	for(auto& e : bsdata<creature>()) {
		if(e.iskind(v))
			return &e;
	}
	return 0;
}

static void main_start() {
	player = creature::create({5, 5}, "Human", "Fighter", true);
	player->set(Ally);
	equip_item("LongBow");
	equip_item("Arrow");
	equip_item("Torch");
	for(auto i =0; i<20; i++)
		equip_item("Ration");
	for(auto i = 0; i < 60; i++)
		equip_item("Bones");
	equip_item("OrnamentalStones");
	game.setowner(player);
	game.newgame();
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	auto seed = getcputime();
	//auto seed = 99738890;
	answers::console = &console;
	actable::logv(str("Seed is %1i", seed), 0, 0, false);
	srand(seed);
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}