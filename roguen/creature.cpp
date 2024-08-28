#include "areapiece.h"
#include "answers.h"
#include "boost.h"
#include "charname.h"
#include "creature.h"
#include "direction.h"
#include "functor.h"
#include "math.h"
#include "indexa.h"
#include "itema.h"
#include "modifier.h"
#include "game.h"
#include "pushvalue.h"
#include "rand.h"
#include "script.h"
#include "siteskill.h"
#include "speech.h"
#include "textscript.h"
#include "trigger.h"
#include "triggern.h"

extern collection<roomi> rooms;
int last_roll_result;
bool last_roll_successed;

bool allow_targets(const variants& conditions);
bool apply_targets(const variants& conditions);
void damage_item(item& it);
void update_console_time();
int getfloor();

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static int get_counter(int counter) {
	if(counter > 100)
		return xrand(1, counter - 100);
	return counter;
}

static int get_experience_reward(creature* p) {
	static int levels[] = {15, 35, 65, 120, 175, 270, 420, 650, 975, 1400, 2000, 3000};
	auto r = p->get(Level);
	return maptbl(levels, r);
}

static void add(creature* player, ability_s id, int value, int minimal = 0) {
	auto i = player->get(id) + value;
	if(i < minimal)
		i = minimal;
	player->set(id, i);
}

void creature::logs(const char* format, ...) const {
	pushvalue push(player, const_cast<creature*>(this));
	logv(format, xva_start(format));
}

bool creature::speak(const char* action, const char* id, ...) const {
	if(!ishuman() && !is(Visible))
		return true;
	if(!canspeak())
		return true;
	auto pm = getmonster();
	const char* format = 0;
	if(pm) {
		speech_get(format, id, action, pm->getid());
		if(pm->parent)
			speech_get(format, id, action, pm->parent->getid());
	} else
		speech_get(format, id, action, getkind().getid());
	speech_get(format, id, action, 0);
	if(!format)
		return false;
	pushvalue push_player(player, const_cast<creature*>(this));
	update_console_time();
	sayv(console, format, xva_start(action));
	return true;
}

static creature* findalive(point m) {
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid() && e.getposition() == m)
			return &e;
	}
	return 0;
}

static void last_item_fix_action(const char* action) {
	auto& ei = last_item->geti();
	auto name = last_item->getname();
	if(player->act(action, ei.id, name))
		return;
	if(ei.unidentified) {
		if(player->act(action, ei.unidentified, name))
			return;
	}
	if(player->act(action, bsdata<weari>::elements[ei.wear].id, name))
		return;
	player->act(action, "You", name);
}

static void pay_movement() {
	auto cost = 200 - player->get(Dexterity);
	if(!player->is(Fly)) {
		auto& ei = area->getfeature(player->getposition());
		if(ei.movedifficult)
			cost = cost * ei.movedifficult / 100;
	}
	if(player->is(FastMove))
		cost /= 2;
	else if(player->is(SlowMove))
		cost *= 2;
	player->waitseconds(cost);
}

static void pay_attack(const item& weapon) {
	auto cost = 100;
	if(player->is(FastAttack, weapon))
		cost /= 2;
	else if(player->is(SlowAttack, weapon))
		cost *= 2;
	player->waitseconds(cost);
}

void pay_action() {
	auto percent = 100;
	if(player->is(FastAction))
		percent -= 50;
	else if(player->is(SlowAction))
		percent += 100;
	player->waitseconds(100 * percent / 100);
}

static ability_s damage_ability(ability_s v) {
	switch(v) {
	case BalisticSkill: return DamageRanged;
	default: return DamageMelee;
	}
}

bool creature::resist(feat_s resist, feat_s immunity) const {
	if(!resist)
		return false;
	if(is(immunity))
		return true;
	if(is(resist)) {
		if(d100() < 50)
			return true;
	}
	return false;
}

static void poison_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->resist(PoisonResistance, PoisonImmunity))
		return;
	if(player->roll(Strenght, -value))
		return;
	auto v = player->get(Poison) + value;
	player->fixeffect("PoisonVisual");
	if(v >= player->basic.abilities[Hits])
		player->kill();
	else
		player->set(Poison, v);
}

static void poison_attack(const item& weapon) {
	auto strenght = 0;
	if(player->is(WeakPoison, weapon))
		strenght += xrand(1, 2);
	if(player->is(StrongPoison, weapon))
		strenght += xrand(2, 4);
	if(player->is(DeathPoison, weapon))
		strenght += xrand(3, 6);
	poison_attack(opponent, strenght);
}

static void illness_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->resist(DiseaseResist, DiseaseImmunity))
		return;
	auto v = player->get(Illness) + value;
	player->fixeffect("PoisonVisual");
	if(v >= player->get(Strenght))
		player->kill();
	else
		player->set(Illness, v);
}

void damage_backpack_item(wear_s type, int chance, int count) {
	if(d100() >= chance)
		return;
	itema source;
	source.selectbackpack(player);
	source.matchf(Potion, true);
	source.top(count);
	auto pi = source.random();
	if(pi)
		pi->damage();
}

void damage_equipment(int bonus, bool allow_save) {
	static wear_s equipment_order[] = {Torso, MeleeWeapon, MeleeWeaponOffhand, Head, Backward, Elbows, Neck, Girdle, Gloves, Legs, FingerRight, FingerLeft};
	for(auto w : equipment_order) {
		auto& e = player->wears[w];
		if(bonus <= 0)
			break;
		if(!e)
			continue;
		if(d100() < 30)
			continue;
		bonus--;
		if(allow_save && player->resist(AcidResistance, AcidImmunity))
			continue;
		e.damage();
	}
}

static void summon_minions(point m, variant v) {
	auto push_player = player;
	auto isenemy = player->is(Enemy);
	auto isally = player->is(Ally);
	auto count = script_count(v.counter);
	auto leader = player;
	for(auto i = 0; i < count; i++) {
		auto p = player_create(m, v, false);
		if(isenemy)
			p->set(Enemy);
		else
			p->remove(Enemy);
		if(isally)
			p->set(Ally);
		else
			p->remove(Ally);
		p->set(Summoned);
		p->setowner(leader);
	}
	player = push_player;
}

