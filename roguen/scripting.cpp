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
	player->act("%����� ������� ��������%�.");
	player->fixaction();
	player->fixeffect("HitVisual");
	player->wait();
}

static void inventory(int bonus) {
	static wear_s source[] = {
		Head, Backward, Torso, MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, ThrownWeapon,
		Elbows, FingerRight, FingerLeft, Gloves, Legs, Ammunition,
	};
	auto push_header = answers::header;
	answers::header = getnm("Inventory");
	answers an;
	for(auto i : source) {
		auto pi = player->getwear(i);
		auto pn = player->getwearname(i);
		if(!pn) {
			pn = "-";
			an.add(pi, pn);
		} else {
			char temp[512]; stringbuilder sb(temp);
			sb.add(pn);
			pi->getinfo(sb, false);
			an.add(pi, temp);
		}
	}
	an.choose("Test answers data", 0);
	answers::header = push_header;
}

static void debug_message(int bonus) {
	console.adds("Sprites %1i", bsdata<draw::object>::source.count);
}

static void open_nearest_door(int bonus) {
	indexa source;
	source.select(i2m(player->getindex()), 2);
	for(auto i : source) {
		if(!area.is(i, Activated))
			area.set(i, Activated);
		else
			area.remove(i, Activated);
	}
}

static const char* getspeech(const char* id) {
	speecha source;
	source.select(id);
	return source.getrandom();
}

static void chat_someone(int bonus) {
	player->say(getspeech("TestYouselfPlease"));
}

BSDATA(script) = {
	{"AttackForward", attack_forward},
	{"ChooseCreature", choose_creature},
	{"ChatSomeone", chat_someone},
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
	{"OpenNearestDoor", open_nearest_door},
};
BSDATAF(script)