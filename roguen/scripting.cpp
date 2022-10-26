#include "draw_object.h"
#include "resource.h"
#include "main.h"

creaturea			creatures, enemies, targets;
itema				items;
extern creature*	enemy;
int					last_hit, last_hit_result, last_parry, last_parry_result;
extern bool			show_floor_rect;

void animate_figures();
void visualize_images(res pid, point size, point offset);

void script::run(const char* id, int bonus) {
	auto p = bsdata<script>::find(id);
	if(!p)
		return;
	p->proc(0);
}

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
	player->fixaction();
	player->fixeffect("HitVisual");
	player->wait();
}

static item* choose_wear() {
	static wear_s source[] = {
		Head, Backward, Torso, MeleeWeapon, MeleeWeaponOffhand, RangedWeapon,
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
	for(auto i = Backpack; i <= BackpackLast; i = (wear_s)(i + 1)) {
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

static void debug_message(int bonus) {
	console.addn("Object count [%1i].", bsdata<draw::object>::source.getcount());
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
	player->lookenemies();
	creaturea source = creatures;
	source.matchrange(player->getposition(), 1, true);
	source.remove(player);
	if(!source) {
		player->actp(getnm("NoCreaturesNearby"));
		return;
	}
	source[0]->speech("HowYouAre");
}

static void pickup(int bonus) {
	itema items;
	items.select(player->getposition());
	if(!items)
		return;
	auto p = items.choose(getnm("PickItem"), getnm("Cancel"));
	if(p) {
		player->act(getnm("PickupItem"), p->getfullname());
		player->additem(*p);
	}
}

static void dropdown(int bonus) {
	itema items;
	items.selectbackpack(player);
	if(!items)
		return;
	auto p = items.choose(getnm("DropItem"), getnm("Cancel"));
	if(p) {
		player->act(getnm("DropdownItem"), p->getfullname());
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
	player->lookenemies();
	animate_figures();
}

static void toggle_floor_rect(int bonus) {
	show_floor_rect = !show_floor_rect;
}

static void range_attack(int bonud) {
	if(!player->canshoot(true))
		return;
	if(!enemy) {
		player->actp(getnm("YouDontSeeAnyEnemy"));
		return;
	}
	if(enemy) {
		player->setdirection(area.getdirection(player->getposition(), enemy->getposition()));
		player->attackrange(*enemy);
		player->wait();
	}
}

static void thrown_attack(int bonud) {
	if(!player->canthrown(true))
		return;
	if(!enemy) {
		player->actp(getnm("YouDontSeeAnyEnemy"));
		return;
	}
	if(enemy) {
		player->setdirection(area.getdirection(player->getposition(), enemy->getposition()));
		player->attackthrown(*enemy);
		player->wait();
	}
}

static void show_images(int bonus) {
	static res source[] = {res::Monsters, res::Items};
	answers an;
	for(auto id : source)
		an.add((void*)id, bsdata<resource>::elements[(int)id].name);
	console.clear();
	player->act(getnm("ChooseImageSet"));
	auto id = (res)(int)an.choose();
	switch(id) {
	case res::Items:
		visualize_images(id, {64, 64}, {64 / 2, 64 / 2});
		break;
	default:
		visualize_images(id, {80, 90}, {80 / 2, 90});
		break;
	}
}

void show_area(int bonus);
void show_logs(int bonus);

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
	{"RangeAttack", range_attack},
	{"ShowImages", show_images},
	{"ShowLogs", show_logs},
	{"ShowMinimap", show_area},
	{"TestArena", test_arena},
	{"ThrownAttack", thrown_attack},
	{"ToggleFloorRect", toggle_floor_rect},
	{"ViewStuff", view_stuff},
};
BSDATAF(script)