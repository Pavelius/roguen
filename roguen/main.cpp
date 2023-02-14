#include "bsreq.h"
#include "charname.h"
#include "draw.h"
#include "game.h"
#include "greatneed.h"
#include "creature.h"
#include "hotkey.h"
#include "talk.h"

void dialog_message(const char* format);

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

static bool test_creatures() {
	collection<creature> source;
	source.select();
	source.match(fntis<creature, &creature::ishuman>, true);
	return source.getcount() == 1;
}

static void initializating() {
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Tiles.txt");
	bsreq::read("rules/Features.txt");
	bsreq::read("rules/Monsters.txt");
	shapei::read("rules/Shapes.txt");
	bsreq::read("rules/Spells.txt");
	bsreq::read("rules/Sites.txt");
	bsreq::read("rules/Advancement.txt");
	bsreq::read("rules/Skills.txt");
	readl("Chats", speech::read);
	readl("NameCharacters", charname::read);
	talki::read();
	readurl("modules", "*.txt", bsreq::read);
	check_need_loading();
	hotkey::initialize();
#ifdef _DEBUG
	main_util();
#endif
}

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
	add_item("Torch");
	add_item("Ration", 20);
	add_item("Bones", 60);
	add_item("OrnamentalStones", 3);
	add_item("HealingPotion");
	add_item("PotionOfStrenght", 2, true);
	add_item("RingOfWarrior");
	add_item("RingOfProtection");
	add_item("RingOfFireResistance");
	add_item("RingOfLevitation");
	player->add(Herbalism, 25);
	player->wears[MeleeWeapon].createpower(100);
	equip_item("Bones");
	game.setowner(player);
	if(!test_creatures())
		return;
	game.newgame();
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	auto seed = getcputime();
	// auto seed = 795343140; // Error dungeon room
	answers::console = &console;
	actable::logv(str("Seed is %1i", seed), 0, 0, false);
	srand(seed);
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}