#include "bsreq.h"
#include "draw.h"
#include "main.h"
#include "hotkey.h"

static_assert(sizeof(item) == sizeof(int), "Struct item greater tha integer");

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

static void initializating() {
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Advancement.txt");
	hotkey::initialize();
#ifdef _DEBUG
	main_util();
#endif
}

static void main_start() {
	area.clear();
	area.set(0, Grass, mps, mps);
	player = creature::create(m2i({5, 5}), "Troll");
	creature::create(m2i({3, 3}), "Goblin");
	area.set(m2i({4, 4}), Webbed);
	area.set(m2i({4, 4}), Iced);
	area.set(m2i({4, 5}), Iced);
	area.set(m2i({5, 5}), Blooded);
	area.set(m2i({6, 5}), Blooded);
	area.set(m2i({6, 6}), Blooded);
	area.set(m2i({2, 4}), Tree);
	area.set(m2i({5, 5}), Tree);
	area.set(m2i({6, 3}), Tree);
	area.set(m2i({7, 7}), Tree);
	area.set(m2i({6, 6}), FootHill);
	area.set(m2i({7, 3}), FootMud);
	area.set(m2i({4, 7}), Grave);
	area.set(m2i({7, 2}), HiveHole);
	area.set(m2i({8, 2}), Plant);
	area.set(m2i({5, 3}), Herbs);
	area.set(m2i({5, 5}), Trap);
	auto p1 = bsdata<itemground>::add();
	p1->index = m2i({8, 3});
	p1->create(bsdata<itemi>::find("Sword"), 1);
	setnext(game.play);
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}