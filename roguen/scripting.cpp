#include "draw_object.h"
#include "resource.h"
#include "main.h"

roomi* add_room(const sitei* ps, const rect& rc);
void animate_figures();
void choose_targets(unsigned flags);
void place_shape(const shapei& e, point m, tile_s floor, tile_s walls);
void show_area(int bonus);
void show_logs(int bonus);
void visualize_images(res pid, point size, point offset);

creaturea			creatures, enemies, targets;
spella				allowed_spells;
itema				items;
creature			*player, *opponent, *enemy;
int					last_hit, last_hit_result, last_parry, last_parry_result, last_value;
ability_s			last_ability;
variant				last_variant;
dungeon*			last_dungeon;
locationi*			last_location;
const sitei*		last_site;
globali*			last_global;
rect				last_rect;
siteskilla			last_actions;
extern bool			show_floor_rect;
const sitegeni*		last_method;

tile_s getfloor() {
	if(last_site && last_site->floors)
		return last_site->floors;
	if(last_location && last_location->floors)
		return last_location->floors;
	return DungeonFloor;
}

tile_s getwall() {
	if(last_site && last_site->walls)
		return last_site->walls;
	if(last_location && last_location->walls)
		return last_location->walls;
	return WallDungeon;
}

static void place_item(point index, const itemi* pe) {
	if(!pe || pe == bsdata<itemi>::elements)
		return;
	if(area.iswall(index))
		return;
	item it; it.clear();
	it.create(pe);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	it.drop(index);
}

static void place_item(const itemi* pe) {
	if(!player || !pe || pe == bsdata<itemi>::elements)
		return;
	item it; it.clear();
	it.create(pe);
	if(pe->is(Coins))
		it.setcount(xrand(3, 18));
	player->additem(it);
}

static void create_creature(variant v, int count) {
	if(count <= 0) {
		if(v.iskind<monsteri>())
			count = bsdata<monsteri>::elements[v.value].appear.roll();
		else
			count = xrand(2, 5);
		if(!count)
			count = 1;
	}
	for(auto i = 0; i < count; i++)
		creature::create(area.get(last_rect), v);
}

