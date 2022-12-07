#include "boost.h"
#include "charname.h"
#include "condition.h"
#include "indexa.h"
#include "main.h"
#include "siteskill.h"
#include "triggern.h"

namespace {
struct rollg {
	int	roll, chance, delta;
	bool miss() const { return roll >= 5 && roll > chance; }
	void make(int chance);
	void make(ability_s& result, ability_s v, int chance);
};
struct defenceg {
	int dodge, parry, block;
};
}

extern collection<roomi> rooms;
extern siteskilla last_actions;
void apply_spell(creature* player, const spelli& ei, int level);
bool choose_targets(unsigned flags, const variants& effects);

void rollg::make(int v) {
	roll = 1 + rand() % 100;
	chance = v;
	delta = v - roll;
}

void rollg::make(ability_s& result, ability_s v, int difficult) {
	if(!difficult)
		return;
	auto d = difficult - roll;
	if(d <= delta)
		return;
	result = v;
	chance = difficult;
	delta = d;
}

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static void fixdamage(const creature* p, int total, int damage_weapon, int damage_armor, int damage_bonus) {
	p->logs(getnm("ApplyDamage"), total, damage_weapon, damage_armor, damage_bonus);
}

static creature* findalive(point m) {
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid() && e.getposition() == m)
			return &e;
	}
	return 0;
}

static void pay_movement(creature* player) {
	auto cost = 100;
	if(!player->is(Fly)) {
		auto& ei = area.getfeature(player->getposition());
		if(ei.movedifficult)
			cost = cost * ei.movedifficult / 100;
	}
	player->waitseconds(cost);
}

static void pay_attack(creature* player, const item& weapon) {
	auto cost = 110 - weapon.get(FO(itemstat, speed)) * 2;
	player->waitseconds(cost);
}

static ability_s damage_ability(ability_s v) {
	switch(v) {
	case BalisticSkill: return DamageRanged;
	default: return DamageMelee;
	}
}

static bool attack_effect(const creature* p, const item& w, feat_s v) {
	if(w.is(v))
		return true;
	return p->is(v);
}

static bool resist_test(creature* player, feat_s resist, feat_s immunity) {
	if(player->is(immunity))
		return true;
	if(player->is(resist)) {
		if(d100() < 50)
			return true;
	}
	return false;
}

static void poison_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(resist_test(player, PoisonResistance, PoisonImmunity))
		return;
	if(player->roll(Strenght, -value))
		return;
	auto v = player->get(Poison) + value;
	player->fixeffect("PoisonVisual");
	if(v >= player->get(HitsMaximum))
		player->kill();
	else
		player->set(Poison, v);
}

static void poison_attack(const creature* player, creature* enemy, const item& weapon) {
	auto strenght = 0;
	if(attack_effect(player, weapon, WeakPoison))
		strenght += xrand(1, 3);
	if(attack_effect(player, weapon, StrongPoison))
		strenght += xrand(2, 6);
	if(attack_effect(player, weapon, DeathPoison))
		strenght += xrand(3, 9);
	poison_attack(enemy, strenght);
}

static void illness_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(resist_test(player, DiseaseResist, DiseaseImmunity))
		return;
	auto v = player->get(Illness) + value;
	if(v >= player->get(Strenght))
		player->kill();
	else
		player->set(Illness, v);
}

static void special_attack(creature* player, const item& weapon, creature* enemy, int& pierce, int& damage) {
	if(attack_effect(player, weapon, BleedingHit))
		enemy->set(Blooding);
	if(attack_effect(player, weapon, StunningHit)) {
		enemy->set(Stun);
		enemy->fixeffect("SearchVisual");
	}
	if(attack_effect(player, weapon, PierceHit))
		pierce += 4;
	if(attack_effect(player, weapon, MightyHit))
		damage += 2;
}

static void acid_attack(creature* player, int value) {
	player->damage(value);
	for(auto& e : player->equipment()) {
		if(value <= 0)
			break;
		if(!e)
			continue;
		if(d100() < (60 - value * 4))
			continue;
		value--;
		if(resist_test(player, AcidResistance, AcidImmunity))
			continue;
		e.damage();
	}
}

static void restore(creature* player, ability_s a, ability_s am, ability_s test) {
	auto v = player->get(a);
	auto mv = player->get(am);
	if(v < mv) {
		if(player->roll(test))
			player->add(a, 1);
	}
}

static void add(creature* player, ability_s id, int value, int minimal = 0) {
	auto i = player->get(id) + value;
	if(i < minimal)
		i = minimal;
	player->set(id, i);
}

