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
	area.set({0, 0, mps, mps}, Grass);
	player = creature::create(m2i({5, 5}), "Troll");
	player->set(Ally);
	//auto p2 = creature::create(m2i({3, 3}), "Goblin");
	//p2->set(Enemy);
	//area.set({2, 2, 6, 7}, GrassCorupted);
	area.set({7, 2, 12, 7}, Cave);
	area.set(m2i({10, 7}), WallCave);
	area.set(m2i({9, 7}), WallCave);
	area.set(m2i({7, 2}), WallCave);
	area.set(m2i({7, 3}), WallCave);
	area.set(m2i({7, 4}), WallCave);
	area.set(m2i({8, 2}), WallCave);
	area.set(m2i({9, 2}), WallCave);
	area.set(m2i({10, 2}), WallCave);
	area.set(m2i({11, 2}), WallCave);
	area.set(m2i({12, 2}), WallCave);
	area.set(m2i({12, 3}), WallCave);
	area.set(m2i({12, 4}), WallCave);
	area.set(m2i({12, 5}), WallCave);
	area.set(m2i({12, 6}), WallCave);
	area.set(m2i({12, 7}), WallCave);
	area.set(m2i({15, 1}), WoodenFloor);
	area.set(m2i({0, 1}), Webbed);
	area.set(m2i({4, 4}), Webbed);
	area.set(m2i({4, 4}), Iced);
	area.set(m2i({4, 5}), Iced);
	area.set(m2i({5, 5}), Blooded);
	area.set(m2i({6, 5}), Blooded);
	area.set(m2i({6, 6}), Blooded);
	area.set(m2i({2, 4}), Tree);
	area.set(m2i({5, 5}), Tree);
	area.set(m2i({5, 7}), Tree);
	area.set(m2i({6, 6}), FootHill);
	area.set(m2i({6, 3}), FootMud);
	area.set(m2i({4, 7}), Grave);
	area.set(m2i({8, 4}), HiveHole);
	area.set(m2i({8, 5}), Plant);
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