#include "bsreq.h"
#include "draw.h"
#include "main.h"
#include "hotkey.h"

static_assert(sizeof(item) == sizeof(int), "Struct item greater tha integer");

using namespace draw;

void create_area(const char* id);

#ifdef _DEBUG
void main_util();
#endif

static void initializating() {
	bsreq::read("rules/Items.txt");
	bsreq::read("rules/Advancement.txt");
	hotkey::initialize();
	readl("Chats", speech::read);
	shapei::read("rules/Shapes.txt");
	bsreq::read("rules/Sites.txt");
	generatori::read("rules/Tables.txt");
#ifdef _DEBUG
	main_util();
#endif
}

static void random(int x1, int y1, int x2, tile_s tile) {
	auto x = xrand(x1, x2);
	point i = {short(x), short(y1)};
	area.set(i, tile);
}

static void random(int x1, int y1, int x2, feature_s tile) {
	auto x = xrand(x1, x2);
	point i = {short(x), short(y1)};
	area.set(i, tile);
}

static void setdoor(point i, tile_s tile) {
	area.set(i, tile);
	area.set(i, Door);
	//area.set(i, Activated);
}

static void place_building(const rect& rc, tile_s wall) {
	auto& ei = bsdata<tilei>::elements[wall];
	if(!ei.iswall())
		return;
	area.set(rc, ei.tile);
	area.horz(rc.x1, rc.y1, rc.x2, wall);
	area.vert(rc.x1, rc.y1, rc.y2, wall);
	area.horz(rc.x1, rc.y2, rc.x2, wall);
	area.vert(rc.x2, rc.y1, rc.y2, wall);
	setdoor({short(xrand(rc.x1 + 1, rc.x2 - 1)), short(rc.y2)}, ei.tile);
	setdoor({short(rc.x1), short(xrand(rc.y1 + 1, rc.y2 - 1))}, ei.tile);
}

static void create_item(point m, const char* id) {
	auto pi = bsdata<itemground>::add();
	pi->position = m;
	pi->create(id);
}

void show_worldmap();

static void main_start() {
	world.clear();
	world.generate({world.mps / 2, world.mps / 2}, 1);
	//show_worldmap();
	//auto p2 = creature::create(m2i({3, 3}), "Goblin");
	//p2->set(Enemy);
	//area.set({2, 2, 6, 7}, GrassCorupted);
	//place_building({7, 2, 12, 7}, WallCave);
	create_area("LightForest");
	create_item({4, 4}, "Sword");
	create_item({4, 4}, "BattleAxe");
	player = creature::create({5, 5}, "Human");
	player->set(Ally);
	setnext(game.play);
}

int start_application(fnevent proc, fnevent initializing);

int main(int argc, char *argv[]) {
	srand(getcputime());
	return start_application(main_start, initializating);
}

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}