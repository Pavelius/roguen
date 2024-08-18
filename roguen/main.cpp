#include "areapiece.h"
#include "bsreq.h"
#include "charname.h"
#include "draw.h"
#include "game.h"
#include "greatneed.h"
#include "creature.h"
#include "hotkey.h"
#include "siteskill.h"
#include "speech.h"
#include "talk.h"
#include "textscript.h"

void dialog_message(const char* format);

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

static void initializating() {
	bsreq::read("rules/Basic.txt");
	bsreq::read("rules/Tiles.txt");
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Features.txt");
	bsreq::read("rules/Monsters.txt");
	shapei::read("rules/Shapes.txt");
	bsreq::read("rules/Spells.txt");
	bsreq::read("rules/Sites.txt");
	bsreq::read("rules/Advancement.txt");
	bsreq::read("rules/Skills.txt");
	readl("NameCharacters", charname::read);
	speech_initialize();
	talki::read();
	readurl("modules", "*.txt", bsreq::read);
	check_need_loading();
	hotkey_initialize();
	site_skills_initialize();
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
	player = player_create({5, 5}, "DarkElf", true);
	player->set(Ally);
	equip_item("LongBow");
	equip_item("Arrow");
	add_item("Torch");
	add_item("Ration", 20);
	add_item("Bones", 60);
	add_item("OrnamentalStones", 3);
	add_item("HealingPotion", 5, true);
	add_item("PotionOfStrenght", 2, true);
	add_item("RingOfWarrior");
	add_item("RingOfProtection");
	add_item("RingOfFireResistance");
	add_item("RingOfLevitation");
	add_item("RingOfRegeneration");
	add_item("HandPick");
	add_item("PotionOfLearning", 1, true);
	player->add(Herbalism, 25);
	player->wears[MeleeWeapon].createpower(100);
	player->wears[MeleeWeapon].setidentified(1);
	equip_item("Bones");
	game.setowner(player);
	game.newgame();
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	auto seed = getcputime();
	//auto seed = 157145343; // Error dungeon room
	string_initialize();
	answers::console = &console;
	logv(str("Seed is %1i", seed), 0, 0, false);
	srand(seed);
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}