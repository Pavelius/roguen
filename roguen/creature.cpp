#include "main.h"

creature* player;

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static creature* findalive(point m) {
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		if(e.getposition() == m)
			return &e;
	}
	return 0;
}

static ability_s damage_ability(wear_s v) {
	switch(v) {
	case RangedWeapon: return DamageRanged;
	case ThrownWeapon: return DamageThrown;
	default: return DamageMelee;
	}
}

void creature::clear() {
	memset(this, 0, sizeof(*this));
	if(player == this)
		player = 0;
}

void creature::levelup() {
}

static bool isfreecr(point m) {
	if(findalive(m))
		return false;
	return area.isfree(m);
}

creature* creature::create(point m, variant kind, variant character) {
	if(!kind)
		return 0;
	if(!character)
		character = "Monster";
	if(!character.iskind<classi>())
		return 0;
	m = area.getfree(m, 10, isfreecr);
	if(!area.isvalid(m))
		return 0;
	auto p = bsdata<creature>::add();
	p->setposition(m);
	p->setkind(kind);
	p->class_id = character.value;
	monsteri* pm = kind;
	if(pm) {
		copy(p->basic, *pm);
		p->feats = pm->feats;
		p->advance(pm->use);
	}
	p->basic.create();
	p->advance(kind, 0);
	if(character.value)
		p->advance(character, 0);
	p->levelup();
	p->finish();
	return p;
}

bool creature::isactive() const {
	return this == player;
}

bool creature::isplayer() const {
	return this == player;
}

bool creature::isenemy(const creature& opponent) const {
	return opponent.is(is(Enemy) ? Ally : Enemy);
}

void creature::movestep(direction_s v) {
	if(area.is(getposition(), Iced)) {
		if(!roll(Dexterity)) {
			act(getnm("IcedSlice"));
			v = round(v, (d100() < 50) ? West : East);
			wait();
		}
	}
	setdirection(v);
	movestep(to(getposition(), v));
}

static void drop_item(point m, const char* id) {
	if(!area.isvalid(m) || !id)
		return;
	itemi* pi = random_value(id);
	if(!pi)
		return;
	item it; it.create(pi);
	it.drop(m);
}

void creature::interaction(point m) {
	if(!area.isvalid(m))
		return;
	auto f = area.features[m];
	if(f) {
		auto& ei = bsdata<featurei>::elements[f];
		if(ei.is(AllowActivate)) {
			if(!area.is(m, Activated))
				area.set(m, Activated);
			else
				area.remove(m, Activated);
		} else if(ei.is(Woods) && wears[MeleeWeapon].geti().is(CutWoods)) {
			auto& wei = wears[MeleeWeapon].geti();
			int chance = 1;
			if(wei.is(TwoHanded))
				chance = wei.bonus * 3;
			else
				chance = wei.bonus;
			if(d100() < chance) {
				act(getnm("YouCutWood"), getnm(ei.id));
				area.features[m] = NoFeature;
				drop_item(m, "WoodenLagsTable");
			}
		}
	}
}

void creature::interaction(creature& opponent) {
	if(opponent.isenemy(*this))
		attackmelee(opponent);
}

int creature::getdamage(wear_s v) const {
	auto result = wears[v].getdamage();
	result += get(damage_ability(v));
	return result;
}

void creature::getdefence(int attacker_strenght, const item& attacker_weapon, defencet& result) const {
	result[0].skill = WeaponSkill;
	result[0].value = getparrying(attacker_weapon, wears[MeleeWeapon], get(WeaponSkill)) - getmightpenalty(attacker_strenght);
	result[1].skill = ShieldUse;
	result[1].value = getblocking(attacker_weapon, wears[MeleeWeaponOffhand], get(ShieldUse)) - getmightpenalty(attacker_strenght);
	result[2].skill = DodgeSkill;
	result[2].value = get(DodgeSkill);
	for(auto& e : result) {
		if(e.value < 0)
			e.value = 0;
	}
}

static ability_s getbestdefence(const defencet& defences, int parry_result, int& parry_skill, int& parry_effect) {
	auto parry = (ability_s)0;
	parry_skill = 0;
	parry_effect = 0;
	for(auto& e : defences) {
		if(parry_result <= e.value) {
			auto v = (e.value - parry_result) / 10;
			if(!parry_skill || parry_effect < v) {
				parry = e.skill;
				parry_skill = e.value;
				parry_effect = v;
			}
		}
	}
	return parry;
}