void cast_spell(const spelli& e, int mana, bool silent) {
	if(player->get(Mana) < mana) {
		player->actp("NotEnoughtMana");
		return;
	}
	if(!e.summon && !allow_targets(e.use)) {
		player->actp("YouDontValidTargets");
		return;
	}
	if(!silent) {
		if(!player->speak("Casting", e.id))
			player->act("Casting", e.id);
	}
	if(e.use)
		apply_targets(e.use);
	if(e.summon)
		summon_minions(player->getposition(), e.summon);
	player->add(Mana, -mana);
	player->update();
}

void cast_spell(const spelli& e) {
	cast_spell(e, e.getmana(), false);
	pay_action();
}

static void special_spell_attack(item& weapon, creature* enemy, const spelli& ei) {
	if(weapon.isheavydamaged())
		return;
	cast_spell(ei, 0, true);
	weapon.damage();
}

static void attack_effect_stun(creature* enemy) {
	if(!enemy->resist(StunResistance, StunImmunity)) {
		enemy->set(Stun);
		enemy->fixeffect("SearchVisual");
	}
}

static void special_attack(item& weapon, creature* opponent, int& pierce, int& damage) {
	if(player->is(Vorpal, weapon)) {
		if(!opponent->resist(DeathResistance, DeathImmunity)) {
			damage = 100;
			pierce = 100;
		}
	}
	if(player->is(BleedingHit, weapon))
		opponent->set(Blooding);
	if(player->is(StunningHit, weapon))
		attack_effect_stun(opponent);
	if(player->is(PierceHit, weapon))
		pierce += 4;
	if(player->is(MightyHit, weapon))
		damage += 2;
	if(player->is(ColdDamage, weapon)) {
		opponent->add(Freezing, 2);
		area->setflag(opponent->getposition(), Iced);
	}
	if(player->is(FireDamage, weapon))
		opponent->add(Burning, 2);
	if(player->is(AcidDamage, weapon))
		opponent->add(Corrosion, 2);
	if(player->is(IllnessDamage, weapon))
		illness_attack(opponent, 1);
	auto power = weapon.getpower();
	if(power.iskind<spelli>())
		special_spell_attack(weapon, opponent, bsdata<spelli>::elements[power.value]);
	// Damage equipment sometime
	if(d100() < 30) {
		pushvalue push_player(player, opponent);
		damage_equipment(1, false);
	}
}

static void restore(ability_s a, ability_s test) {
	if(player->is(Illness))
		return;
	auto v = player->get(a);
	auto mv = player->basic.abilities[a];
	if(v < mv) {
		if(player->roll(test))
			player->add(a, 1);
	}
}

static void magic_restore(ability_s a, int max) {
	auto v = player->get(a);
	auto mv = player->basic.abilities[a];
	if(v < mv && max) {
		auto n = mv - v;
		if(n > max)
			n = max;
		player->add(a, rand() % n);
	}
}

static void posion_recovery(ability_s v) {
	if(player->get(v) > 0) {
		add(player, v, -1);
		if(player->is(PoisonResistance))
			add(player, v, -1);
		if(!player->roll(Strenght)) {
			player->fixeffect("PoisonVisual");
			player->damage(1);
		}
	}
}

static void normalize_reputation() {
	if(player->abilities[Reputation] < player->basic.abilities[Reputation])
		player->abilities[Reputation]++;
	else if(player->abilities[Reputation] > player->basic.abilities[Reputation])
		player->abilities[Reputation]--;
}

static direction_s move_direction(point m) {
	if(m.x < 0)
		return West;
	else if(m.y < 0)
		return North;
	else if(m.y >= area->mps)
		return South;
	else
		return East;
}

static void add_hits(creature* player, int v) {
	player->abilities[Hits] += v;
	if(player->abilities[Hits] <= 0)
		player->kill();
}

static void drop_wears(creature* player, int chance) {
	point pt = player->getposition();
	for(auto& e : player->wears) {
		if(!e || e.is(Natural))
			continue;
		if(d100() >= chance)
			continue;
		e.drop(pt);
	}
}

static void drop_treasure(creature* player) {
	if(player->is(Summoned))
		return;
	drop_wears(player, 15);
}

static void drop_throphy(creature* player, int chance) {
	if(player->is(Summoned))
		return;
	auto pm = player->getmonster();
	if(!pm)
		return;
	if(d100() >= chance)
		return;
	point pt = player->getposition();
	item it; it.create(pm->rest);
	it.drop(pt);
}

static void nullify_elements(feat_s v1, feat_s v2) {
	if(player->is(v1) && player->is(v2)) {
		player->remove(v1);
		player->remove(v2);
	}
}

static void nullify_elements(ability_s v1, ability_s v2) {
	if(player->is(v1) && player->is(v2)) {
		player->set(v1, 0);
		player->set(v2, 0);
	}
}

static void nullify_elements() {
	nullify_elements(Burning, Freezing);
	nullify_elements(FastAction, SlowAction);
	nullify_elements(FastAttack, SlowAttack);
	nullify_elements(FastMove, SlowMove);
	nullify_elements(Enemy, Ally);
}

static int get_next_level_experience(int level) {
	if(!level)
		return 0;
	static int levels[] = {500, 1000, 1500, 2500, 3500, 5000, 7000, 9000, 12000, 15000, 18000};
	const auto m = sizeof(levels) / sizeof(levels[0]);
	auto n = maptbl(levels, level - 1);
	if(level >= m)
		n += (level - m + 1) * 10000;
	return n;
}

static bool need_levelup(int experience, int level) {
	return experience >= get_next_level_experience(level);
}

static int additional_skill_points(int v) {
	auto r = 0;
	if(v >= 50)
		r++;
	if(v >= 70)
		r++;
	if(v >= 90)
		r++;
	if(v >= 100)
		r++;
	return r;
}

static void advance_value(variant v) {
	if(v.iskind<itemi>())
		player->wearable::equip(bsdata<itemi>::elements + v.value, v.counter);
	else if(v.iskind<feati>())
		ftscript<feati>(v.value, v.counter);
	else if(v.iskind<modifieri>())
		ftscript<modifieri>(v.value, v.counter);
	else if(v.iskind<abilityi>())
		ftscript<abilityi>(v.value, v.counter);
	else if(v.iskind<spelli>())
		player->learn_spell(v.value);
	else if(v.iskind<script>())
		bsdata<script>::elements[v.value].proc(v.counter);
}

