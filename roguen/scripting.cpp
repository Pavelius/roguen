#include "boost.h"
#include "condition.h"
#include "creature.h"
#include "draw_object.h"
#include "ifscript.h"
#include "itema.h"
#include "listcolumn.h"
#include "greatneed.h"
#include "pushvalue.h"
#include "race.h"
#include "resid.h"
#include "resource.h"
#include "script.h"
#include "siteskill.h"
#include "stringact.h"
#include "trigger.h"
#include "triggern.h"
#include "indexa.h"

roomi* add_room(const sitei* ps, const rect& rc);
void animate_figures();
bool check_activate(creature* player, point m, const featurei& ei);
bool isfreeltsv(point m);
bool isfreecr(point m);
void place_shape(const shapei& e, point m, int floor, int walls);
void show_area(int bonus);
void show_logs(int bonus);
void visualize_images(res pid, point size, point offset);

creaturea		creatures, enemies, targets;
rooma			rooms;
itema			items;
indexa			indecies;
spella			allowed_spells;
creature		*player, *opponent, *enemy;
ability_s		last_ability;
siteskilla		last_actions;
int				last_coins;
const char*		last_id;
point			last_index;
globali*		last_global;
locationi*		last_location;
quest*			last_quest;
rect			last_rect;
roomi*			last_room;
const sitei*	last_site;
greatneed*		last_need;
int				last_value, last_cap;
extern bool		show_floor_rect;
static bool		stop_script;
static itemstat *item_prefix, *item_suffix;

static const sitegeni* get_local_method() {
	if(last_site && last_site->local)
		return last_site->local;
	if(last_location && last_location->local)
		return last_location->local;
	return 0;
}

static void place_item(point index, const itemi* pe) {
	if(!pe || pe == bsdata<itemi>::elements)
		return;
	if(area.iswall(index))
		return;
	item it; it.clear();
	it.create(pe);
	if(item_prefix || item_suffix) {
		it.setupgrade(item_prefix);
		it.setupgrade(item_suffix);
		item_prefix = item_suffix = 0;
	} else {
		auto chance_prefix = (areahead.level - 1) * 15;
		auto chance_suffix = chance_prefix / 2;
		if(chance_prefix > 80)
			chance_prefix = 80;
		if(chance_suffix > 70)
			chance_suffix = 70;
		it.upgrade(chance_prefix, chance_suffix, areahead.level);
	}
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
		if(p->is(Enemy))
			areahead.total.monsters++;
	}
}

static void visualize_activity(point m) {
	if(!area.is(m, Visible))
		return;
	movable::fixeffect(m2s(m), "SearchVisual");
}

