#include "draw_object.h"
#include "ifscript.h"
#include "greatneed.h"
#include "resource.h"
#include "stringact.h"
#include "indexa.h"
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
int					last_value;
int					last_coins;
greatneed*			last_need;
ability_s			last_ability;
variant				last_variant;
quest*				last_quest;
locationi*			last_location;
const sitei*		last_site;
globali*			last_global;
rect				last_rect;
siteskilla			last_actions;
extern bool			show_floor_rect;
static bool			stop_script;
const sitegeni*		last_method;

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

static void place_creature(variant v, int count) {
	if(count <= 0) {
		if(v.iskind<monsteri>())
			count = bsdata<monsteri>::elements[v.value].appear.roll();
		else
			count = xrand(2, 5);
		if(!count)
			count = 1;
	}
	for(auto i = 0; i < count; i++) {
		auto p = creature::create(area.get(last_rect), v);
		p->set(Local);
	}
}

void runscript(variant v) {
	if(v.iskind<featurei>())
		area.set(last_rect, (featuren)v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(last_rect, (tile_s)v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(last_rect, (areaf)v.value, v.counter);
	else if(v.iskind<globali>()) {
		last_global = bsdata<globali>::elements + v.value;
		last_value = game.get(*last_global);
		game.set(*last_global, last_value + v.counter);
	} else if(v.iskind<listi>()) {
		for(auto v : bsdata<listi>::elements[v.value].elements)
			runscript(v);
	} else if(v.iskind<randomizeri>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			runscript(bsdata<randomizeri>::elements[v.value].random());
	} else if(v.iskind<script>()) {
		pushvalue push_result(last_variant, {});
		bsdata<script>::elements[v.value].run(v.counter);
		if(last_variant)
			runscript(last_variant);
	} else if(v.iskind<monsteri>()) {
		auto count = game.getcount(v, 0);
		if(count < 0)
			return;
		if(count == 0)
			count = bsdata<monsteri>::elements[v.value].appear.roll();
		if(!count)
			count = 1;
		place_creature(v, count);
	} else if(v.iskind<racei>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		place_creature(v, count);
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
		pushvalue push_site(last_site);
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
	} else if(v.iskind<needni>()) {
		if(!last_need)
			return;
		if(v.counter >= 0)
			last_need->set((needn)v.value);
		else
			last_need->remove((needn)v.value);
	} else {
		if(player)
			player->apply(v);
	}
}

static bool ifscript(variant v) {
	if(v.iskind<ifscripti>())
		stop_script = !bsdata<ifscripti>::elements[v.value].proc(v.counter);
	else if(v.iskind<needni>()) {
		if(v.counter <= 0)
			stop_script = !last_need || !last_need->is((needn)v.value);
	} else if(v.iskind<abilityi>()) {
		if(v.counter > 0)
			return false;
		else if(v.counter < 0 && player->get((ability_s)v.value) < -v.counter)
			stop_script = true;
		else
			last_value = player->get((ability_s)v.value);
	} else if(v.iskind<monsteri>()) {
		if(v.counter != 0)
			return false;
		stop_script = !player->iskind(v);
	} else
		return false;
	return true;
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
	dialog_message(getdescription("WinGame1"));
	//console.addn("Object count [%1i].", bsdata<draw::object>::source.getcount());
	//actable::pressspace();
}

static void open_nearest_door(int bonus) {
	indexa source;
	source.select(player->getposition(), 1);
	for(auto i : source) {
		auto& ei = area.getfeature(i);
		auto fa = ei.getactivate();
		if(fa)
			area.set(i, *fa);
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
	if(opponent->speechneed())
		return;
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
	p->wait();
	player->wait();
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

static void set_result(variant v, int bonus) {
	last_variant = v;
	if(last_variant)
		last_variant.counter = bonus;
}

static void quest_minion(int bonus) {
	last_variant.clear();
	if(!last_quest)
		return;
	monsteri* pm = last_quest->problem;
	if(pm)
		set_result(pm->ally(), bonus);
}

static void quest_guardian(int bonus) {
	last_variant.clear();
	if(last_quest)
		set_result(last_quest->problem, bonus);
}

static void quest_reward(int bonus) {
	last_variant.clear();
	if(last_quest && last_quest->reward)
		set_result(last_quest->reward, bonus);
}

static void quest_landscape(int bonus) {
	if(!last_quest)
		return;
	locationi* pm = last_quest->modifier;
	if(pm)
		runscript(pm->landscape);
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
	pushvalue push_width(window_width, 300);
	last_variant = allowed_spells.choose(getnm("ChooseSpell"), getnm("Cancel"), player);
}

static void cast_spell(int bonus) {
	choose_spell(bonus);
	if(last_variant.iskind<spelli>())
		player->cast(bsdata<spelli>::elements[last_variant.value]);
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
	auto pn = bonus ? str("WinGame%1i", bonus) : "WinGame";
	dialog_message(getdescription(pn));
	game.next(game.mainmenu);
}

static void lose_game(int bonus) {
	auto pn = bonus ? str("LoseGame%1i", bonus) : "LoseGame";
	dialog_message(getdescription(pn));
	game.next(game.mainmenu);
}

static void add_dungeon_rumor(int bonus) {
	quest::add(KillBossQuest, game.position);
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

void runscript(const variants& elements) {
	if(stop_script)
		return;
	pushvalue push_stop(stop_script);
	for(auto v : elements) {
		if(stop_script)
			break;
		runscript(v);
	}
}

bool ifscript(const variants& elements) {
	pushvalue push_stop(stop_script, false);
	for(auto v : elements) {
		if(!ifscript(v) || stop_script)
			break;
	}
	return !stop_script;
}

static const char* day_left(unsigned end_stamp) {
	auto stamp = game.getminutes();
	if(end_stamp <= stamp)
		return getnm("FewTime");
	auto value = (end_stamp - stamp) / (24 * 60);
	if(value > 0)
		return str("%1i %-2", value, stringbuilder::getbycount("Day", value));
	value = (end_stamp - stamp) / 60;
	if(value > 0)
		return str("%1i %-2", value, stringbuilder::getbycount("Hour", value));
	value = end_stamp - stamp;
	return str("%1i %-2", value, stringbuilder::getbycount("Minute", value));
}

static void need_help_info(stringbuilder& sb) {
	if(!last_need)
		return;
	auto pn = getdescription(last_need->geti().getid());
	if(!pn)
		return;
	sb.add(pn, day_left(last_need->deadline));
}

static void last_coins_info(stringbuilder& sb) {
	sb.add("%1i", last_coins);
}

static bool if_lesser(int bonus) {
	return last_value < bonus;
}

static bool if_greater(int bonus) {
	return last_value > bonus;
}

void add_need(int bonus);
void add_need_answers(int bonus);

BSDATA(textscript) = {
	{"LastCoins", last_coins_info},
	{"NeedHelpIntro", need_help_info},
};
BSDATAF(textscript)
BSDATA(ifscripti) = {
	{"Greater", if_greater},
	{"Lesser", if_lesser},
};
BSDATAF(ifscripti)
BSDATA(triggerni) = {
	{"WhenCreatureP1EnterSiteP2"},
	{"WhenCreatureP1Dead"},
	{"WhenCreatureP1InSiteP2UpdateAbilities"},
	{"EverySeveralDays"},
	{"EverySeveralDaysForP1"},
};
assert_enum(triggerni, EverySeveralDaysForP1)
BSDATA(script) = {
	{"AddDungeonRumor", add_dungeon_rumor},
	{"AddNeed", add_need},
	{"AddNeedAnswers", add_need_answers},
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