static void advance_value(variants elements) {
	for(auto v : elements)
		advance_value(v);
}

static void advance_value(variant kind, int level) {
	for(auto& e : bsdata<advancement>()) {
		if(e.type != kind)
			continue;
		if(e.level < 0) {
			if(level <= 1)
				continue;
			if((level % -e.level) != 0)
				continue;
		} else if(e.level != level)
			continue;
		advance_value(e.elements);
		player->update();
	}
}

void player_levelup() {
	player->basic.abilities[Level] += 1;
	if(!player->ischaracter())
		return;
	if(player->basic.abilities[Level] > 1) {
		player->basic.abilities[SkillPoints] += additional_skill_points(player->basic.abilities[Wits]);
		player->actp("LevelUp");
	}
	advance_value(player->getkind(), player->basic.abilities[Level]);
}

static void check_levelup() {
	while(need_levelup(player->experience, player->basic.abilities[Level]))
		player_levelup();
}

static void check_burning() {
	if(player->is(Burning)) {
		if(!player->resist(FireResistance, FireImmunity))
			player->damage(xrand(1, 3));
		player->add(Burning, -1);
	}
}

static void check_freezing() {
	if(player->is(Freezing)) {
		if(!player->resist(ColdResistance, ColdImmunity)) {
			player->slowdown(100);
			damage_backpack_item(Potion, 20);
		}
		player->add(Freezing, -1);
	}
}

static void check_corrosion() {
	if(player->is(Corrosion)) {
		if(!player->resist(AcidResistance, AcidImmunity)) {
			player->damage(player->get(Corrosion));
			damage_equipment(1, true);
		}
		player->add(Corrosion, -1);
	}
}

static void check_blooding() {
	if(player->is(Blooding)) {
		player->damage(1);
		area->setflag(player->getposition(), Blooded);
		if(player->roll(Strenght))
			player->remove(Blooding);
	}
}

static void check_stun() {
	if(player->is(Stun)) {
		if(player->roll(Strenght))
			player->remove(Stun);
		if(player->is(StunResistance) && player->roll(Strenght))
			player->remove(Stun);
	}
}

static void check_mood() {
	if(player->abilities[Mood] != 0) {
		if(player->abilities[Mood] > 0)
			player->abilities[Mood]--;
		else
			player->abilities[Mood]++;
	}
}

static void random_walk() {
	if(d100() < 60)
		pay_action();
	else {
		static direction_s allaround[] = {North, South, East, West};
		move_step(maprnd(allaround));
	}
}

static bool check_stairs_movement(point m) {
	auto& ei = area->getfeature(m);
	auto pf = ei.getlead();
	if(pf) {
		if(player->ishuman()) {
			if(yesno(getnm(str("Move%1", ei.id)))) {
				enter_area(game.position, game.level + ei.lead, pf, Center);
				return false;
			}
		}
	}
	return true;
}

static void check_illness_effect() {
	if(!player->is(Illness))
		return;
	auto minimal_hp = player->getmaximum(Hits) / 3;
	if(player->abilities[Hits] > minimal_hp)
		add_hits(player, -1);
}

static void check_illness_cure() {
	if(player->abilities[Illness] > 0) {
		if(player->resist(DiseaseResist, DiseaseImmunity) || player->roll(Strenght))
			player->abilities[Illness]--;
		else if(d100() < 20)
			illness_attack(player, 1); // Disease have 20% chance to progress
	}
}

static bool check_dangerous_feature(point m) {
	auto& ei = area->getfeature(m);
	if(ei.is(DangerousFeature)) {
		pay_action();
		pay_action();
		if(!player->roll(Strenght)) {
			player->act("Entagled", ei.id);
			player->damage(1);
			player->fixactivity();
			return false;
		}
		player->act("Break", ei.id);
		area->setfeature(m, 0);
	}
	return true;
}

static void check_trap(point m) {
	if(player->is(Fly))
		return;
	auto& ei = area->getfeature(m);
	if(ei.is(TrappedFeature)) {
		auto bonus = area->is(m, Hidden) ? -30 : 20;
		if(!player->roll(Wits, bonus)) {
			player->act(ei.id, 0, getnm(ei.id));
			script_run(ei.effect);
		}
		if(!player->is(Local))
			area->remove(m, Hidden);
	}
}

static bool check_webbed_tile(point m) {
	if(player->is(IgnoreWeb))
		return true;
	if(player->is(Webbed)) {
		pay_action();
		pay_action();
		if(!player->roll(Strenght)) {
			player->act("WebEntagled");
			player->fixactivity();
			return false;
		}
		player->act("WebBreak");
		area->remove(m, Webbed);
	}
	return true;
}

static bool check_leave_area(point m) {
	if(!area->isvalid(m) && game.level == 0) {
		if(player->ishuman()) {
			auto direction = move_direction(m);
			auto np = to(game.position, direction);
			if(yesno(getnm("LeaveArea"), getnm(bsdata<directioni>::elements[direction].id)))
				enter_area(np, 0, 0, direction);
		}
		pay_action();
		return false;
	}
	return true;
}

static bool check_place_owner(point m) {
	if(player->is(PlaceOwner)) {
		auto pr = roomi::find(m);
		if(player->getroom() != pr) {
			pay_action();
			return false;
		}
	}
	return true;
}

static void detect_hidden_objects() {
	if(!last_location)
		return;
	auto floor = last_location->floors;
	indexa source;
	source.select(player->getposition(), imin(player->getlos(), 2));
	source.match(Visible, true);
	// Secrect doors
	for(auto m : source) {
		auto& ei = area->getfeature(m);
		if(ei.isvisible())
			continue;
		if(!player->roll(Alertness))
			continue;
		player->actp("YouFoundSecretDoor");
		area->setreveal(m, floor);
		break;
	}
	// Traps
	for(auto m : source) {
		if(!area->is(m, Hidden))
			continue;
		auto& ei = area->getfeature(m);
		if(!ei.is(TrappedFeature))
			continue;
		if(!player->roll(Alertness))
			continue;
		area->remove(m, Hidden);
		player->actp("YouDetectTrap", 0, area->getfeature(m).getname());
		break;
	}
}

