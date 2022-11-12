#include "boost.h"
#include "charname.h"
#include "indexa.h"
#include "main.h"

namespace {
struct rollg {
	int			roll, chance, delta;
	bool		miss() const { return roll >= 5 && roll > chance; }
	void		make(int chance);
	void		make(ability_s& result, ability_s v, int chance);
};
struct defenceg {
	int			dodge, parry, block;
};
}

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

static void action_text(const creature* player, const char* id, const char* action) {
	if(!player->is(AnimalInt)) {
		auto pn = player->getspeech(str("%1%2Speech", id, action));
		if(pn) {
			player->say(pn);
			return;
		}
	}
	auto pn = getdescription(str("%1%2", id, action));
	if(pn)
		player->act(pn);
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

static ability_s damage_ability(wear_s v) {
	switch(v) {
	case RangedWeapon: return DamageRanged;
	default: return DamageMelee;
	}
}

static bool attack_effect(const creature* p, const item& w, feat_s v) {
	if(w.is(v))
		return true;
	return p->is(v);
}

static void poison_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->is(PoisonImmunity))
		return;
	auto bonus = -value;
	if(player->is(PoisonResistance))
		bonus += 30;
	if(player->roll(Strenght, bonus))
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
		strenght += xrand(3, 6);
	if(attack_effect(player, weapon, DeathPoison))
		strenght += xrand(6, 12);
	poison_attack(enemy, strenght);
}

static void illness_attack(creature* player, int value) {
	if(value <= 0)
		return;
	auto v = player->get(Illness) + value;
	//player->fixeffect("PoisonVisual");
	if(v >= player->get(Strenght))
		player->kill();
	else
		player->set(Illness, v);
}

static void special_attack(creature* player, const item& weapon, creature& enemy, int& pierce, int& damage) {
	if(attack_effect(player, weapon, StunningHit)) {
		enemy.set(Stun);
		enemy.fixeffect("SearchVisual");
	}
	if(attack_effect(player, weapon, PierceHit))
		pierce += 4;
	if(attack_effect(player, weapon, MightyHit))
		damage += 2;
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
	runscript(p->treasure);
}