void script::run(variant v) {
	if(v.iskind<featurei>())
		area.set(last_rect, &areamap::setfeature, v.value, v.counter);
	else if(v.iskind<tilei>())
		area.set(last_rect, &areamap::settile, v.value, v.counter);
	else if(v.iskind<areafi>())
		area.set(last_rect, &areamap::setflag, v.value, v.counter);
	else if(v.iskind<globali>()) {
		last_global = bsdata<globali>::elements + v.value;
		last_value = game.get(*last_global);
		game.set(*last_global, last_value + v.counter);
	} else if(v.iskind<listi>()) {
		for(auto v : bsdata<listi>::elements[v.value].elements)
			script::run(v);
	} else if(v.iskind<randomizeri>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			script::run(bsdata<randomizeri>::elements[v.value].random());
	} else if(v.iskind<script>())
		bsdata<script>::elements[v.value].run(v.counter);
	else if(v.iskind<monsteri>()) {
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
	} else if(v.iskind<itemstat>()) {
		if(!item_prefix)
			item_prefix = v;
		if(!item_suffix)
			item_suffix = v;
	} else if(v.iskind<itemi>()) {
		auto count = game.getcount(v);
		if(count <= 0)
			return;
		for(auto i = 0; i < count; i++)
			place_item(area.get(last_rect), bsdata<itemi>::elements + v.value);
	} else if(v.iskind<sitei>()) {
		pushvalue push_rect(last_rect);
		pushvalue push_site(last_site);
		last_site = bsdata<sitei>::elements + v.value;
		auto last_method = get_local_method();
		if(last_method)
			(last_site->*last_method->proc)();
		add_room(last_site, last_rect);
		script::run(bsdata<sitei>::elements[v.value].landscape);
	} else if(v.iskind<locationi>()) {
		pushvalue push_rect(last_rect);
		script::run(bsdata<locationi>::elements[v.value].landscape);
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

static void add_creatures(feat_s v) {
	for(auto p : creatures) {
		if(p->is(v))
			targets.add(p);
	}
}

static void add_neutrals() {
	for(auto p : creatures) {
		if(p->is(Ally) || p->is(Enemy))
			continue;
		targets.add(p);
	}
}

static void match_creatures(const variants& source) {
	auto ps = targets.begin();
	for(auto p : targets) {
		if(!p->isallow(source))
			continue;
		*ps++ = p;
	}
	targets.count = ps - targets.begin();
}

static bool is(const featurei& ei, condition_s v) {
	switch(v) {
	case Locked: return ei.islocked();
	default: return false;
	}
}

static bool isallow(const featurei& ei, variant v) {
	if(v.iskind<conditioni>())
		return is(ei, (condition_s)v.value);
	return true;
}

static bool isallow(const featurei& ei, const variants source) {
	for(auto v : source) {
		if(!isallow(ei, v))
			return false;
	}
	return true;
}

static void match_features(const variants& source) {
	auto ps = indecies.begin();
	for(auto m : indecies) {
		auto& ei = area.getfeature(m);
		if(!isallow(ei, source))
			continue;
		*ps++ = m;
	}
	indecies.count = ps - indecies.begin();
}

static bool iscondition(const void* object, int v) {
	auto p = (item*)object;
	switch(v) {
	case Identified: return p->isidentified();
	case Unaware: return !p->isidentified() && !p->iscountable();
	default: return false;
	}
}

static bool isallow(const item& e, variant v) {
	if(v.iskind<conditioni>())
		return iscondition(&e, v.value);
	return true;
}

static bool isallow(const item& e, const variants& source) {
	for(auto v : source) {
		if(!isallow(e, v))
			return false;
	}
	return true;
}

static void match_items(const variants& source) {
	auto ps = items.begin();
	for(auto p : items) {
		if(!isallow(*p, source))
			continue;
		*ps++ = p;
	}
	items.count = ps - items.begin();
}

bool choose_targets(unsigned flags, const variants& effects) {
	indecies.clear();
	items.clear();
	rooms.clear();
	targets.clear();
	if(FGT(flags, TargetCreatures)) {
		if(FGT(flags, Allies)) {
			if(player->is(Ally))
				add_creatures(Ally);
			if(player->is(Enemy))
				add_creatures(Enemy);
		}
		if(FGT(flags, Enemies)) {
			if(player->is(Ally))
				add_creatures(Enemy);
			if(player->is(Enemy))
				add_creatures(Ally);
		}
		if(FGT(flags, Neutrals))
			add_neutrals();
		if(FGT(flags, You))
			targets.add(player);
		if(!FGT(flags, Ranged))
			targets.matchrange(player->getposition(), 1, true);
		targets.distinct();
		match_creatures(effects);
		targets.sort(player->getposition());
	}
	if(FGT(flags, TargetFeatures)) {
		indecies.clear();
		indecies.select(player->getposition(), FGT(flags, Ranged) ? 3 : 1);
		indecies.match(fntis<featurei, &featurei::isvisible>, true);
		match_features(effects);
		indecies.sort(player->getposition());
	}
	if(FGT(flags, TargetRooms)) {
		rooms.select(fntis<roomi, &roomi::islocal>);
		if(!FGT(flags, You))
			rooms.remove(player->getroom());
		if(!FGT(flags, Ranged))
			rooms.match(fntis<roomi, &roomi::ismarkable>, true);
	}
	if(FGT(flags, TargetItems)) {
		items.select(player);
		if(FGT(flags, Unaware))
			items.match(iscondition, Unaware, true);
		if(FGT(flags, Identified))
			items.match(iscondition, Identified, true);
	}
	if(FGT(flags, Random)) {
		targets.shuffle();
		items.shuffle();
		rooms.shuffle();
		indecies.shuffle();
	}
	return targets.getcount() != 0
		|| rooms.getcount() != 0
		|| items.getcount() != 0
		|| indecies.getcount() != 0;
}

static bool choose_target_interactive(const char* id, unsigned flags, bool autochooseone) {
	if(!id)
		return true;
	auto pn = getdescription(str("%1Choose", id));
	if(!pn)
		return true;
	pushvalue push_width(window_width, 300);
	if(FGT(flags, TargetCreatures)) {
		if(!targets.chooseu(pn, getnm("Cancel")))
			return false;
	}
	if(FGT(flags, TargetRooms)) {
		if(!rooms.chooseu(pn, getnm("Cancel")))
			return false;
	}
	if(FGT(flags, TargetItems)) {
		if(!items.chooseu(pn, getnm("Cancel")))
			return false;
	}
	return true;
}

static void action_text(const creature* player, const char* id, const char* action) {
	if(!player->is(AnimalInt)) {
		auto pn = player->getspeech(str("%1%2Speech", id, action), false);
		if(pn) {
			player->say(pn);
			return;
		}
	}
	auto pn = getdescription(str("%1%2", id, action));
	if(pn)
		player->act(pn);
}

static bool bound_targets(const char* id, unsigned flags, int multi_targets, bool interactive) {
	pushvalue push_interactive(answers::interactive, interactive);
	unsigned target_count = 1 + multi_targets;
	if(!multi_targets) {
		if(!choose_target_interactive(id, flags, false))
			return false;
	}
	if(targets.count > target_count)
		targets.count = target_count;
	if(indecies.count > target_count)
		indecies.count = target_count;
	if(rooms.count > target_count)
		rooms.count = target_count;
	return true;
}

void apply_spell(creature* player, const spelli& ei, int level) {
	pushvalue push_value(last_value, ei.getcount(level));
	if(ei.duration) {
		auto minutes = bsdata<durationi>::elements[ei.duration].get(level);
		auto stop_time = game.getminutes() + minutes;
		player->fixvalue(str("%1 %2i %-Minutes", ei.getname(), minutes), 2);
		for(auto v : ei.effect)
			boosti::add(player, v, stop_time);
	} else
		player->apply(ei.effect);
}

void creature::cast(const spelli& e, int level, int mana) {
	if(get(Mana) < mana) {
		actp(getnm("NotEnoughtMana"));
		return;
	}
	if(!choose_targets(e.target, e.effect) && !e.summon) {
		actp(getnm("YouDontValidTargets"));
		return;
	}
	if(!bound_targets(e.id, e.target, e.is(Multitarget) ? level : 0, ishuman()))
		return;
	action_text(this, e.id, "Casting");
	if(FGT(e.target, TargetCreatures)) {
		for(auto p : targets)
			apply_spell(p, e, level);
	} else if(FGT(e.target, TargetFeatures)) {
		for(auto p : indecies) {
			pushvalue push_rect(last_rect, {p.x, p.y, p.x, p.y});
			script::run(e.effect);
		}
	} else if(FGT(e.target, TargetRooms)) {
		for(auto p : rooms) {
			pushvalue push_rect(last_room, p);
			script::run(e.effect);
		}
	} else if(FGT(e.target, TargetItems)) {
		for(auto p : items) {
			pushvalue push_object(last_item, p);
			script::run(e.effect);
		}
	}
	if(e.summon)
		summon(player->getposition(), e.summon, e.getcount(level), level);
	add(Mana, -mana);
	update();
	wait();
}

static const char* item_weight(const void* object, stringbuilder& sb) {
	auto p = (item*)object;
	if(!(*p))
		return "";
	auto w = p->getweight();
	sb.add("%1i.%2i%3i %Kg", w / 100, (w/10) % 10, w % 10);
	return sb.begin();
}

static item* choose_wear() {
	static wear_s source[] = {
		Head, Backward, Torso, MeleeWeapon, MeleeWeaponOffhand, RangedWeapon,
		Elbows, FingerRight, FingerLeft, Gloves, Legs, Ammunition,
	};
	answers an;
	for(auto& e : player->equipment()) {
		if(!e)
			an.add(&e, "-");
		else
			an.add(&e, e.getfullname());
	}
	static listcolumn columns[] = {
		{"Weight", 60, item_weight, true},
		{}};
	pushvalue push_columns(current_columns, columns);
	return (item*)an.choose(getnm("Inventory"), getnm("Cancel"));
}

static item* choose_stuff(wear_s wear) {
	answers an;
	char temp[512]; stringbuilder sb(temp);
	for(auto& e : player->backpack()) {
		if(!e)
			continue;
		if(wear && !e.is(wear))
			continue;
		sb.clear();
		e.getinfo(sb);
		an.add(&e, temp);
	}
	sb.clear();
	sb.add("%Choose %-1", bsdata<weari>::elements[wear].getname());
	return (item*)an.choose(temp, getnm("Cancel"));
}

static creature* getowner(const item* pi) {
	auto i = bsdata<creature>::source.indexof(pi);
	if(i == -1)
		return 0;
	return bsdata<creature>::elements + i;
}

static void inventory(int bonus) {
	while(true) {
		auto pi = choose_wear();
		if(!pi)
			break;
		auto owner = getowner(pi);
		if(!owner)
			break;
		if((*pi)) {
			player->additem(*pi);
			player->update();
		} else {
			auto ni = choose_stuff(owner->getwearslot(pi));
			if(ni)
				iswap(*ni, *pi);
			player->update();
		}
	}
}

static void debug_message(int bonus) {
	//dialog_message(getdescription("LoseGame1"));
	console.addn("Object count [%1i].", bsdata<draw::object>::source.getcount());
	draw::pause();
}

static void open_nearest_door(int bonus) {
	indexa source;
	source.select(player->getposition(), 1);
	for(auto i : source) {
		auto& ei = area.getfeature(i);
		if(!ei.isvisible())
			continue;
		check_activate(player, i, ei);
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
		if(player->talk(room->geti().id))
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
	unsigned target = FG(TargetCreatures) | FG(Allies) | FG(Neutrals);
	choose_targets(target, {});
	if(!targets) {
		player->actp(getnm("NoCreaturesNearby"));
		return;
	}
	if(!choose_target_interactive("Creature", target, true))
		return;
	for(auto p : targets) {
		pushvalue push(opponent, p);
		chat_someone();
		player->wait();
		opponent->wait();
	}
}

static void test_rumor(int bonus) {
	player->speechrumor();
}

static bool payment(creature* player, creature* keeper, const char* object, int coins) {
	if(player->ishuman()) {
		if(answers::console)
			answers::console->clear();
		player->actp(getnm("WantBuyItem"), object, coins);
		if(!draw::yesno(0))
			return false;
	}
	auto allow_coins = player->getmoney();
	if(player->getmoney() < coins) {
		keeper->speech("NotEnoughCoins", allow_coins, coins, coins - allow_coins);
		return false;
	}
	keeper->addcoins(coins);
	player->addcoins(-coins);
	return true;
}

static bool selling(creature* player, creature* opponent, const char* object, int coins) {
	if(player->ishuman()) {
		if(answers::console)
			answers::console->clear();
		player->actp(getnm("WantSellItem"), object, coins);
		if(!draw::yesno(0))
			return false;
	}
	auto allow_coins = opponent->getmoney();
	if(opponent->getmoney() < coins) {
		opponent->speech("KeeperNotEnoughCoins", allow_coins, coins, coins - allow_coins);
		return false;
	}
	player->addcoins(coins);
	opponent->addcoins(-coins);
	return true;
}

static void pickup(int bonus) {
	itema items;
	items.select(player->getposition());
	if(!items)
		return;
	auto p = items.choose(getnm("PickItem"), getnm("Cancel"));
	if(p) {
		auto payment_cost = player->getpaymentcost();
		if(payment_cost) {
			auto item_cost = p->getcostall() * payment_cost / 100;
			auto keeper = player->getroom()->getowner();
			if(!payment(player, keeper, p->getfullname(), item_cost))
				return;
		}
		player->act(getnm("PickupItem"), p->getfullname());
		player->additem(*p);
	}
}

static void pickup_all(int bonus) {
	if(player->getpaymentcost()) {
		player->actp("YouCantPickUpAllForCost");
		return;
	}
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
		auto payment_cost = player->getsellingcost();
		if(payment_cost) {
			auto item_cost = p->getcostall() * payment_cost / 100;
			auto keeper = player->getroom()->getowner();
			if(!selling(player, keeper, p->getfullname(), item_cost))
				return;
		}
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
	choose_stuff(Backpack);
}

static void explore_area(int bonus) {
	area.set({0, 0, area.mps, area.mps}, &areamap::setflag, Explored);
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

static void quest_minion(int bonus) {
	if(!last_quest)
		return;
	monsteri* pm = last_quest->problem;
	if(pm)
		script::runv(pm->ally(), bonus);
}

static void quest_guardian(int bonus) {
	if(!last_quest)
		return;
	monsteri* pm = last_quest->problem;
	if(pm) {
		areahead.total.boss++;
		script::runv((monsteri*)last_quest->problem, bonus);
	}
}

static void quest_reward(int bonus) {
	if(last_quest && last_quest->reward) {
		variant v = last_quest->reward;
		v.counter = bonus;
		script::run(v);
	}
}

static void quest_landscape(int bonus) {
	if(!last_quest)
		return;
	locationi* pm = last_quest->modifier;
	if(pm)
		script::run(pm->landscape);
}

static void site_floor(int bonus) {
	if(last_site && last_site->floors)
		script::runv(bsdata<tilei>::elements + last_site->floors, bonus);
	else if(last_location && last_location->floors)
		script::runv(bsdata<tilei>::elements + last_location->floors, bonus);
}

static void site_wall(int bonus) {
	if(last_site && last_site->walls)
		script::runv(bsdata<tilei>::elements + last_site->walls, bonus);
	else if(last_location && last_location->walls)
		script::runv(bsdata<tilei>::elements + last_location->walls, bonus);
}

static const spelli* choose_spell(int bonus) {
	pushvalue push_width(window_width, 300);
	return allowed_spells.choose(getnm("ChooseSpell"), getnm("Cancel"), player);
}

static void cast_spell(int bonus) {
	auto p = choose_spell(bonus);
	if(p)
		player->cast(*p);
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
		script::run(pa);
	}
	player->wait();
}

static void jump_to_site(int bonus) {
	if(!last_room)
		return;
	if(!player->ishuman())
		player->act(getnm("YouSundellyDisappear"));
	auto m = area.getfree(center(last_room->rc), 8, isfreecr);
	player->place(m);
	if(!player->ishuman())
		player->act(getnm("YouSundellyAppear"));
	else
		area.setlos(m, player->getlos(), isfreeltsv);
	player->fixteleport(player->ishuman());
}

static void wait_hour(int bonus) {
	player->wait(6 * 60 * 24);
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

static void random_chance(int bonus) {
	if(d100() >= bonus)
		stop_script = true;
}

static void activate_feature(int bonus) {
	point m = center(last_rect);
	visualize_activity(m);
	area.setactivate(m);
}

static void destroy_feature(int bonus) {
	point m = center(last_rect);
	visualize_activity(m);
	area.setfeature(m, 0);
}

static void identify_item(int bonus) {
	last_item->setidentified(bonus);
}

void script::run(const variants& elements) {
	if(stop_script)
		return;
	pushvalue push_stop(stop_script);
	pushvalue push_cap(last_cap, 0);
	for(auto v : elements) {
		if(stop_script)
			break;
		script::run(v);
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

static void need_help_info(stringbuilder& sb) {
	if(!last_need)
		return;
	auto pn = getdescription(last_need->geti().getid());
	if(!pn)
		return;
	sb.add(pn, game.timeleft(last_need->deadline));
}

static const char* visualize_progress(int score) {
	if(score == 0)
		return "NoAnyProgress";
	else if(score < 40)
		return "LittleProgress";
	else if(score < 70)
		return "HalfwayProgress";
	else
		return "AlmostFinishProgress";
}

static void actual_need_state(stringbuilder& sb) {
	if(!last_need)
		return;
	sb.add(getnm("VisualizeProgress"), getnm(visualize_progress(last_need->score)), game.timeleft(last_need->deadline));
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
	{"ActualNeedState", actual_need_state},
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
	{"Activate", activate_feature},
	{"CastSpell", cast_spell},
	{"ChooseAction", choose_action},
	{"Chance", random_chance},
	{"ChatSomeone", chat_someone},
	{"DebugMessage", debug_message},
	{"DestroyFeature", destroy_feature},
	{"DropDown", dropdown},
	{"ExploreArea", explore_area},
	{"JumpToSite", jump_to_site},
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
	{"IdentifyItem", identify_item},
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
	{"WaitHour", wait_hour},
	{"UseItem", use_item},
};
BSDATAF(script)