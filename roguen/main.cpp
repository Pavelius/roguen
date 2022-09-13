#include "bsreq.h"
#include "draw.h"
#include "main.h"

static_assert(sizeof(item) == sizeof(int), "Struct item greater tha integer");

using namespace draw;

#ifdef _DEBUG
void main_util();
#endif

static void initializating() {
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Advancement.txt");
#ifdef _DEBUG
	main_util();
#endif
}

static creature* create_monster(indext index, const char* id) {
	auto pm = bsdata<monsteri>::find(id);
	if(!pm)
		return 0;
	return creature::create(index, pm);
}

static creature* create_character(indext index, const char* id) {
	auto pm = bsdata<racei>::find(id);
	if(!pm)
		return 0;
	return creature::create(index, pm, Male);
}

static void paint_character() {
	auto pa = gres(res::PCArms);
	auto pc = gres(res::PCAccessories);
	auto pb = gres(res::PCBody);
	image(pc, 4, 0); // Missile
	image(pc, 1, 0); // Cloack
	image(pa, 9, 0); // Arm
	image(pb, 0, 0); // Body
	image(pc, 3, 0); // Throwing
	image(pa, 4, 0); // Left Arms
}

static surface& testsurface() {
	static surface bitmap("D:/games/adom/gfx/adom/maps/dungeon/features/tree.png");
	return bitmap;
}

static void test_bitmap() {
	auto& sf = testsurface();
	canvas->blit(10, 10, sf.width, sf.height, 0, sf, 0, 0);
}

static void main_start() {
	game.clear();
	game.set(0, Grass, mps, mps);
	player = create_character(m2i({5, 5}), "Troll");
	create_monster(m2i({3, 3}), "Goblin");
	game.set(m2i({4, 4}), Webbed);
	game.set(m2i({4, 4}), Iced);
	game.set(m2i({4, 5}), Iced);
	game.set(m2i({5, 5}), Blooded);
	game.set(m2i({6, 5}), Blooded);
	game.set(m2i({6, 6}), Blooded);
	game.set(m2i({2, 4}), Tree);
	game.set(m2i({5, 5}), Tree);
	game.set(m2i({6, 3}), Tree);
	game.set(m2i({7, 7}), Tree);
	game.set(m2i({6, 6}), FootHill);
	game.set(m2i({7, 3}), FootMud);
	game.set(m2i({4, 7}), Grave);
	game.set(m2i({7, 2}), HiveHole);
	game.set(m2i({8, 2}), Plant);
	game.set(m2i({5, 3}), Herbs);
	game.set(m2i({5, 5}), Trap);
	auto p1 = bsdata<itemground>::add();
	p1->index = m2i({8, 3});
	p1->create(bsdata<itemi>::find("Sword"), 1);
	//scene(test_bitmap);
	setnext(game.play);
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}