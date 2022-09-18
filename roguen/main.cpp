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

static void horz(int x1, int y1, int x2, tile_s tile) {
	while(x1 <= x2) {
		if(x1 < 0)
			continue;
		if(x1 >= mps)
			continue;
		auto i = m2i(x1, y1);
		area.tiles[i] = tile;
		x1++;
	}
}

static void vert(int x1, int y1, int y2, tile_s tile) {
	while(y1 <= y2) {
		if(y1 < 0)
			continue;
		if(y1 >= mps)
			continue;
		auto i = m2i(x1, y1);
		area.tiles[i] = tile;
		y1++;
	}
}

static void random(int x1, int y1, int x2, tile_s tile) {
	auto x = xrand(x1, x2);
	auto i = m2i(x, y1);
	area.set(i, tile);
}

static void random(int x1, int y1, int x2, feature_s tile) {
	auto x = xrand(x1, x2);
	auto i = m2i(x, y1);
	area.set(i, tile);
}

static void setdoor(indext i) {
	area.set(i, WoodenFloor);
	area.set(i, Door);
	//area.set(i, Activated);
}

static void place_building(const rect& rc, tile_s wall) {
	auto& ei = bsdata<tilei>::elements[wall];
	if(!ei.iswall())
		return;
	area.set(rc, ei.tile);
	horz(rc.x1, rc.y1, rc.x2, wall);
	vert(rc.x1, rc.y1, rc.y2, wall);
	horz(rc.x1, rc.y2, rc.x2, wall);
	vert(rc.x2, rc.y1, rc.y2, wall);
	setdoor(m2i(xrand(rc.x1 + 1, rc.x2 - 1), rc.y2));
	setdoor(m2i(rc.x1, xrand(rc.y1 + 1, rc.y2 - 1)));
}

static void main_start() {
	area.clear();
	area.set({0, 0, mps, mps}, Grass);
	player = creature::create(m2i({5, 5}), "Human");
	player->set(Ally);
	//auto p2 = creature::create(m2i({3, 3}), "Goblin");
	//p2->set(Enemy);
	//area.set({2, 2, 6, 7}, GrassCorupted);
	place_building({7, 2, 12, 7}, WallBuilding);
	area.set(m2i({15, 1}), WoodenFloor);
	area.set(m2i({0, 1}), Webbed);
	area.set(m2i({4, 4}), Webbed);
	area.set(m2i({4, 4}), Iced);
	area.set(m2i({4, 5}), Iced);
	area.set(m2i({5, 5}), Blooded);
	area.set(m2i({6, 5}), Blooded);
	area.set(m2i({6, 6}), Blooded);
	area.set(m2i({2, 4}), Tree);
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