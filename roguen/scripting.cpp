#include "draw_object.h"
#include "main.h"

creaturea creatures, enemies, targets;
creature* enemy;

static void choose_creature(int bonus) {
}

static void choose_targets(target_s target) {
	targets.clear();
	switch(target) {
	case You:
		targets.add(player);
		break;
	case YouOrAlly:
		break;
	}
}

static void move_left(int bonus) {
	player->movestep(West);
}

static void move_right(int bonus) {
	player->movestep(East);
}

static void move_up(int bonus) {
	player->movestep(North);
}

static void move_down(int bonus) {
	player->movestep(South);
}

static void move_up_left(int bonus) {
	player->movestep(NorthWest);
}

static void move_up_right(int bonus) {
	player->movestep(NorthEast);
}

static void move_down_left(int bonus) {
	player->movestep(SouthWest);
}

static void move_down_right(int bonus) {
	player->movestep(SouthEast);
}

static void attack_forward(int bonus) {
	player->act("%герой успешно атаковал%а.");
	player->fixaction();
	player->wait();
}

static void inventory(int bonus) {
	auto push_header = answers::header;
	answers::header = getnm("Inventory");
	answers an;
	an.add((void*)1, "Play");
	an.choose("Test answers data", "Cancel");
	answers::header = push_header;
}

static void debug_message(int bonus) {
	console.adds("Sprites %1i", bsdata<draw::object>::source.count);
}

BSDATA(script) = {
	{"AttackForward", attack_forward},
	{"ChooseCreature", choose_creature},
	{"DebugMessage", debug_message},
	{"MoveDown", move_down},
	{"MoveDownLeft", move_down_left},
	{"MoveDownRight", move_down_right},
	{"MoveLeft", move_left},
	{"MoveRight", move_right},
	{"MoveUp", move_up},
	{"MoveUpRight", move_up_right},
	{"MoveUpLeft", move_up_left},
	{"Inventory", inventory},
};
BSDATAF(script)