static void drop_item(point m, const char* id) {
	if(!area->isvalid(m) || !id)
		return;
	itemi* pi = single(id);
	if(!pi)
		return;
	item it; it.create(pi);
	it.drop(m);
}

static bool check_stuck_doors(creature* p, point m, const featurei& ei) {
	if(!ei.is(StuckFeature))
		return false;
	if(p->roll(Strenght, -10)) {
		p->act("YouOpenStuckDoor", 0, getnm(ei.id));
		area->setactivate(m);
		area->setactivate(m);
	} else {
		auto random_table = bsdata<randomizeri>::find(str("%1%2", ei.id, "Fail"));
		if(random_table) {
			auto effect = random_table->random();
			if(effect.iskind<listi>()) {
				auto p = bsdata<listi>::elements + effect.value;
				player->act(p->id, 0, getnm(ei.id));
				pushvalue push_rect(last_rect, {m.x, m.y, m.x, m.y});
				script_run(p->elements);
			}
		}
		movable::fixeffect(m2s(m), "SearchVisual");
		area->setfeature(m, 0);
	}
	return true;
}

bool check_activate(creature* player, point m, const featurei& ei) {
	auto pa = ei.getactivate();
	if(!pa)
		return false;
	auto activate_item = ei.activate_item;
	if(activate_item) {
		if(ei.random_count)
			activate_item.value += area->param[m] % ei.random_count;
		if(activate_item.iskind<itemi>()) {
			if(!player->useitem(bsdata<itemi>::elements + activate_item.value, true)) {
				player->actp("NoActivateItem", ei.id, bsdata<itemi>::elements[activate_item.value].getname());
				return false;
			} else
				player->actp("UseActivateItem", ei.id, bsdata<itemi>::elements[activate_item.value].getname());
		}
	}
	area->setactivate(m);
	return true;
}

static int chance_cut_wood(const item& weapon) {
	auto& ei = player->wears[MeleeWeapon].geti();
	auto chance = player->get(Strenght) / 10 + ei.weapon.damage;
	if(chance < 1)
		chance = 1;
	if(ei.is(TwoHanded))
		chance *= 2;
	return chance;
}

static bool check_cut_wood(creature* player, point m, const featurei& ei) {
	if(ei.is(Woods) && player->wears[MeleeWeapon].geti().is(CutWoods)) {
		if(player->roll(Woodcutting)) {
			player->act("YouCutWood", 0, getnm(ei.id));
			area->setfeature(m, 0);
			drop_item(m, "WoodenLagsTable");
			return true;
		}
	}
	return false;
}

static bool issame(point m, tilef f) {
	if(!area->isvalid(m))
		return false;
	return bsdata<tilei>::elements[area->tiles[m]].is(f);
}

static bool issame(point m, direction_s d, tilef f) {
	if(!issame(m, f))
		return false;
	auto m1 = to(m, round(d, East));
	auto m2 = to(m, round(d, West));
	if(!area->isvalid(m1) || !area->isvalid(m2))
		return false;
	if(bsdata<featurei>::elements[area->features[m1]].is(BetweenWalls))
		return false;
	if(bsdata<featurei>::elements[area->features[m2]].is(BetweenWalls))
		return false;
	return true;
}

static bool check_mining(creature* player, point m) {
	if(!player->wears[MeleeWeapon].geti().is(CutMines))
		return false;
	if(!issame(m, Mines))
		return false;
	auto direction = area->getdirection(player->getposition(), m);
	if(!issame(m, direction, Mines) || !m.in({2, 2, areamap::mps - 2, areamap::mps - 2})) {
		player->actp("YouCantMineHere");
		return false;
	}
	if(player->roll(Mining)) {
		player->act("YouCrashWall", 0, getnm(bsdata<tilei>::elements[area->tiles[m]].id));
		area->setfeature(m, 0);
		area->settile(m, getfloor());
		if(d100() < 25) {
			drop_item(m, "MiningOre");
			if(player->roll(Gemcutting, -2000))
				drop_item(m, "MiningGem");
		}
		return true;
	} else if(d100() < 10)
		damage_item(player->wears[MeleeWeapon]);
	return false;
}

static void check_interaction(creature* player, point m) {
	auto& ei = area->getfeature(m);
	if(ei.isvisible()) {
		if(check_stuck_doors(player, m, ei))
			return;
		if(check_activate(player, m, ei))
			return;
		if(check_cut_wood(player, m, ei))
			return;
	}
}

static void look_items(creature* player, point m) {
	items.clear();
	items.select(m);
	if(!items)
		return;
	if(player->ishuman()) {
		auto index = 0;
		char temp[4096]; stringbuilder sb(temp);
		auto count = items.getcount();
		auto payment_cost = player->getpaymentcost();
		sb.add(getnm("ThereIsLying"));
		for(auto p : items) {
			if(index > 0) {
				if(index == count - 1)
					sb.adds(getnm("And"));
				else
					sb.add(",");
			}
			sb.adds("%-1", p->getfullname(payment_cost));
			index++;
		}
		sb.add(".");
		actv(console, temp, 0, '\n');
	}
}

static void update_boost(featable& feats, variant parent) {
	feats.clear();
	for(auto& e : bsdata<boosti>()) {
		if(e.parent != parent)
			continue;
		if(e.effect.iskind<feati>())
			feats.set(e.effect.value);
	}
}

static void update_basic(char* dest, const char* source) {
	memcpy(dest, source, Hits * sizeof(statable::abilities[0]));
}

static bool spell_allowmana(const void* object) {
	auto p = (spelli*)object;
	return player->get(Mana) >= p->mana;
}

static bool spell_allowuse(const void* object) {
	auto p = (spelli*)object;
	if(p->summon)
		return true;
	return allow_targets(p->use);
}

static bool spell_iscombat(const void* object) {
	auto p = (spelli*)object;
	if(p->adventure)
		return false;
	return true;
}

static bool	spell_isnotcombat(const void* object) {
	return !spell_iscombat(object);
}

void creature::clear() {
	memset(this, 0, sizeof(*this));
	worldpos = {-1000, -1000};
	moveorder = {-1000, -1000};
	guardorder = {-1000, -1000};
	setroom(0);
	setowner(0);
	if(game.getowner() == this)
		game.setowner(0);
}