static void check_blooding(creature* p) {
	if(p->is(Blooding)) {
		p->damage(1);
		area.set(p->getposition(), Blooded);
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
			if(answers::confirm(getnm(str("Move%1", ei.id)))) {
				game.enter(game.position, game.level + ei.lead, *pf, Center);
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
		area.set(m, featuren::No);
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
			if(answers::confirm(getnm("LeaveArea"), getnm(bsdata<directioni>::elements[direction].id)))
				game.enter(np, 0, featuren::No, direction);
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
	value += enemy_weapon.geti().weapon.enemy_parry;
	value += weapon.geti().weapon.parry;
	return value;
}

static int block_skill(const item& enemy_weapon, const item& weapon, int value) {
	if(!weapon || !weapon.is(ShieldUse))
		return 0;
	value += enemy_weapon.geti().weapon.enemy_block;
	if(enemy_weapon.is(RangedWeapon))
		value += weapon.geti().weapon.block_ranged;
	else
		value += weapon.geti().weapon.block;
	return value;
}

static void defence_skills(defenceg& result, const creature* player, int attacker_strenght, const item& attacker_weapon) {
	auto penalty = might_penalty(player->get(Strenght), attacker_strenght);
	result.parry = parry_skill(attacker_weapon, player->wears[MeleeWeapon], player->get(WeaponSkill)) - penalty;
	result.block = block_skill(attacker_weapon, player->wears[MeleeWeaponOffhand], player->get(ShieldUse)) - penalty;
	result.dodge = player->get(DodgeSkill);
	if(result.parry < 0)
		result.parry = 0;
	if(result.block < 0)
		result.block = 0;
	if(result.dodge < 0)
		result.dodge = 0;
}

static void match_creatures(const spelli& ei, int level) {
	auto ps = targets.begin();
	for(auto p : targets) {
		if(!p->isallow(ei, level))
			continue;
		*ps++ = p;
	}
	targets.count = ps - targets.begin();
}

static bool spell_ready(const spelli& e, int level) {
	choose_targets(e.target);
	unsigned target_count = 1;
	if(e.is(Multitarget))
		target_count += level;
	if(targets.count > target_count)
		targets.count = target_count;
	match_creatures(e, level);
	return targets.getcount() != 0 || e.summon.size() != 0;
}

static bool spell_allowmana(const void* object) {
	auto p = (spelli*)object;
	return player->get(Mana) >= p->mana;
}

static bool spell_allowuse(const void* object) {
	auto p = (spelli*)object;
	return spell_ready(*p, player->get(*p));
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

static bool isfreecr(point m) {
	if(findalive(m))
		return false;
	auto tile = area[m];
	if(bsdata<tilei>::elements[tile].is(CanSwim))
		return false;
	return area.isfree(m);
}

void creature::place(point m) {
	m = area.getfree(m, 10, isfreecr);
	setposition(m);
	update_room();
}

creature* creature::create(point m, variant kind, variant character, bool female) {
	if(!kind)
		return 0;
	if(!character)
		character = "Monster";
	if(!character.iskind<classi>())
		return 0;
	m = area.getfree(m, 10, isfreecr);
	if(!area.isvalid(m))
		return 0;
	pushvalue push_player(player);
	player = bsdata<creature>::addz();
	player->clear();
	player->setposition(m);
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
	player->update_room();
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

static void drop_item(point m, const char* id) {
	if(!area.isvalid(m) || !id)
		return;
	itemi* pi = single(id);
	if(!pi)
		return;
	item it; it.create(pi);
	it.drop(m);
}

void creature::interaction(point m) {
	auto& ei = area.getfeature(m);
	if(ei.isvisible()) {
		auto pa = ei.getactivate();
		if(pa)
			area.setactivate(m);
		else if(ei.is(Woods) && wears[MeleeWeapon].geti().is(CutWoods)) {
			auto& wei = wears[MeleeWeapon].geti();
			auto chance = get(Strenght) / 10 + wei.weapon.damage;
			if(chance < 1)
				chance = 1;
			if(wei.is(TwoHanded))
				chance *= 2;
			if(d100() < chance) {
				act(getnm("YouCutWood"), getnm(ei.id));
				area.set(m, featuren::No);
				drop_item(m, "WoodenLagsTable");
			}
		}
	}
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

const char* creature::getspeech(const char* id) const {
	speecha source;
	source.select(id);
	if(!source)
		return getnm("NothingToSay");
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
		return;
	else if(opponent.is(PlaceOwner)) {
		fixaction();
		opponent.wait();
		if(d100() < 40 && !opponent.is(AnimalInt))
			opponent.speech("DontPushMePlaceOwner");
		return;
	} else {
		auto pt = opponent.getposition();
		opponent.setposition(getposition());
		opponent.fixmovement();
		opponent.wait();
		setposition(pt);
		fixmovement();
		if(d100() < 40 && !opponent.is(AnimalInt))
			opponent.speech("DontPushMe");
	}
}

int creature::getdamage(wear_s v) const {
	auto result = wears[v].getdamage();
	result += get(damage_ability(v));
	return result;
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

void creature::damage(const item& weapon, int effect) {
	auto weapon_damage = weapon.getdamage();
	auto damage_reduction = get(Armor) - weapon.geti().weapon.pierce;
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage - damage_reduction + effect;
	fixdamage(this, result_damage, weapon_damage, -damage_reduction, effect);
	damage(result_damage);
}

void creature::attack(creature& enemy, wear_s v, int attack_skill, int damage_multiplier) {
	rollg check, parry;
	defenceg defence;
	auto enemy_name = enemy.getname();
	auto attacker_name = getname();
	auto weapon_damage = wears[v].getdamage() + get(damage_ability(v));
	check.make(attack_skill + get(wears[v].geti().ability));
	if(check.miss()) {
		logs(getnm("AttackMiss"), check.roll, check.chance);
		return;
	}
	defence_skills(defence, &enemy, get(Strenght), wears[v]);
	auto parry_ability = best_defence(parry, defence);
	auto ability_id = "";
	if(parry_ability) {
		ability_id = bsdata<abilityi>::elements[parry_ability].id;
		if(parry.delta > check.delta) {
			logs(getnm("AttackHitButParryCritical"), check.roll, check.chance, parry.roll, parry.chance, enemy_name, getnm(ability_id));
			if(parry_ability == MeleeWeapon && enemy.wears[MeleeWeapon].is(Retaliate)) {
				auto range = area.getrange(enemy.getposition(), getposition());
				if(range <= 1)
					damage(enemy.wears[MeleeWeapon], (parry.delta - check.delta) / 10);
			} else {
				switch(parry_ability) {
				case DodgeSkill: enemy.fixvalue(getnm("DodgeSuccess"), 2); break;
				case ShieldUse: enemy.fixvalue(getnm("BlockSuccess"), 2); break;
				case WeaponSkill: enemy.fixvalue(getnm("ParrySuccess"), 2); break;
				}
			}
			return;
		}
	}
	if(parry_ability)
		logs(getnm("AttackHitButParrySuccess"), check.roll, check.chance, parry.roll, parry.chance, enemy_name, getnm(ability_id));
	else
		logs(getnm("AttackHitButParryFail"), check.roll, check.chance, parry.roll, enemy.getname(), defence.parry, defence.block, defence.dodge);
	int pierce = wears[v].geti().weapon.pierce;
	int armor = enemy.get(Armor);
	if(v == MeleeWeapon || v == MeleeWeaponOffhand)
		armor += (enemy.get(Strenght) - get(Strenght)) / 10;
	if(check.roll < check.chance / 2)
		special_attack(this, wears[v], enemy, pierce, weapon_damage);
	if(armor > 0) {
		if(pierce > armor)
			armor = 0;
		else
			armor -= pierce;
	}
	int result_damage = weapon_damage - armor + (check.delta - parry.delta) / 10;
	fixdamage(&enemy, result_damage, weapon_damage, -armor, (check.delta - parry.delta) / 10);
	result_damage = result_damage * damage_multiplier / 100;
	enemy.damage(result_damage);
	poison_attack(this, &enemy, wears[v]);
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
			area.set(getposition(), Blooded);
		logs(getnm("ApplyKill"), v);
		kill();
	}
}

void creature::attackmelee(creature& enemy) {
	fixaction();
	auto number_attackers = enemy.get(EnemyAttacks);
	if(number_attackers > 3)
		number_attackers = 3;
	attack(enemy, MeleeWeapon, number_attackers * 10, 100);
	enemy.add(EnemyAttacks, 1);
}

bool creature::canshoot(bool interactive) const {
	if(!wears[RangedWeapon]) {
		if(interactive)
			actp(getnm("YouNeedRangeWeapon"));
		return false;
	}
	auto ammo = wears[RangedWeapon].geti().getammunition();
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
	auto pa = wears[RangedWeapon].geti().getammunition();
	if(pa)
		fixshoot(enemy.getposition(), pa->wear_index);
	else
		fixaction();
	attack(enemy, RangedWeapon, 0, 100);
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
	attack(enemy, MeleeWeapon, 0, 100);
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

void creature::lookitems() const {
	items.clear();
	items.select(getposition());
	if(ishuman() && items) {
		char temp[4096]; stringbuilder sb(temp);
		auto count = items.getcount();
		auto index = 0;
		sb.add(getnm("ThereIsLying"));
		for(auto p : items) {
			if(index > 0) {
				if(index == count - 1)
					sb.adds(getnm("And"));
				else
					sb.add(",");
			}
			sb.adds("%-1", p->getfullname());
			index++;
		}
		sb.add(".");
		actp(temp);
	}
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
		fixmovement();
		lookitems();
	} else {
		interaction(ni);
		fixaction();
	}
	update_room();
	wait();
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
		switch(v.value) {
		case Experience: gainexperience(v.counter); break;
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
	return p->getroom() ? p->getroom()->getsite() : 0;
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
	case Cursed: return -2;
	case Blessed: return 2;
	case Artifact: return 4;
	default: return 1;
	}
}

void creature::dress(variant v, int multiplier) {
	if(v.iskind<abilityi>())
		abilities[v.value] += v.counter * multiplier;
	else if(v.iskind<listi>())
		dress(bsdata<listi>::elements[v.value].elements, multiplier);
	else if(v.iskind<feati>()) {
		if(v.counter > 0)
			feats.set(v.value);
		else if(v.counter < 0)
			feats.remove(v.value);
	}
}

void creature::dress(variants source, int multiplier) {
	for(auto v : source)
		dress(v, multiplier);
}

void creature::update_wears() {
	for(auto i = MeleeWeapon; i <= Elbows; i = (wear_s)(i + 1)) {
		if(!wears[i])
			continue;
		auto& ei = wears[i].geti();
		auto magic = wears[i].getmagic();
		feats.add(ei.feats);
		if(wears[i].isidentified()) {
			if(ei.dress)
				dress(ei.dress, getmultiplier(magic));
		}
		if(ei.ability) {
			abilities[ei.ability] += ei.bonus;
			switch(magic) {
			case Cursed: abilities[ei.ability] -= 2; break;
			case Blessed: case Artifact: abilities[ei.ability] += 1; break;
			default: break;
			}
		}
	}
}

void creature::update_room() {
	auto pn = roomi::find(worldpos, getposition());
	if(pn) {
		auto pb = getroom();
		auto room_changed = false;
		if(pn != pb) {
			if(ishuman()) {
				auto ps = pn->getsite();
				auto pd = getdescription(ps->id);
				if(pd)
					actp(pd);
			}
			room_changed = true;
		}
		room_id = bsdata<roomi>::source.indexof(pn);
		if(room_changed)
			trigger::fire(WhenCreatureP1EnterSiteP2, getkind(), pn->getsite());
	} else
		room_id = 0xFFFF;
}

void creature::update_room_abilities() {
	auto p = getroom();
	if(!p)
		return;
	trigger::fire(WhenCreatureP1InSiteP2UpdateAbilities, getkind(), p->getsite());
}

void creature::update() {
	update_basic(abilities, basic.abilities);
	update_boost(feats_active, this);
	update_wears();
	update_room_abilities();
	update_abilities();
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
			area.blocktiles((tile_s)bsid(&e));
	}
}

bool creature::moveto(point ni) {
	area.clearpath();
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
	source.select(roomi::notknown);
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

bool creature::isallow(const spelli& e, int level) const {
	if(e.conditions) {
		if(!isallow(e.conditions))
			return false;
	}
	if(e.effect) {
		if(!isallow(e.effect))
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
		if(v.counter < 0)
			return int(present) == v.value;
		return int(present) != v.value;
	} else if(v.iskind<script>())
		return true;
	return false;
}

bool creature::isallow(const variants& source) const {
	for(auto v : source) {
		if(isallow(v))
			return true;
	}
	return false;
}

void creature::apply(const spelli& ei, int level) {
	pushvalue push_value(last_value, ei.getcount(level));
	if(ei.duration) {
		auto minutes = bsdata<durationi>::elements[ei.duration].get(level);
		auto stop_time = game.getminutes() + minutes;
		fixvalue(str("%1 %2i %-Minutes", ei.getname(), minutes), 2);
		for(auto v : ei.effect)
			boosti::add(this, v, stop_time);
	} else
		apply(ei.effect);
}

void creature::apply(variant v) {
	if(v.iskind<spelli>())
		apply(bsdata<spelli>::elements[v.value], v.counter);
	else if(v.iskind<areafi>()) {
		if(v.counter < 0)
			area.remove(getposition(), (areaf)v.value);
		else
			area.set(getposition(), (areaf)v.value);
	} else if(v.iskind<featurei>()) {
		if(v.counter < 0)
			area.set(getposition(), featuren::No);
		else
			area.set(getposition(), (featuren)v.value);
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

void creature::cast(const spelli& e, int level, int mana) {
	if(get(Mana) < mana) {
		actp(getnm("NotEnoughtMana"));
		return;
	}
	if(!spell_ready(e, level)) {
		actp(getnm("YouDontValidTargets"));
		return;
	}
	action_text(this, e.id, "Casting");
	for(auto p : targets)
		p->apply(e, level);
	if(e.summon) {
		auto count = e.getcount(level);
		summon(player->getposition(), e.summon, count, level);
	}
	add(Mana, -mana);
	update();
	wait();
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