static void standart_script(variant v) {
	if(v.iskind<featurei>())
		area.set(last_rect, (feature_s)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(last_rect, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(last_rect, (mapf_s)v.value, v.counter);
	else if(v.iskind<globali>()) {
		last_global = bsdata<globali>::elements + v.value;
		last_value = game.get(*last_global);
		game.set(*last_global, last_value + v.counter);
	} else if(v.iskind<listi>()) {
		for(auto v : bsdata<listi>::elements[v.value].elements)
			last_scipt_proc(v);
	} else if(v.iskind<randomizeri>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			last_scipt_proc(bsdata<randomizeri>::elements[v.value].random());
	} else if(v.iskind<script>()) {
		pushvalue push_result(last_variant, {});
		bsdata<script>::elements[v.value].run(v.counter);
		if(last_variant)
			last_scipt_proc(last_variant);
	} else if(v.iskind<monsteri>() || v.iskind<racei>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		create_creature(v, count);
	} else if(v.iskind<classi>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			creature::create(area.get(last_rect), single("RandomRace"), v, (rand() % 2) != 0);
	} else if(v.iskind<shapei>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++) {
			place_shape(bsdata<shapei>::elements[v.value],
				area.get(last_rect), getfloor(), getwall());
		}
	} else if(v.iskind<itemi>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			place_item(area.get(last_rect), bsdata<itemi>::elements + v.value);
	} else if(v.iskind<sitei>()) {
		pushvalue push_rect(last_rect);
		pushvalue push_method(last_method);
		last_site = bsdata<sitei>::elements + v.value;
		if(last_site->local)
			last_method = last_site->local;
		if(last_method)
			(last_site->*last_method->proc)();
		add_room(last_site, last_rect);
		runscript(bsdata<sitei>::elements[v.value].landscape);
	} else if(v.iskind<locationi>()) {
		pushvalue push_rect(last_rect);
		runscript(bsdata<locationi>::elements[v.value].landscape);
	} else if(v.iskind<speech>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		if(player)
			player->speech(bsdata<speech>::elements[v.value].id);
	} else {
		if(player)
			player->apply(v);
	}
}

static void choose_creature(int bonus) {
}

static void choose_opponent(unsigned flags) {
	opponent = 0;
	choose_targets(flags);
	if(!targets)
		return;
	else if(targets.getcount() == 1) {
		opponent = targets[0];
		return;
	}
	opponent = targets[0];
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

static void chat_someone() {
	auto monster = opponent->getmonster();
	if(monster) {
		if(player->talk(monster->id))
			return;
	}
	auto room = opponent->getroom();
	if(room) {
		if(player->talk(room->getsite()->id))
			return;
	}
	if(opponent->is(KnowRumor) && d100() < 70) {
		if(opponent->speechrumor())
			return;
	}
	if(opponent->is(KnowLocation) && d100() < 30) {
		if(opponent->speechlocation())
			return;
	}
	opponent->speech("HowYouAre");
}

static void chat_someone(int bonus) {
	choose_opponent(0);
	if(!opponent) {
		player->actp(getnm("NoCreaturesNearby"));
		return;
	}
	chat_someone();
	player->wait();
	opponent->wait();
}

static void test_rumor(int bonus) {
	player->speechrumor();
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

static void pickup_all(int bonus) {
	itema items;
	items.select(player->getposition());
	for(auto p : items) {
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

static void use_item(int bonus) {
	itema items;
	items.selectbackpack(player);
	if(!items)
		return;
	auto p = items.choose(getnm("UseItem"), getnm("Cancel"), false);
	if(p)
		player->use(*p);
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
		if(e.friendly <= -5)
			an.add(&e, e.getname());
	}
	pushvalue push_column(answers::column_count);
	answers::column_count = 3;
	auto pm = (monsteri*)an.choose(getnm("ChooseMonsterToFight"), getnm("Cancel"));
	if(!pm)
		return;
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
	player->lookenemies();
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

static void set_result(variant v, int bonus) {
	last_variant = v;
	if(last_variant)
		last_variant.counter = bonus;
}

static void quest_minion(int bonus) {
	last_variant.clear();
	if(last_dungeon && last_dungeon->guardian)
		set_result(last_dungeon->guardian->ally(), bonus);
}

static void quest_guardian(int bonus) {
	last_variant.clear();
	if(last_dungeon && last_dungeon->guardian)
		set_result(last_dungeon->guardian, bonus);
}

static void quest_reward(int bonus) {
	last_variant.clear();
	if(last_dungeon && last_dungeon->reward)
		set_result(last_dungeon->reward, bonus);
}

static void quest_landscape(int bonus) {
	if(!last_dungeon || !last_dungeon->modifier)
		return;
	runscript(last_dungeon->modifier->landscape);
}

static void site_floor(int bonus) {
	last_variant.clear();
	if(last_site && last_site->floors)
		set_result(bsdata<tilei>::elements + last_site->floors, bonus);
	else if(last_location && last_location->floors)
		set_result(bsdata<tilei>::elements + last_location->floors, bonus);
}

static void site_wall(int bonus) {
	last_variant.clear();
	if(last_site && last_site->walls)
		set_result(bsdata<tilei>::elements + last_site->walls, bonus);
	else if(last_location && last_location->walls)
		set_result(bsdata<tilei>::elements + last_location->walls, bonus);
}

static void choose_spell(int bonus) {
	last_variant = allowed_spells.choose(getnm("ChooseSpell"), getnm("Cancel"));
}

static void cast_spell(int bonus) {
	choose_spell(bonus);
	if(last_variant)
		player->cast((spell_s)last_variant.value);
}

static void heal_player(int bonus) {
	player->heal(bonus);
	player->wait();
}

static void heal_all(int bonus) {
	heal_player(100);
}

static void set_offset(int bonus) {
	if(bonus > last_rect.width() / 2)
		bonus = last_rect.width() / 2;
	if(bonus > last_rect.height() / 2)
		bonus = last_rect.height() / 2;
	last_rect.offset(bonus);
}

static void trigger_text(int bonus) {
	if(!last_site)
		return;
}

static void win_game(int bonus) {
}

static void lose_game(int bonus) {
}

static void choose_action(int bonus) {
	if(!last_actions)
		player->actp(getnm("YouDontHaveAnyActions"));
	else {
		auto pa = last_actions.choose(getnm("ChooseAction"), getnm("Cancel"), false);
		if(!pa)
			return;
		runscript(pa);
	}
	player->wait();
}

static void roll_value(int bonus) {
	if(!player)
		return;
	if(!player->roll(last_ability, bonus)) {
		player->logs(getnm("YouMakeRoll"), last_value, player->get(last_ability) + bonus, bonus);
		stop_script = true;
	} else
		player->logs(getnm("YouFailRoll"), last_value, player->get(last_ability) + bonus, bonus);
}

void initialize_script() {
	last_scipt_proc = standart_script;
}

BSDATA(triggeri) = {
	{"WhenCreatureP1EnterSiteP2"},
	{"WhenCreatureP1Dead"},
	{"WhenCreatureP1InSiteP2UpdateAbilities"}
};
assert_enum(triggeri, WhenCreatureP1InSiteP2UpdateAbilities)
BSDATA(script) = {
	{"AttackForward", attack_forward},
	{"CastSpell", cast_spell},
	{"ChooseAction", choose_action},
	{"ChooseCreature", choose_creature},
	{"ChooseSpell", choose_spell},
	{"ChatSomeone", chat_someone},
	{"DebugMessage", debug_message},
	{"DropDown", dropdown},
	{"ExploreArea", explore_area},
	{"Heal", heal_player},
	{"HealAll", heal_all},
	{"MoveDown", move_down},
	{"MoveDownLeft", move_down_left},
	{"MoveDownRight", move_down_right},
	{"MoveLeft", move_left},
	{"MoveRight", move_right},
	{"MoveUp", move_up},
	{"MoveUpRight", move_up_right},
	{"MoveUpLeft", move_up_left},
	{"Inventory", inventory},
	{"LoseGame", lose_game},
	{"Offset", set_offset},
	{"OpenNearestDoor", open_nearest_door},
	{"PickUp", pickup},
	{"PickUpAll", pickup_all},
	{"QuestGuardian", quest_guardian},
	{"QuestLandscape", quest_landscape},
	{"QuestMinion", quest_minion},
	{"QuestReward", quest_reward},
	{"Roll", roll_value},
	{"RangeAttack", range_attack},
	{"ShowImages", show_images},
	{"ShowLogs", show_logs},
	{"ShowMinimap", show_area},
	{"SiteFloor", site_floor},
	{"SiteWall", site_wall},
	{"TestArena", test_arena},
	{"TestRumor", test_rumor},
	{"ThrownAttack", thrown_attack},
	{"ToggleFloorRect", toggle_floor_rect},
	{"TriggerText", trigger_text},
	{"ViewStuff", view_stuff},
	{"WinGame", win_game},
	{"UseItem", use_item},
};
BSDATAF(script)