bool isfreecr(point m) {
	if(findalive(m))
		return false;
	auto tile = area->tiles[m];
	if(bsdata<tilei>::elements[tile].is(CanSwim))
		return false;
	return area->isfree(m);
}

bool isfreecrfly(point m) {
	if(findalive(m))
		return false;
	return area->isfree(m);
}

static void update_room(creature* player) {
	auto pn = roomi::find(player->getposition());
	if(pn) {
		auto pb = player->getroom();
		auto room_changed = false;
		if(pn != pb) {
			player->actp("Look", pn->geti().id);
			room_changed = true;
		}
		player->setroom(pn);
		if(room_changed)
			fire_trigger(WhenCreatureP1EnterSiteP2, player->getkind(), &pn->geti());
	} else
		player->setroom(0);
}

void creature::update_abilities() {
	abilities[DamageMelee] += get(Strenght) / 15;
	abilities[DamageRanged] += get(Dexterity) / 10;
	abilities[Armor] += get(Strenght) / 15;
	if(is(Stun)) {
		abilities[WeaponSkill] /= 2;
		abilities[BalisticSkill] /= 2;
		abilities[Dodge] -= 40;
	}
	if(is(LightSource))
		abilities[LineOfSight] += 3;
	if(is(Freezing))
		abilities[Armor] += 2;
	if(is(Burning))
		abilities[Dodge] -= 20;
	if(ispresent()) {
		if(!is(IgnoreWeb) && is(Webbed)) {
			abilities[WeaponSkill] -= 10;
			abilities[Dodge] -= 20;
		}
	}
}

void creature::place(point m) {
	if(!area)
		return;
	m = area->getfree(m, 10, isfreecr);
	setposition(m);
	update_room(this);
}

bool creature::ishuman() const {
	return game.getowner() == this;
}

static bool is_enemy(const creature* p1, const creature* p2) {
	if(p1->getenemy() == p2)
		return true;
	auto r = p1->getreputation();
	if(r < 0)
		return p2->getreputation() >= 0;
	else if(r > 0)
		return p2->getreputation() < 0;
	return false;
}

static bool is_enemy(const void* object) {
	auto opponent = (creature*)object;
	if(is_enemy(player, opponent))
		return true;
	if(is_enemy(opponent, player))
		return true;
	opponent = opponent->getowner();
	if(opponent) {
		if(is_enemy(player, opponent))
			return true;
		if(is_enemy(opponent, player))
			return true;
	}
	return false;
}

bool creature::isenemy(const creature& opponent) const {
	return opponent.is(is(Enemy) ? Ally : Enemy);
}

bool creature::is(areaf v) const {
	if(!area)
		return false;
	return ispresent() && area->is(getposition(), v);
}

static void opponent_interaction() {
	if(opponent->isenemy(*player))
		attack_melee(0);
	else if(!player->ishuman())
		pay_movement();
	else if(opponent->is(PlaceOwner)) {
		player->fixactivity();
		opponent->wait();
		if(d100() < 40 && opponent->canspeak())
			opponent->speak("DontPushMePlaceOwner");
		pay_movement();
	} else {
		auto pt = opponent->getposition();
		opponent->setposition(player->getposition());
		opponent->fixmovement();
		opponent->wait();
		update_room(opponent);
		player->setposition(pt);
		update_room(player);
		player->fixmovement();
		if(d100() < 40 && opponent->canspeak())
			opponent->speak("DontPushMe");
		pay_movement();
	}
}

static ability_s weapon_skill(const item& weapon) {
	auto& ei = weapon.geti();
	switch(ei.wear) {
	case RangedWeapon: return BalisticSkill;
	default: return WeaponSkill;
	}
}

static int add_bonus_damage(creature* enemy, item& weapon, feat_s feat, int value, feat_s resistance, feat_s immunity) {
	if(!player->is(feat, weapon))
		return 0;
	auto bonus_damage = value;
	if(immunity && enemy->is(immunity))
		bonus_damage = 0;
	else if(resistance && enemy->is(resistance))
		bonus_damage /= 2;
	return bonus_damage;
}

static void apply_pierce(int& armor, int pierce) {
	if(armor > 0) {
		if(pierce > armor)
			armor = 0;
		else
			armor -= pierce;
	} else
		armor = 0;
}

static void make_attack(item& weapon, int attack_skill, int damage_percent) {
	auto enemy_name = opponent->getname();
	auto attacker_name = player->getname();
	auto weapon_ability = weapon_skill(weapon);
	auto weapon_damage = (int)weapon.geti().weapon.damage;
	auto damage = weapon_damage;
	damage += player->get(damage_ability(weapon_ability));
	damage += add_bonus_damage(opponent, weapon, FireDamage, 2, FireResistance, FireImmunity);
	damage += add_bonus_damage(opponent, weapon, ColdDamage, 2, ColdResistance, ColdImmunity);
	if(weapon.isidentified()) { // Blessed or artifact weapon effective only if identified
		if(weapon.is(Blessed))
			damage += 1; // Blessed weapon do more damage
		else if(weapon.is(Artifact))
			damage += 2; // Artifact weapon do more damage
	}
	attack_skill += player->get(weapon_ability);
	auto roll_result = d100();
	if(roll_result <= attack_skill)
		damage += xrand(0, 2);
	else
		damage -= xrand(1, 3);
	if(roll_result <= attack_skill - 30)
		damage += weapon_damage;
	if(roll_result <= attack_skill - 60)
		damage += weapon_damage;
	if(damage_percent)
		damage = damage * damage_percent / 100;
	auto pierce = (int)weapon.geti().weapon.pierce;
	if(roll_result < attack_skill / 3)
		special_attack(weapon, opponent, pierce, damage); // If hit critical
	auto armor = opponent->get(Armor);
	apply_pierce(armor, pierce);
	auto damage_result = damage - armor;
	if(damage_result > 0 && weapon.is(Cursed) && (d100() < 50)) // Cursed weapon miss half time
		damage_result = 0;
	if(damage_result <= 0) {
		player->logs(getnm("AttackMiss"), damage_result, opponent->getname(), roll_result, damage, -armor);
		return;
	}
	auto block_damage = opponent->get(Block);
	if(block_damage > 0) {
		damage_result -= xrand(0, block_damage);
		if(damage_result <= 0) {
			opponent->fixvalue(getnm("Block"), ColorGreen);
			return;
		}
	}
	if(opponent->roll(Dodge)) {
		player->logs(getnm("AttackHitButEnemyDodge"), opponent->getname());
		opponent->fixvalue(getnm("Dodge"), ColorGreen);
	} else {
		player->logs(getnm("AttackHit"), damage_result, opponent->getname(), roll_result, damage, -armor);
		opponent->damage(damage_result);
		poison_attack(weapon);
		if(opponent->is(Retaliate, opponent->wears[MeleeWeapon])) {
			if(!player->roll(Dodge))
				player->damage(1);
		}
	}
	if(roll_result >= 95 && d100() < 30)
		weapon.damage();
}