void creature::damage(const item& weapon, int effect) {
	auto weapon_damage = weapon.getdamage();
	auto damage_reduction = get(DamageReduciton) - weapon.geti().weapon.pierce;
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage - damage_reduction + effect;
	fixdamage(result_damage, weapon_damage, 0, -damage_reduction, effect, 0);
	damage(result_damage);
}

void creature::attack(creature& enemy, wear_s v, int attack_skill, int damage_multiplier) {
	skilli defences[3] = {};
	int parry_skill = 0;
	last_hit = 1 + d100(); last_hit_result = 0;
	last_parry = 1 + d100(); last_parry_result = 0;
	attack_skill += get(wears[v].geti().ability);
	last_hit_result = (attack_skill - last_hit) / 10;
	if(last_hit > 5 && last_hit > attack_skill) {
		logs(getnm("AttackMiss"), last_hit, attack_skill);
		return;
	}
	auto enemy_name = enemy.getname();
	auto attacker_name = getname();
	enemy.getdefence(get(Strenght), wears[v], defences);
	auto parry = getbestdefence(defences, last_parry, parry_skill, last_parry_result);
	if(parry) {
		if(last_parry_result > last_hit_result) {
			logs(getnm("AttackHitButParryCritical"), last_hit, attack_skill, last_parry, parry_skill, enemy.getname(), getnm(bsdata<abilityi>::elements[parry].id));
			if(parry == MeleeWeapon && enemy.wears[MeleeWeapon].is(Retaliate))
				damage(enemy.wears[MeleeWeapon], last_parry_result - last_hit_result);
			return;
		}
	}
	if(parry)
		logs(getnm("AttackHitButParrySuccess"), last_hit, attack_skill, last_parry, parry_skill, enemy.getname(), getnm(bsdata<abilityi>::elements[parry].id));
	else
		logs(getnm("AttackHitButParryFail"), last_hit, attack_skill, last_parry, enemy.getname(), defences[0].value, defences[1].value, defences[2].value);
	auto ability_damage = get(damage_ability(v));
	auto weapon_damage = wears[v].getdamage();
	auto damage_reduction = enemy.get(DamageReduciton) - wears[v].geti().weapon.pierce;
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage + ability_damage - damage_reduction + last_hit_result - last_parry_result;
	enemy.fixdamage(result_damage, weapon_damage, ability_damage, -damage_reduction, last_hit_result, -last_parry_result);
	//result_damage = result_damage * damage_multiplier / 100;
	enemy.damage(result_damage);
}

void creature::fixdamage(int total, int damage_weapon, int damage_strenght, int damage_armor, int damage_skill, int damage_parry) const {
	logs(getnm("ApplyDamage"), total, damage_weapon, damage_strenght, damage_armor,
		damage_skill, damage_parry);
}

void creature::damage(int v) {
	if(v <= 0) {
		logs(getnm("ArmorNegateDamage"));
		return;
	}
	fixvalue(-v);
	abilities[Hits] -= v;
	if(abilities[Hits] <= 0) {
		auto player_killed = isplayer();
		logs(getnm("ApplyKill"), v);
		fixeffect("HitVisual");
		fixremove();
		clear();
		if(player_killed)
			game.endgame();
	}
}

void creature::attackmelee(creature& enemy) {
	fixaction();
	attack(enemy, MeleeWeapon, 0, 100);
}

void creature::fixcantgo() const {
	fixaction();
	act(getnm("CantGoThisWay"));
}

static bool isfreelt(point m) {
	return area.isfree(m);
}

void creature::movestep(point ni) {
	if(!area.isvalid(ni)) {
		if(isactive())
			fixcantgo();
		wait();
		return;
	}
	auto m = getposition();
	if(area.is(m, Webbed)) {
		wait(2);
		if(!roll(Strenght)) {
			act(getnm("WebEntagled"));
			wait();
			fixaction();
			return;
		}
		act(getnm("WebBreak"));
		area.remove(m, Webbed);
	}
	auto opponent = findalive(ni);
	if(opponent)
		interaction(*opponent);
	else if(area.isfree(ni)) {
		setposition(ni);
		fixmovement();
	} else {
		interaction(ni);
		fixaction();
	}
	wait();
}

