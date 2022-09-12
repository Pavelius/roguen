#include "draw.h"
#include "main.h"

using namespace draw;

void main_util();
void paint_floor();
void add_feature(indext i, void* data, unsigned char priority = 10);
int start_application(fnevent proc, fnevent initializing);

static void initializating() {
	main_util();
}

static creature* create_monster(indext index, const char* id) {
	auto pm = bsdata<monsteri>::find(id);
	if(!pm)
		return 0;
	return creature::create(index, pm);
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
	area.clear();
	area.set(0, Grass, mps, mps);
	player = create_monster(m2i({5, 5}), "Ettin");
	create_monster(m2i({3, 3}), "Goblin");
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
	area.set(m2i({4, 7}), FootMud);
	//scene(test_bitmap);
	adventure_mode();
}

int main(int argc, char *argv[]) {
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}