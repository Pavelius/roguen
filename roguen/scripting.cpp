#include "draw_object.h"
#include "main.h"

creaturea creatures, enemies, targets;
creature* enemy;

void animate_figures();

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
	player->fixeffect("HitVisual");
	player->wait();
}

static item* choose_wear() {
	static wear_s source[] = {
		Head, Backward, Torso, MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, ThrownWeapon,
		Elbows, FingerRight, FingerLeft, Gloves, Legs, Ammunition,
	};
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
	return (item*)an.choose(getnm("Inventory"), getnm("Cancel"));
}

static item* choose_stuff(wear_s wear) {
	answers an;
	char temp[512]; stringbuilder sb(temp);
	for(auto i = Backpack; i <= BackpackLast; i = (wear_s)(i+1)) {
		auto pi = player->getwear(i);
		if(!(*pi))
			continue;
		if(wear && !pi->is(wear))
			continue;
		sb.clear();
		pi->getinfo(sb, true);
		an.add(pi, temp);
	}
	sb.clear();
	sb.add("%Choose %1", bsdata<weari>::elements[wear].getname());
	return (item*)an.choose(temp, getnm("Cancel"));
}

static void inventory(int bonus) {
	while(true) {
		auto pi = choose_wear();
		if(!pi)
			break;
		auto owner = pi->getowner();
		if(!owner)
			break;
		if((*pi))
			player->additem(*pi);
		else {
			auto ni = choose_stuff(owner->getwearslot(pi));
			if(ni)
				iswap(*ni, *pi);
		}
	}
}

static const char* getspeech(const char* id) {
	speecha source;
	source.select(id);
	return source.getrandom();
}

static void debug_message(int bonus) {
	player->say(getspeech("TestYouselfPlease"));
	actable::pressspace();
}

static void open_nearest_door(int bonus) {
	indexa source;
	source.select(player->getposition(), 2);
	for(auto i : source) {
		if(!area.is(i, Activated))
			area.set(i, Activated);
		else
			area.remove(i, Activated);
	}
}

static void chat_someone(int bonus) {
	player->say(getspeech("TestYouselfPlease"));
}

static void pickup(int bonus) {
	itema items;
	items.select(player->getposition());
	if(!items)
		return;
	auto p = items.choose(getnm("PickItem"));
	if(p) {
		player->act(getnm("PickupItem"), p->getname());
		player->additem(*p);
	}
}

static void dropdown(int bonus) {
	itema items;
	items.selectbackpack(player);
	if(!items)
		return;
	auto p = items.choose(getnm("DropItem"));
	if(p) {
		player->act(getnm("DropdownItem"), p->getname());
		p->drop(player->getposition());
	}
}

static void view_stuff(int bonus) {
}

static void explore_area(int bonus) {
	point m;
	for(m.y = 0; m.y < area.mps; m.y++)
		for(m.x = 0; m.x < area.mps; m.x++)
			area.set(m, Explored);
}

static void test_arena(int bonus) {
	answers an;
	auto count = 0;
	for(auto& e : bsdata<monsteri>()) {
		if(e.is(Female))
			continue;
		an.add(&e, e.getname());
	}
	auto pm = (monsteri*)an.choose(getnm("ChooseMonsterToFight"));
	auto m = player->getposition();
	auto p = creature::create(m.to(3, 0), pm);
	p->set(Enemy);
	animate_figures();
}

void show_area(int bonus);

BSDATA(script) = {
	{"AttackForward", attack_forward},
	{"ChooseCreature", choose_creature},
	{"ChatSomeone", chat_someone},
	{"DebugMessage", debug_message},
	{"DropDown", dropdown},
	{"ExploreArea", explore_area},
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
	{"PickUp", pickup},
	{"ShowMinimap", show_area},
	{"TestArena", test_arena},
	{"ViewStuff", view_stuff},
};
BSDATAF(script)