void creature::finish() {
	update();
	abilities[Hits] = get(HitsMaximum);
	abilities[Mana] = get(ManaMaximum);
	fixappear();
}

void creature::aimove() {
	// When stay on ground you do some work
	if(d100() < 60) {
		wait();
		return;
	}
	static direction_s allaround[] = {North, South, East, West};
	movestep(maprnd(allaround));
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
	if(v.iskind<abilityi>())
		basic.abilities[v.value] += v.counter;
	else if(v.iskind<itemi>()) {
		item it;
		it.create(bsdata<itemi>::elements + v.value, 1);
		equip(it);
	}
}

bool creature::roll(ability_s v, int bonus) const {
	auto value = get(v);
	auto result = d100();
	if(result <= 5)
		return true;
	else if(result >= 95)
		return false;
	return result <= (value + bonus);
}

void adventure_mode();

void creature::lookcreatures() {
	creatures.select(getposition(), getlos());
	if(is(Ally)) {
		enemies = creatures;
		enemies.match(Enemy, true);
	} else if(is(Enemy)) {
		enemies = creatures;
		enemies.match(Ally, true);
	} else
		enemies.clear();
}

void creature::makemove() {
	// Recoil form action
	if(wait_seconds > 0) {
		wait_seconds -= get(Speed);
		return;
	}
	update();
	// Dazzled creature don't make turn
	if(is(Stun)) {
		if(roll(Strenght))
			remove(Stun);
		else {
			wait();
			return;
		}
	}
	// Sleeped creature don't move
	if(is(Sleep))
		return;
	if(is(Unaware))
		remove(Unaware);
	// Get nearest creatures
	lookcreatures();
	creature* enemy = 0;
	if(enemies) {
		// Combat situation - need eliminate enemy
		enemies.sort(getposition());
		enemy = enemies[0];
	}
	if(isactive()) {
		area.setlos(getposition(), getlos(), isfreelt);
		adventure_mode();
	} else if(enemy)
		moveto(enemy->getposition());
	else
		aimove();
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
		feats.add(ei.flags);
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

void creature::update_basic() {
	memcpy(abilities, basic.abilities, Hits * sizeof(abilities[0]));
}

void creature::update_abilities() {
	abilities[DamageMelee] += get(Strenght) / 10;
	abilities[DamageThrown] += get(Strenght) / 10;
	abilities[Speed] += get(Dexterity);
}

void creature::update() {
	update_basic();
	update_wears();
	update_abilities();
}

void creature::moveto(point ni) {
	area.clearpath();
	area.blockwalls();
	area.blockfeatures();
	area.makewave(getposition());
	area.blockzero();
	auto m0 = getposition();
	auto m1 = area.getnext(m0, ni);
	if(!area.isvalid(m1))
		return;
	movestep(area.getdirection(m0, m1));
}

void creature::unlink() {
	boosti::remove(this);
}

void creature::act(const char* format, ...) const {
	if(!player || player == this || area.is(player->getposition(), Visible))
		actv(console, format, xva_start(format), getname(), is(Female));
}

int creature::getmightpenalty(int enemy_strenght) const {
	auto strenght = get(Strenght);
	if(strenght < enemy_strenght)
		return enemy_strenght - strenght;
	return 0;
}

int creature::getblocking(const item& enemy_weapon, const item& weapon, int value) const {
	if(!weapon || !weapon.is(ShieldUse))
		return 0;
	value += enemy_weapon.geti().weapon.enemy_block;
	if(enemy_weapon.is(RangedWeapon))
		value += weapon.geti().weapon.block_ranged;
	else
		value += weapon.geti().weapon.block;
	return value;
}

int creature::getparrying(const item& enemy_weapon, const item& weapon, int value) const {
	if(!weapon)
		return 0;
	if(enemy_weapon.is(RangedWeapon))
		return 0;
	value += enemy_weapon.geti().weapon.enemy_parry;
	value += weapon.geti().weapon.parry;
	return value;
}