static void posion_recovery(creature* player, ability_s v) {
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

static direction_s movedirection(point m) {
	if(m.x < 0)
		return West;
	else if(m.y < 0)
		return North;
	else if(m.y >= area.mps)
		return South;
	else
		return East;
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

static void drop_treasure(creature* pe) {
	if(pe->is(Summoned))
		return;
	drop_wears(pe, 15);
	monsteri* p = pe->getkind();
	if(!p)
		return;
	pushvalue push_player(player, pe);
	pushvalue push_rect(last_rect, pe->getposition().rectangle());
	script::run(p->treasure);
}

static void check_blooding(creature* p) {
	if(p->is(Blooding)) {
		p->damage(1);
		area.setflag(p->getposition(), Blooded);
		if(p->roll(Strenght))
			p->remove(Blooding);
	}
}

static void check_stun(creature* p) {
	if(p->is(Stun)) {
		if(p->roll(Strenght))
			p->remove(Stun);
	}
}

static void random_walk(creature* p) {
	if(d100() < 60)
		p->wait();
	else {
		static direction_s allaround[] = {North, South, East, West};
		p->movestep(maprnd(allaround));
	}
}

static bool check_stairs_movement(creature* p, point m) {
	auto& ei = area.getfeature(m);
	auto pf = ei.getlead();
	if(pf) {
		if(p->ishuman()) {
			if(draw::yesno(getnm(str("Move%1", ei.id)))) {
				game.enter(game.position, game.level + ei.lead, pf, Center);
				return false;
			}
		}
	}
	return true;
}

static bool check_dangerous_feature(creature* p, point m) {
	auto& ei = area.getfeature(m);
	if(ei.is(DangerousFeature)) {
		p->wait(2);
		if(!p->roll(Strenght)) {
			p->act(getnme(str("%1Entagled", ei.id)));
			p->damage(1);
			p->wait();
			p->fixaction();
			return false;
		}
		p->act(getnme(str("%1Break", ei.id)));
		area.setfeature(m, 0);
	}
	return true;
}

static bool check_trap(creature* p, point m) {
	auto& ei = area.getfeature(m);
	if(ei.is(TrappedFeature)) {
		auto bonus = ei.isvisible() ? 20 : -30;
		if(!p->roll(Wits, bonus)) {
			auto pn = getdescription(ei.id);
			if(pn)
				p->act(pn, getnm(ei.id));
			p->apply(ei.effect);
		}
	}
	return true;
}

static bool check_webbed_tile(creature* p, point m) {
	if(p->is(IgnoreWeb))
		return true;
	if(area.is(m, Webbed)) {
		p->wait(2);
		if(!p->roll(Strenght)) {
			p->act(getnm("WebEntagled"));
			p->wait();
			p->fixaction();
			return false;
		}
		p->act(getnm("WebBreak"));
		area.remove(m, Webbed);
	}
	return true;
}

static bool check_leave_area(creature* p, point m) {
	if(!area.isvalid(m) && game.level == 0) {
		if(p->ishuman()) {
			auto direction = movedirection(m);
			auto np = to(game.position, direction);
			if(draw::yesno(getnm("LeaveArea"), getnm(bsdata<directioni>::elements[direction].id)))
				game.enter(np, 0, 0, direction);
		}
		p->wait();
		return false;
	}
	return true;
}

static bool check_place_owner(creature* p, point m) {
	if(p->is(PlaceOwner)) {
		auto pr = roomi::find(p->worldpos, m);
		if(p->getroom() != pr) {
			p->wait();
			return false;
		}
	}
	return true;
}

static void check_hidden_doors(creature* p) {
	if(!last_location)
		return;
	auto floor = last_location->floors;
	indexa source;
	source.select(p->getposition(), 1);
	auto found_doors = 0;
	for(auto m : source) {
		auto& ei = area.getfeature(m);
		if(ei.isvisible())
			continue;
		if(!p->roll(Wits, -3000))
			continue;
		found_doors++;
		area.setreveal(m, floor);
	}
	if(found_doors)
		p->actp(getnm("YouFoundSecretDoor"), found_doors);
}

static void drop_item(point m, const char* id) {
	if(!area.isvalid(m) || !id)
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
		area.setactivate(m);
		area.setactivate(m);
		p->act(getnm("YouOpenStuckDoor"), getnm(ei.id));
	} else {
		auto random_table = bsdata<randomizeri>::find(str("%1%2", ei.id, "Fail"));
		if(random_table) {
			auto effect = random_table->random();
			if(effect.iskind<listi>()) {
				auto p = bsdata<listi>::elements + effect.value;
				auto pn = getdescription(p->id);
				if(pn)
					player->act(pn, getnm(ei.id));
				pushvalue push_rect(last_rect, {m.x, m.y, m.x, m.y});
				script::run(p->elements);
			}
		}
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
			activate_item.value += area.random[m] % ei.random_count;
		if(activate_item.iskind<itemi>()) {
			if(!player->useitem(bsdata<itemi>::elements + activate_item.value)) {
				player->actp(getnm(str("%1%2", ei.id, "NoActivateItem")), bsdata<itemi>::elements[activate_item.value].getname());
				return false;
			} else
				player->actp(getnm(str("%1%2", ei.id, "UseActivateItem")), bsdata<itemi>::elements[activate_item.value].getname());
		}
	}
	area.setactivate(m);
	return true;
}

static int chance_cut_wood(const item& weapon) {
	auto& ei = player->wears[MeleeWeapon].geti();
	auto chance = player->get(Strenght) / 10 + ei.damage;
	if(chance < 1)
		chance = 1;
	if(ei.is(TwoHanded))
		chance *= 2;
	return chance;
}

static bool check_cut_wood(creature* player, point m, const featurei& ei) {
	if(ei.is(Woods) && player->wears[MeleeWeapon].geti().is(CutWoods)) {
		auto chance = chance_cut_wood(player->wears[MeleeWeapon]);
		if(d100() < chance) {
			player->act(getnm("YouCutWood"), getnm(ei.id));
			area.setfeature(m, 0);
			drop_item(m, "WoodenLagsTable");
			return true;
		}
	}
	return false;
}

static void check_interaction(creature* player, point m) {
	auto& ei = area.getfeature(m);
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
	if(player->ishuman() && items) {
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
		player->actp(temp);
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

static int might_penalty(int strenght, int enemy_strenght) {
	if(strenght < enemy_strenght)
		return enemy_strenght - strenght;
	return 0;
}

static int parry_skill(const item& enemy_weapon, const item& weapon, int value) {
	if(!weapon)
		return 0;
	if(enemy_weapon.is(RangedWeapon))
		return 0;
	value += enemy_weapon.get(FO(itemstat, enemy_parry));
	value += weapon.get(FO(itemstat, parry));
	return value;
}

static int block_skill(const item& enemy_weapon, const item& weapon, int value) {
	if(!weapon)
		return 0;
	value += enemy_weapon.get(FO(itemstat, enemy_block));
	if(enemy_weapon.is(RangedWeapon))
		value += weapon.get(FO(itemstat, block_ranged));
	else
		value += weapon.get(FO(itemstat, block));
	return value;
}

static void defence_skills(defenceg& result, const creature* player, int attacker_strenght, const item& attacker_weapon) {
	auto penalty = might_penalty(player->get(Strenght), attacker_strenght);
	result.parry = parry_skill(attacker_weapon, player->wears[MeleeWeapon], player->get(WeaponSkill)) - penalty;
	result.block = block_skill(attacker_weapon, player->wears[MeleeWeaponOffhand], player->get(ShieldUse)) - penalty;
	result.dodge = player->get(DodgeSkill);
	if(result.dodge > 80)
		result.dodge = 80;
	if(result.parry < 0)
		result.parry = 0;
	if(result.block < 0)
		result.block = 0;
	if(result.dodge < 0)
		result.dodge = 0;
}

static bool spell_allowmana(const void* object) {
	auto p = (spelli*)object;
	return player->get(Mana) >= p->mana;
}

static bool spell_allowuse(const void* object) {
	auto p = (spelli*)object;
	if(p->summon.size() != 0)
		return true;
	return choose_targets(p->target, p->effect);
}

static bool spell_iscombat(const void* object) {
	auto p = (spelli*)object;
	if(p->is(Enemies))
		return true;
	if(p->summon)
		return true;
	return false;
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

void creature::levelup() {
	basic.abilities[Level] += 1;
}

bool isfreecr(point m) {
	if(findalive(m))
		return false;
	auto tile = area.tiles[m];
	if(bsdata<tilei>::elements[tile].is(CanSwim))
		return false;
	return area.isfree(m);
}

bool isfreecrfly(point m) {
	if(findalive(m))
		return false;
	auto tile = area.tiles[m];
	if(bsdata<tilei>::elements[tile].is(CanSwim))
		return false;
	return area.isfree(m);
}

static void update_room(creature* player) {
	auto pn = roomi::find(player->worldpos, player->getposition());
	if(pn) {
		auto pb = player->getroom();
		auto room_changed = false;
		if(pn != pb) {
			if(player->ishuman()) {
				auto pd = getdescription(pn->geti().id);
				if(pd)
					player->actp(pd);
			}
			room_changed = true;
		}
		player->setroom(pn);
		if(room_changed)
			trigger::fire(WhenCreatureP1EnterSiteP2, player->getkind(), &pn->geti());
	} else
		player->setroom(0);
}

void creature::place(point m) {
	m = area.getfree(m, 10, isfreecr);
	setposition(m);
	update_room(this);
}

creature* creature::create(point m, variant kind, variant character, bool female) {
	if(!kind)
		return 0;
	if(!character)
		character = "Monster";
	if(!character.iskind<classi>())
		return 0;
	pushvalue push_player(player);
	player = bsdata<creature>::addz();
	player->clear();
	player->worldpos = game;
	player->setkind(kind);
	player->setnoname();
	player->class_id = character.value;
	if(female)
		player->set(Female);
	monsteri* pm = kind;
	if(pm) {
		copy(player->basic, *pm);
		player->advance(pm->use);
		adat<variant> conditions;
		conditions.add(kind);
		player->setname(charname::random(conditions));
	} else {
		adat<variant> conditions;
		conditions.add(kind);
		if(player->is(Female))
			conditions.add("Female");
		player->setname(charname::random(conditions));
	}
	player->basic.abilities[LineOfSight] += 4;
	player->advance(kind, 0);
	if(character.value)
		player->advance(character, 0);
	player->levelup();
	player->finish();
	player->place(m);
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

bool creature::ishuman() const {
	return game.getowner() == this;
}

bool creature::isenemy(const creature& opponent) const {
	return opponent.is(is(Enemy) ? Ally : Enemy);
}

void creature::movestep(direction_s v) {
	if(area.is(getposition(), Iced)) {
		if(!roll(Dexterity, 30)) {
			act(getnm("IcedSlice"));
			v = round(v, (d100() < 50) ? NorthWest : NorthEast);
			wait();
		}
	}
	setdirection(v);
	movestep(to(getposition(), v));
}

bool creature::matchspeech(variant v) const {
	if(v.iskind<monsteri>())
		return iskind(v);
	else if(v.iskind<conditioni>())
		return is((condition_s)v.value);
	return true;
}

bool creature::matchspeech(const variants& source) const {
	for(auto v : source) {
		if(!matchspeech(v))
			return false;
	}
	return true;
}

void creature::matchspeech(speecha& source) const {
	auto ps = source.data;
	for(auto p : source) {
		if(!matchspeech(p->condition))
			continue;
		*ps++ = p;
	}
	source.count = ps - source.data;
}

const speech* creature::matchfirst(const speecha& source) const {
	auto ps = source.data;
	for(auto p : source) {
		if(!matchspeech(p->condition))
			continue;
		return p;
	}
	return 0;
}

const char* creature::getspeech(const char* id, bool always_speak) const {
	speecha source;
	source.select(id);
	if(!source) {
		if(!always_speak)
			return 0;
		return getnm("NothingToSay");
	}
	auto conditional = source[0]->condition.count > 0;
	if(conditional) {
		auto p = matchfirst(source);
		if(!p)
			return getnm("NothingToSay");
		return p->name;
	} else
		return source.getrandom();
}

void creature::interaction(creature& opponent) {
	if(opponent.isenemy(*this))
		attackmelee(opponent);
	else if(!ishuman())
		pay_movement(this);
	else if(opponent.is(PlaceOwner)) {
		fixaction();
		opponent.wait();
		if(d100() < 40 && !opponent.is(AnimalInt))
			opponent.speech("DontPushMePlaceOwner");
		pay_movement(this);
	} else {
		auto pt = opponent.getposition();
		opponent.setposition(getposition());
		opponent.fixmovement();
		opponent.wait();
		update_room(&opponent);
		setposition(pt);
		update_room(this);
		fixmovement();
		if(d100() < 40 && !opponent.is(AnimalInt))
			opponent.speech("DontPushMe");
		pay_movement(this);
	}
}

static ability_s best_defence(rollg& check, const defenceg& defences) {
	auto result = (ability_s)0;
	zclear(check);
	check.roll = 1 + rand() % 100;
	check.make(result, WeaponSkill, defences.parry);
	check.make(result, ShieldUse, defences.block);
	check.make(result, DodgeSkill, defences.dodge);
	return result;
}

static void apply_damage(creature* player, const item& weapon, int effect) {
	auto weapon_damage = weapon.get(FO(itemstat, damage));
	auto damage_reduction = player->get(Armor) - weapon.get(FO(itemstat, pierce));
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage - damage_reduction + effect;
	fixdamage(player, result_damage, weapon_damage, -damage_reduction, effect);
	player->damage(result_damage);
}

static ability_s weapon_skill(const item& weapon) {
	auto& ei = weapon.geti();
	switch(ei.wear) {
	case RangedWeapon: return BalisticSkill;
	default: return WeaponSkill;
	}
}

static void make_attack(creature* player, creature* enemy, item& weapon, int attack_skill, int damage_multiplier) {
	rollg check, parry;
	defenceg defence;
	auto enemy_name = enemy->getname();
	auto attacker_name = player->getname();
	auto weapon_ability = weapon_skill(weapon);
	auto weapon_damage = weapon.get(FO(itemstat, damage)) + player->get(damage_ability(weapon_ability));
	if(enemy->is(Undead) && weapon.is(NoDamageUndead))
		weapon_damage = 0;
	check.make(attack_skill + player->get(weapon_ability));
	auto attack_miss = check.miss();
	if(!attack_miss && weapon.is(MissHalfTime))
		attack_miss = true;
	if(attack_miss) {
		player->logs(getnm("AttackMiss"), check.roll, check.chance);
		return;
	}
	defence_skills(defence, enemy, player->get(Strenght), weapon);
	auto parry_ability = best_defence(parry, defence);
	auto ability_id = "";
	if(parry_ability) {
		ability_id = bsdata<abilityi>::elements[parry_ability].id;
		if(parry.delta > check.delta) {
			player->logs(getnm("AttackHitButParryCritical"), check.roll, check.chance, parry.roll, parry.chance, enemy_name, getnm(ability_id));
			if(parry_ability == MeleeWeapon && enemy->wears[MeleeWeapon].is(Retaliate)) {
				auto range = area.getrange(enemy->getposition(), player->getposition());
				if(range <= 1)
					apply_damage(enemy, enemy->wears[MeleeWeapon], (parry.delta - check.delta) / 10);
			} else {
				switch(parry_ability) {
				case DodgeSkill: enemy->fixvalue(getnm("DodgeSuccess"), 2); break;
				case ShieldUse: enemy->fixvalue(getnm("BlockSuccess"), 2); break;
				case WeaponSkill: enemy->fixvalue(getnm("ParrySuccess"), 2); break;
				}
			}
			return;
		}
	}
	if(parry_ability)
		player->logs(getnm("AttackHitButParrySuccess"), check.roll, check.chance, parry.roll, parry.chance, enemy_name, getnm(ability_id));
	else
		player->logs(getnm("AttackHitButParryFail"), check.roll, check.chance, parry.roll, enemy->getname(), defence.parry, defence.block, defence.dodge);
	int pierce = weapon.get(FO(itemstat, pierce));
	int armor = enemy->get(Armor);
	if(weapon.geti().ismelee())
		armor += (enemy->get(Strenght) - player->get(Strenght)) / 10;
	if(check.roll < check.chance / 2)
		special_attack(player, weapon, enemy, pierce, weapon_damage);
	if(armor > 0) {
		if(pierce > armor)
			armor = 0;
		else
			armor -= pierce;
	}
	int result_damage = weapon_damage - armor + (check.delta - parry.delta) / 10;
	fixdamage(enemy, result_damage, weapon_damage, -armor, (check.delta - parry.delta) / 10);
	result_damage = result_damage * damage_multiplier / 100;
	enemy->damage(result_damage);
	poison_attack(player, enemy, weapon);
}

void creature::heal(int v) {
	if(v <= 0)
		return;
	v += abilities[Hits];
	if(v > abilities[HitsMaximum])
		v = abilities[HitsMaximum];
	if(v != abilities[Hits]) {
		fixvalue(v - abilities[Hits]);
		abilities[Hits] = v;
	}
}

void creature::kill() {
	auto human_killed = ishuman();
	fixeffect("HitVisual");
	fixremove();
	drop_treasure(this);
	trigger::fire(WhenCreatureP1Dead, getkind());
	clear();
	if(human_killed)
		game.endgame();
}

void creature::damage(int v) {
	if(v <= 0) {
		logs(getnm("ArmorNegateDamage"));
		return;
	}
	fixvalue(-v);
	abilities[Hits] -= v;
	if(abilities[Hits] <= 0) {
		if(d100() < 40)
			area.setflag(getposition(), Blooded);
		logs(getnm("ApplyKill"), v);
		kill();
	}
}

void creature::attackmelee(creature& enemy) {
	fixaction();
	auto number_attackers = enemy.get(EnemyAttacks);
	if(number_attackers > 3)
		number_attackers = 3;
	make_attack(this, &enemy, wears[MeleeWeapon], number_attackers * 10, 100);
	pay_attack(this, wears[MeleeWeapon]);
	enemy.add(EnemyAttacks, 1);
}

bool creature::canshoot(bool interactive) const {
	if(!wears[RangedWeapon]) {
		if(interactive)
			actp(getnm("YouNeedRangeWeapon"));
		return false;
	}
	auto ammo = wears[RangedWeapon].geti().ammunition;
	if(ammo && !wears[Ammunition].is(*ammo)) {
		if(interactive)
			actp(getnm("YouNeedAmmunition"), ammo->getname());
		return false;
	}
	return true;
}

bool creature::canthrown(bool interactive) const {
	if(!wears[MeleeWeapon].is(Thrown)) {
		if(interactive)
			actp(getnm("YouNeedThrownWeapon"));
		return false;
	}
	return true;
}

void creature::attackrange(creature& enemy) {
	if(!canshoot(false))
		return;
	auto pa = wears[RangedWeapon].geti().ammunition;
	if(pa)
		fixshoot(enemy.getposition(), pa->wear_index);
	else
		fixaction();
	make_attack(this, &enemy, wears[RangedWeapon], 0, 100);
	pay_attack(this, wears[RangedWeapon]);
	if(pa) {
		wears[Ammunition].use();
		if(d100() < 50) {
			item it;
			it.create(pa, 1);
			it.setcount(1);
			it.drop(enemy.getposition());
		}
	}
}

void creature::attackthrown(creature& enemy) {
	if(!canthrown(false))
		return;
	fixthrown(enemy.getposition(), "FlyingItem", wears[MeleeWeapon].geti().getindex());
	make_attack(this, &enemy, wears[MeleeWeapon], 0, 100);
	pay_attack(this, wears[MeleeWeapon]);
	item it;
	it.create(&wears[MeleeWeapon].geti(), 1);
	it.setcount(1);
	it.drop(enemy.getposition());
	wears[MeleeWeapon].use();
}

void creature::fixcantgo() const {
	fixaction();
	act(getnm("CantGoThisWay"));
}

void creature::movestep(point ni) {
	if(!check_leave_area(this, ni))
		return;
	if(!check_place_owner(this, ni))
		return;
	auto m = getposition();
	if(!check_dangerous_feature(this, m))
		return;
	if(!check_stairs_movement(this, ni))
		return;
	if(!check_webbed_tile(this, m))
		return;
	auto opponent = findalive(ni);
	if(opponent)
		interaction(*opponent);
	else if(isfreecr(ni)) {
		setposition(ni);
		update_room(this);
		fixmovement();
		look_items(this, getposition());
		pay_movement(this);
	} else {
		check_interaction(this, ni);
		fixaction();
		pay_movement(this);
	}
	check_trap(this, getposition());
}

void creature::finish() {
	update();
	abilities[Hits] = get(HitsMaximum);
	abilities[Mana] = get(ManaMaximum);
	fixappear();
}

void creature::advance(variant kind, int level) {
	for(auto& e : bsdata<advancement>()) {
		if(e.type == kind && e.level == level)
			advance(e.elements);
	}
}

void creature::advance(variants elements) {
	for(auto v : elements)
		advance(v);
}

void creature::advance(variant v) {
	if(v.iskind<abilityi>()) {
		last_ability = (ability_s)v.value;
		if(v.counter > 0) {
			if(last_cap && basic.abilities[v.value] >= last_cap) {
				last_cap = 0;
				return;
			}
			last_cap = 0;
		} else if(v.counter < 0) {
			if(last_cap && basic.abilities[v.value] <= last_cap) {
				last_cap = 0;
				return;
			}
			last_cap = 0;
		}
		switch(v.value) {
		case Experience: gainexperience(v.counter); break;
		case Satiation: satiation += v.counter; break;
		case Money: money += v.counter * 10; break;
		default: basic.abilities[v.value] += v.counter; break;
		}
	} else if(v.iskind<itemi>()) {
		item it;
		it.create(bsdata<itemi>::elements + v.value, game.getpositivecount(v));
		equip(it);
	} else if(v.iskind<feati>()) {
		if(v.counter < 0)
			feats.remove(v.value);
		else
			feats.set(v.value);
	} else if(v.iskind<spelli>())
		spells[v.value] += v.counter;
}

bool creature::roll(ability_s v, int bonus) const {
	auto value = get(v);
	auto result = d100();
	if(bonus == -2000) {
		value = value / 2;
		bonus = 0;
	} else if(bonus == -3000) {
		value = value / 3;
		bonus = 0;
	}
	last_value = (value - result) / 10;
	if(result <= 5)
		return true;
	else if(result >= 95)
		return false;
	return result <= (value + bonus);
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
	enemy = 0;
	if(enemies) {
		enemies.sort(player->getposition());
		enemy = enemies[0];
	}
}

static void ready_actions() {
	look_creatures();
	ready_enemy();
	allowed_spells.select(player);
	last_actions.select(siteskilli::isvalid);
}

int creature::getloh() const {
	return 2 + get(Wits) / 10;
}

bool creature::canhear(point i) const {
	return area.getrange(getposition(), i) <= getloh();
}

bool creature::isfollowmaster() const {
	auto master = getowner();
	if(!master)
		return false;
	const int bound_range = 2;
	if(area.getrange(master->getposition(), getposition()) <= bound_range)
		return false;
	return true;
}

static const sitei* get_site(creature* p) {
	return p->getroom() ? &p->getroom()->geti() : 0;
}

void creature::makemovelong() {
	if(wait_seconds < 100 * 6)
		return;
	wait_seconds -= 100 * 6;
}

void creature::makemove() {
	// Recoil form action
	if(wait_seconds > 0) {
		wait_seconds -= get(Speed);
		return;
	}
	pushvalue push_player(player, this);
	pushvalue push_rect(last_rect, getposition().rectangle());
	pushvalue push_site(last_site, get_site(this));
	set(EnemyAttacks, 0);
	update();
	check_blooding(this);
	if(!is(Local))
		check_hidden_doors(this);
	ready_actions();
	if(ishuman())
		adventure_mode();
	else if(enemy) {
		allowed_spells.match(spell_iscombat, true);
		allowed_spells.match(spell_allowmana, true);
		allowed_spells.match(spell_allowuse, true);
		if(allowed_spells && d100() < 40)
			cast(*((spelli*)allowed_spells.random()));
		if(canshoot(false))
			attackrange(*enemy);
		else
			moveto(enemy->getposition());
	} else {
		allowed_spells.match(spell_isnotcombat, true);
		allowed_spells.match(spell_allowmana, true);
		allowed_spells.match(spell_allowuse, true);
		if(allowed_spells && d100() < 20)
			cast(*((spelli*)allowed_spells.random()));
		else if(isfollowmaster())
			moveto(getowner()->getposition());
		else if(area.isvalid(moveorder)) {
			if(moveorder == getposition())
				moveorder = {-1000, -1000};
			else if(!moveto(moveorder))
				moveorder = {-1000, -1000};
		} else if(area.isvalid(guardorder)) {
			if(guardorder != getposition())
				moveorder = guardorder;
		} else
			random_walk(this);
	}
	check_stun(this);
}

static int getmultiplier(magic_s v) {
	switch(v) {
	case Cursed: return -1;
	default: return 1;
	}
}

static int getmagicbonus(magic_s v) {
	switch(v) {
	case Cursed: return -2;
	case Blessed: return 1;
	case Artifact: return 2;
	default: return 0;
	}
}

void creature::dress(variant v) {
	if(v.iskind<abilityi>())
		abilities[v.value] += v.counter;
	else if(v.iskind<listi>())
		dress(bsdata<listi>::elements[v.value].elements);
	else if(v.iskind<feati>()) {
		if(v.counter > 0)
			feats.set(v.value);
		else if(v.counter < 0)
			feats.remove(v.value);
	}
}

void creature::dress(variants source) {
	for(auto v : source)
		dress(v);
}

static void apply_dress(creature* player, const item& e) {
	auto magic = e.getmagic();
	player->dress(e.geti().getdress(magic));
	auto p = e.getprefix();
	if(p)
		player->dress(p->getdress(magic));
	p = e.getsuffix();
	if(p)
		player->dress(p->getdress(magic));
}

static void update_dress(creature* player, const item& e) {
	player->apply(e.geti().feats);
	player->abilities[Armor] += e.get(FO(itemstat, armor));
	player->abilities[DodgeSkill] += e.get(FO(itemstat, dodge));
	player->abilities[Speed] += e.get(FO(itemstat, speed));
}

static void update_wears(creature* player) {
	for(auto& e : player->gears()) {
		if(!e)
			continue;
		update_dress(player, e);
		apply_dress(player, e);
	}
	for(auto& e : player->weapons()) {
		if(!e)
			continue;
		player->abilities[DodgeSkill] += e.get(FO(itemstat, dodge));
		apply_dress(player, e);
	}
}

void creature::update_room_abilities() {
	auto p = getroom();
	if(!p)
		return;
	trigger::fire(WhenCreatureP1InSiteP2UpdateAbilities, getkind(), &p->geti());
}

static void update_negative_skills(creature* player) {
	for(auto i = (ability_s)0; i < Hits; i = (ability_s)(i + 1)) {
		if(!bsdata<abilityi>::elements[i].basic)
			continue;
		if(player->abilities[i] < 0)
			player->abilities[i] = 0;
	}
}

void creature::update() {
	update_basic(abilities, basic.abilities);
	update_boost(feats_active, this);
	update_wears(this);
	update_room_abilities();
	update_abilities();
	update_negative_skills(this);
}

static void block_creatures(creature* exclude) {
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos != game)
			continue;
		if(&e == exclude)
			continue;
		area.setblock(e.getposition(), 0xFFFF);
	}
}

static void block_swimable() {
	for(auto& e : bsdata<tilei>()) {
		if(e.is(CanSwim))
			area.blocktiles(bsid(&e));
	}
}

bool creature::moveto(point ni) {
	area.clearpath();
	if(!is(Fly))
		block_swimable();
	area.blockwalls();
	area.blockfeatures();
	block_creatures(this);
	area.makewave(getposition());
	area.blockzero();
	auto m0 = getposition();
	auto m1 = area.getnext(m0, ni);
	if(!area.isvalid(m1))
		return false;
	movestep(area.getdirection(m0, m1));
	return true;
}

void creature::unlink() {
	boosti::remove(this);
	for(auto& e : bsdata<creature>()) {
		if(e.getowner() == this)
			e.setowner(0);
	}
}

void creature::act(const char* format, ...) const {
	if(game.getowner() == this || area.is(getposition(), Visible))
		actv(console, format, xva_start(format), getname(), is(Female), '\n');
}

void creature::actp(const char* format, ...) const {
	if(ishuman())
		actv(console, format, xva_start(format), getname(), is(Female), '\n');
}

void creature::sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const {
	if(ishuman() || area.is(getposition(), Visible))
		actable::sayv(sb, format, format_param, name, female);
}

int	creature::getlos() const {
	auto r = get(LineOfSight) - areahead.darkness;
	auto m = 1;
	if(is(Darkvision))
		m++;
	// Local folk knows his place better that you
	if(is(Local))
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
	collection<roomi> source;
	source.select(fntis<roomi, &roomi::isnotable>);
	source.match(fntis<roomi, &roomi::isexplored>, false);
	auto p = source.random();
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	p->getrumor(sb);
	say(temp);
	return true;
}

void creature::use(variants source) {
	for(auto v : source)
		apply(v);
}

void creature::use(item& v) {
	if(!v)
		return;
	auto& ei = v.geti();
	if(!ei.use)
		return;
	use(ei.use);
	act(getnm("YouUseItem"), v.getname());
	v.use();
	update();
	wait();
}

void creature::everyminute() {
	if(is(Regeneration))
		restore(this, Hits, HitsMaximum, Strenght);
	if(is(ManaRegeneration))
		restore(this, Mana, ManaMaximum, Charisma);
	posion_recovery(this, Poison);
}

void creature::every10minutes() {
	restore(this, Mana, ManaMaximum, Charisma);
}

void creature::every30minutes() {
}

void creature::every4hour() {
	restore(this, Hits, HitsMaximum, Strenght);
}

void creature::gainexperience(int v) {
	if(v > 0) {
		fixability(Experience, v);
		experience += v;
	}
}

bool creature::isallow(const variants& source) const {
	for(auto v : source) {
		if(!isallow(v))
			return false;
	}
	return true;
}

bool creature::isallow(variant v) const {
	if(v.iskind<conditioni>())
		return is((condition_s)v.value);
	else if(v.iskind<feati>())
		return !is((feat_s)v.value);
	else if(v.iskind<areafi>()) {
		auto present = area.is(getposition(), (areaf)v.value);
		if(v.counter < 0)
			return present;
		return !present;
	} else if(v.iskind<featurei>()) {
		auto m = getposition();
		if(!area.isvalid(m))
			return false;
		auto present = area.features[m];
		return (v.counter < 0) ? present == v.value : present != v.value;
	}
	return true;
}

static void apply_ability(creature* player, ability_s i, int value, int minimum, int maximum, int color) {
	auto current_value = player->abilities[i];
	value += current_value;
	if(value < minimum)
		value = minimum;
	else if(value > maximum)
		value = maximum;
	value -= current_value;
	if(value == 0)
		return;
	player->fixvalue(value, color);
	player->abilities[i] = value;
}

void creature::apply(variant v) {
	if(v.iskind<abilityi>()) {
		last_ability = (ability_s)v.value;
		if(!v.counter)
			return;
		switch(v.value) {
		case Mana: apply_ability(this, last_ability, v.counter, 0, abilities[ManaMaximum], 3); break;
		case Hits: (v.counter < 0) ? damage(-v.counter) : heal(v.counter); break;
		default: advance(v); break;
		}
	} else if(v.iskind<spelli>())
		apply_spell(this, bsdata<spelli>::elements[v.value], v.counter);
	else if(v.iskind<areafi>()) {
		if(v.counter < 0)
			area.remove(getposition(), v.value);
		else
			area.setflag(getposition(), v.value);
	} else if(v.iskind<featurei>()) {
		if(v.counter < 0)
			area.setfeature(getposition(), 0);
		else
			area.setfeature(getposition(), v.value);
	} else
		advance(v);
}

void creature::apply(const variants& source) {
	for(auto v : source)
		apply(v);
}

void creature::cast(const spelli& e) {
	cast(e, get(e), e.mana);
}

void creature::summon(point m, const variants& elements, int count, int level) {
	auto isenemy = is(Enemy);
	auto isally = is(Ally);
	for(auto i = 0; i < count; i++) {
		auto v = randomizeri::random(elements);
		auto p = creature::create(m, v);
		if(isenemy)
			p->set(Enemy);
		else
			p->remove(Enemy);
		if(isally)
			p->set(Ally);
		else
			p->remove(Ally);
		p->set(Summoned);
		p->setowner(this);
	}
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
	auto skill_delta = keeper->get(Charisma) - get(Charisma);
	return 200 + skill_delta;
}

int	creature::getsellingcost() const {
	auto room = getroom();
	if(!room)
		return 0;
	auto keeper = room->getowner();
	if(!keeper)
		return 0;
	auto skill_delta = get(Charisma) - keeper->get(Charisma);
	auto result = 40 + skill_delta;
	if(result < 10)
		result = 10;
	return result;
}