void creature::kill() {
	if(d100() < 40)
		area->setflag(getposition(), Blooded);
	logs(getnm("ApplyKill"));
	auto human_killed = ishuman();
	fixeffect("HitVisual");
	fixremove();
	drop_treasure(this);
	drop_throphy(this, 30);
	if(opponent == this && player)
		player->experience += get_experience_reward(opponent);
	fire_trigger(WhenCreatureP1Dead, getkind());
	clear();
	if(human_killed)
		end_game();
}

void creature::damage(int v) {
	if(v <= 0)
		return;
	fixvalue(-v);
	add_hits(this, -v);
}

static void turn_to_opponent() {
	player->setdirection(area->getdirection(player->getposition(), opponent->getposition()));
}

static void drop_to_opponent(const itemi& ei) {
	item it;
	it.create(&ei, 1);
	it.setcount(1);
	it.drop(opponent->getposition());
}

void attack_melee(int bonus) {
	player->fixactivity();
	auto number_attackers = opponent->get(EnemyAttacks);
	if(number_attackers > 3)
		number_attackers = 3;
	make_attack(player->wears[MeleeWeapon], number_attackers * 10, 100);
	pay_attack(player->wears[MeleeWeapon]);
	opponent->add(EnemyAttacks, 1);
}

static bool can_shoot() {
	if(!opponent) {
		player->actp("YouDontSeeAnyEnemy");
		return false;
	}
	if(!player->wears[RangedWeapon]) {
		player->actp("YouNeedRangeWeapon");
		return false;
	}
	auto ammo = player->wears[RangedWeapon].geti().weapon.ammunition;
	if(ammo && !player->wears[Ammunition].is(*ammo)) {
		player->actp("YouNeedAmmunition", 0, ammo->getname());
		return false;
	}
	return true;
}

void attack_range(int bonus) {
	if(!can_shoot())
		return;
	turn_to_opponent();
	auto& weapon = player->wears[RangedWeapon];
	auto pa = weapon.geti().weapon.ammunition;
	if(pa)
		player->fixshoot(opponent->getposition(), pa->wear_index);
	else
		player->fixactivity();
	make_attack(weapon, 0, 100);
	pay_attack(weapon);
	if(pa) {
		player->wears[Ammunition].use();
		if(d100() < 50)
			drop_to_opponent(*pa);
	}
}

static bool can_thrown() {
	if(!player->wears[MeleeWeapon].is(Thrown)) {
		player->actp("YouNeedThrownWeapon");
		return false;
	}
	return true;
}

void attack_thrown(int bonus) {
	if(!can_thrown())
		return;
	if(!opponent) {
		player->actp("YouDontSeeAnyEnemy");
		return;
	}
	turn_to_opponent();
	player->fixthrown(opponent->getposition(), "FlyingItem", player->wears[MeleeWeapon].geti().getindex());
	make_attack(player->wears[MeleeWeapon], 0, 100);
	pay_attack(player->wears[MeleeWeapon]);
	drop_to_opponent(player->wears[MeleeWeapon].geti());
	player->wears[MeleeWeapon].use();
}

static bool isfreeplace(creature* player, point ni) {
	if(player->is(Fly))
		return isfreecrfly(ni);
	return isfreecr(ni);
}

static void move_step(point ni) {
	if(!check_leave_area(ni))
		return;
	if(!check_place_owner(ni))
		return;
	auto m = player->getposition();
	if(!check_dangerous_feature(m))
		return;
	if(!check_stairs_movement(ni))
		return;
	if(!check_webbed_tile(m))
		return;
	auto push_opponent = opponent;
	opponent = findalive(ni);
	if(opponent)
		opponent_interaction();
	else if(isfreeplace(player, ni)) {
		player->setposition(ni);
		update_room(player);
		player->fixmovement();
		look_items(player, player->getposition());
		pay_movement();
	} else {
		check_mining(player, ni);
		check_interaction(player, ni);
		player->fixactivity();
		pay_movement();
	}
	opponent = push_opponent;
	check_trap(player->getposition());
}

void move_step(direction_s v) {
	if(player->is(Iced)) {
		if(!player->roll(Dexterity, 30)) {
			player->act("IcedSlice");
			v = round(v, (d100() < 50) ? NorthWest : NorthEast);
			pay_action();
		}
	}
	player->setdirection(v);
	move_step(to(player->getposition(), v));
}

void creature::finish() {
	update();
	abilities[Hits] = getmaximum(Hits);
	abilities[Mana] = getmaximum(Mana);
	fixappear();
}

bool creature::roll(ability_s v, int bonus) const {
	auto value = get(v);
	auto base = bsdata<abilityi>::elements[v].base;
	auto skill_divider = bsdata<abilityi>::elements[v].skill_divider;
	if(base && skill_divider) {
		// Skills depend on roll.
		auto base_value = get(base) / skill_divider;
		if(base_value > value)
			value = base_value;
	}
	if(value <= 0)
		return false;
	last_roll_result = d100();
	if(bonus == -2000) {
		value = value / 2;
		bonus = 0;
	} else if(bonus == -3000) {
		value = value / 3;
		bonus = 0;
	}
	last_value = (value - last_roll_result) / 10;
	if(last_roll_result <= 5)
		last_roll_successed = true;
	else if(last_roll_result >= 95)
		last_roll_successed = false;
	else
		last_roll_successed = last_roll_result <= (value + bonus);
	return last_roll_successed;
}

void adventure_mode();

static void look_creatures() {
	creatures.select(player->getposition(), player->getlos(), player->ishuman(), player);
	if(player->is(Ally)) {
		enemies = creatures;
		enemies.match(Enemy, true);
	} else if(player->is(Enemy)) {
		enemies = creatures;
		enemies.match(Ally, true);
	} else
		enemies.clear();
}

static void ready_enemy() {
	opponent = 0;
	if(enemies) {
		enemies.sort(player->getposition());
		opponent = enemies[0];
	}
}

static void ready_actions() {
	look_creatures();
	ready_enemy();
	allowed_spells.select(player);
	last_actions.clear();
}

static int compare_actions(const void* v1, const void* v2) {
	auto p1 = *((siteskilli**)v1);
	auto p2 = *((siteskilli**)v2);
	if(p1->key != p2->key)
		return p1->key - p2->key;
	return szcmp(p1->getname(), p2->getname());
}

static void ready_skills() {
	last_actions.select(fntis<siteskilli, &siteskilli::isusable>);
}

int creature::getloh() const {
	return 2 + get(Wits) / 10;
}

bool creature::canhear(point i) const {
	return area->getrange(getposition(), i) <= getloh();
}

bool creature::isfollowmaster() const {
	auto master = getowner();
	if(!master)
		return false;
	const int bound_range = 2;
	if(area->getrange(master->getposition(), getposition()) <= bound_range)
		return false;
	return true;
}

static const sitei* get_site(creature* p) {
	return p->getroom() ? &p->getroom()->geti() : 0;
}

void make_move_long() {
	if(player->wait_seconds < 100 * 6)
		return;
	player->wait_seconds -= 100 * 6;
}

static void use_skills() {
	ready_skills();
	if(last_actions) {
		last_action = last_actions.random();
		script_execute("ApplyAction");
	} else
		pay_action();
}

static void use_spells() {
	allowed_spells.select(player);
}

void make_move() {
	// Recoil form action
	if(player->wait_seconds > 0) {
		player->wait_seconds -= 25;
		return;
	}
	pushvalue push_action(last_action);
	pushvalue push_player(opponent);
	pushvalue push_rect(last_rect, player->getposition().rectangle());
	pushvalue push_site(last_site, get_site(player));
	player->set(EnemyAttacks, 0);
	player->update();
	nullify_elements();
	check_blooding();
	if(!player->is(Local))
		detect_hidden_objects();
	check_burning();
	check_freezing();
	check_corrosion();
	if(!(*player))
		return; // Dead from blooding, burning, cold or other bad
	check_levelup();
	ready_actions();
	if(player->ishuman()) {
		ready_skills();
		last_actions.sort(compare_actions);
		adventure_mode();
	} else if(opponent) {
		allowed_spells.select(player);
		allowed_spells.match(spell_iscombat, true);
		allowed_spells.match(spell_allowmana, true);
		allowed_spells.match(spell_allowuse, true);
		if(allowed_spells && d100() < 70)
			cast_spell(*((spelli*)allowed_spells.random()));
		else if(can_shoot())
			attack_range(0);
		else if(can_thrown() && d100() < 60)
			attack_thrown(0);
		else
			player->moveto(opponent->getposition());
	} else {
		allowed_spells.match(spell_isnotcombat, true);
		allowed_spells.match(spell_allowmana, true);
		allowed_spells.match(spell_allowuse, true);
		if(player->isfollowmaster())
			player->moveto(player->getowner()->getposition());
		else if(d100() < 20)
			use_skills();
		else if(allowed_spells && d100() < 20)
			cast_spell(*((spelli*)allowed_spells.random()));
		else if(area->isvalid(player->moveorder)) {
			if(player->moveorder == player->getposition())
				player->moveorder = {-1000, -1000};
			else if(!player->moveto(player->moveorder))
				player->moveorder = {-1000, -1000};
		} else if(area->isvalid(player->guardorder)) {
			if(player->guardorder != player->getposition())
				player->moveorder = player->guardorder;
		} else
			random_walk();
	}
}

static void wearing(variant v, int multiplier) {
	if(v.iskind<abilityi>())
		player->abilities[v.value] += v.counter * multiplier;
	else if(v.iskind<feati>()) {
		auto f = (feat_s)v.value;
		if(multiplier < 0) {
			auto f1 = negative_feat(f);
			if(f1) {
				f = f1;
				multiplier = -multiplier;
			}
		}
		multiplier = v.counter * multiplier;
		if(multiplier >= 0)
			player->feats_active.set(v.value);
		else
			player->feats_active.remove(v.value);
	} else if(v.iskind<listi>()) {
		for(auto e : bsdata<listi>::elements[v.value].elements)
			wearing(e, multiplier);
	}
}

static void update_wears() {
	for(auto& e : player->equipment()) {
		if(!e)
			continue;
		for(auto v : e.geti().wearing)
			wearing(v, 1);
		auto power = e.getpower();
		if(power)
			wearing(power, e.geteffect());
	}
}

static void update_room_abilities() {
	auto p = player->getroom();
	if(!p)
		return;
	fire_trigger(WhenCreatureP1InSiteP2UpdateAbilities, player->getkind(), &p->geti());
}

static void update_negative_skills() {
	for(auto i = (ability_s)0; i < Hits; i = (ability_s)(i + 1)) {
		if(player->abilities[i] < 0)
			player->abilities[i] = 0;
	}
}

void creature::update() {
	auto push_player = player;
	player = this;
	update_basic(abilities, basic.abilities);
	update_boost(feats_active, this);
	update_wears();
	update_room_abilities();
	update_abilities();
	update_negative_skills();
	player = push_player;
}

static void block_creatures(creature* exclude) {
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos != game)
			continue;
		if(&e == exclude)
			continue;
		area->setblock(e.getposition(), 0xFFFF);
	}
}

static void block_swimable() {
	for(auto& e : bsdata<tilei>()) {
		if(e.is(CanSwim))
			area->blocktiles(bsid(&e));
	}
}

bool creature::moveto(point ni) {
	area->clearpath();
	if(!is(Fly))
		block_swimable();
	area->blockwalls();
	area->blockfeatures();
	block_creatures(this);
	area->makewave(getposition());
	area->blockzero();
	auto m0 = getposition();
	auto m1 = area->getnext(m0, ni);
	if(!area->isvalid(m1))
		return false;
	move_step(area->getdirection(m0, m1));
	return true;
}

void creature::unlink() {
	remove_boost(this);
	for(auto& e : bsdata<creature>()) {
		if(e.getowner() == this)
			e.setowner(0);
	}
}

bool creature::act(const char* action, const char* id, ...) const {
	if(!ishuman() && !is(Visible))
		return true;
	update_console_time();
	pushvalue push_player(player, const_cast<creature*>(this));
	return actn(console, id, action, xva_start(id), ' ');
}

bool creature::actp(const char* action, const char* id, ...) const {
	if(!ishuman())
		return true;
	update_console_time();
	pushvalue push_player(player, const_cast<creature*>(this));
	return actn(console, id, action, xva_start(id), ' ');
}

void creature::say(const char* format, ...) const {
	if(ishuman() || is(Visible)) {
		update_console_time();
		pushvalue push_player(player, const_cast<creature*>(this));
		sayv(console, format, xva_start(format));
	}
}

int	creature::getlos() const {
	auto r = get(LineOfSight) - area->darkness;
	auto m = 1;
	if(is(Darkvision))
		m++;
	if(is(Local)) // Local folk knows his place better that you
		m++;
	if(r < m)
		r = m;
	if(r > 4)
		r = 4;
	return r;
}

bool creature::isvalid() const {
	return worldpos == game;
}

bool creature::speechrumor() const {
	collection<quest> source;
	source.select();
	if(!source)
		return false;
	auto seed = game.getminutes() / (60 * 12);
	auto p = (quest*)source.data[seed % source.getcount()];
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	getrumor(*p, sb);
	say(temp);
	return true;
}

bool creature::speechlocation() const {
	rooma source;
	source.collectiona::select(area->rooms, fntis<roomi, &roomi::isnotable>);
	source.match(fntis<roomi, &roomi::isexplored>, false);
	auto p = source.random();
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	p->getrumor(sb);
	say(temp);
	return true;
}

void use_item(item& v) {
	if(!v)
		return;
	auto script = v.getuse();
	if(!script) {
		player->actp("ItemNotUsable", 0, v.getname());
		return;
	}
	auto push_item = last_item;
	auto push_state = script_fail; script_fail = false;
	last_item = &v;
	if(player->ishuman())
		last_player_used_wear = player->getwearslot(last_item);
	last_item_fix_action("UseItem");
	script_run(script);
	if(script_fail)
		player->actp("ItemFailScript", 0, v.getname());
	else {
		auto chance_consume = last_item->chance_consume();
		if(chance_consume) {
			if(d100() < chance_consume) {
				last_item_fix_action("ConsumeItem");
				v.use();
			}
		} else
			v.use();
	}
	script_fail = push_state;
	last_item = push_item;
	player->update();
	pay_action();
}

void creature_every_minute() {
	if(player->is(Regeneration))
		restore(Hits, Strenght);
	if(player->is(ManaRegeneration))
		magic_restore(Mana, 3);
	restore(Mana, Wits);
	check_stun();
	check_mood();
	posion_recovery(Poison);
}

void creature_every_10_minutes() {
	restore(Hits, Strenght);
	check_illness_effect();
	normalize_reputation();
}

void creature_every_day_part() {
	check_illness_cure();
}

bool creature::ispresent() const {
	return worldpos == game;
}

int	creature::getpaymentcost() const {
	auto room = getroom();
	if(!room)
		return 0;
	auto keeper = room->getowner();
	if(!keeper)
		return 0;
	auto skill_delta = keeper->get(Wits) - get(Wits);
	return 200 + skill_delta;
}

int	creature::getsellingcost() const {
	auto room = getroom();
	if(!room)
		return 0;
	auto keeper = room->getowner();
	if(!keeper)
		return 0;
	auto skill_delta = get(Wits) - keeper->get(Wits);
	auto result = 40 + skill_delta;
	if(result < 10)
		result = 10;
	return result;
}

creature* player_create(point m, variant kind, bool female) {
	if(!kind)
		return 0;
	pushvalue push_player(player);
	player = bsdata<creature>::addz();
	player->clear();
	player->worldpos = game;
	player->setkind(kind);
	player->setnoname();
	if(female)
		player->set(Female);
	monsteri* pm = kind;
	if(pm) {
		copy(player->basic, *pm);
		advance_value(pm->use);
		player->setname(random_charname(pm->id));
		player->basic.abilities[LineOfSight] += 4;
	} else {
		char temp[64]; stringbuilder sb(temp);
		sb.addv(kind.getid(), 0);
		if(player->is(Female))
			sb.addv("Female", 0);
		player->setname(random_charname(temp));
		player->basic.abilities[LineOfSight] += 4;
		advance_value(kind, 0);
		player_levelup();
	}
	player->place(m);
	player->finish();
	if(pm) {
		if(pm->friendly <= -10)
			player->set(Enemy);
	}
	if(player->is(PlaceOwner)) {
		auto pr = player->getroom();
		if(pr)
			pr->setowner(player);
	}
	return player;
}

bool creature::isallow(const item& it) const {
	auto& ei = it.geti();
	for(auto i = Strenght; i <= Wits; i = (ability_s)(i + 1)) {
		auto v = ei.required[i - Strenght];
		if(v && get(i) < v)
			return false;
	}
	return true;
}

void creature::equipi(short unsigned type, int count) {
	item it; it.create(bsdata<itemi>::elements + type, count);
	it.createpower(0);
	if(isallow(it))
		equip(it);
	else
		wearable::additem(it);
}

bool creature::canremove(item& it) const {
	if(it.iscursed()) {
		it.setidentified(true);
		speak("ThisIsMyCursedItem", 0, it.getname());
		return false;
	}
	return true;
}

roomi* creature::getroom() const {
	if(!area)
		return 0;
	return area->rooms.ptrs(room_id);
}

void creature::setroom(const roomi* v) {
	room_id = (v == 0) ? 0xFFFF : area->rooms.indexof(v);
}

int	creature::getmaximum(ability_s v) const {
	switch(v) {
	case Hits: case Mana:
		return basic.abilities[v];
	default:
		return 0;
	}
}

bool ispresent(const void* p) {
	return ((creature*)p)->ispresent();
}

int get_maximum_faith(creature* p) {
	return p->get(Religion) / 4;
}

creature* creature::getenemy() const {
	return (enemy_id != 0xFFFF) ? bsdata<creature>::elements + enemy_id : 0;
}