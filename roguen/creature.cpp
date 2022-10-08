#include "main.h"

creature* player;
bool show_detail_info = true;

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
	if(abilities[Level] == 0) {
		while(true) {
			auto hp = xrand(1, getclass().hd);
			if(getclass().player && (hp == 1 || hp == 2))
				continue;
			basic.abilities[HitsMaximum] += hp;
			break;
		}
		basic.abilities[Level]++;
	}
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
	else {
		p->basic.abilities[WeaponSkill] += 15 + p->get(Level) * 5;
		p->basic.abilities[HeavyWeaponSkill] += 10 + p->get(Level) * 5;
		p->basic.abilities[RangedWeaponSkill] += 15 + p->get(Level) * 5;
		p->basic.abilities[ShieldUse] += 5 + p->get(Level) * 5;
	}
	p->levelup();
	p->finish();
	return p;
}

bool creature::isactive() const {
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

ability_s creature::matchparry(wear_s attack, int attacker_strenght, int value) const {
	if(value < abilities[DodgeSkill])
		return DodgeSkill;
	if(value < abilities[WeaponSkill]
		&& (attack == MeleeWeapon || attack == MeleeWeaponOffhand)
		&& wears[MeleeWeapon]
		&& (attacker_strenght - get(Brawl)) <= 20)
		return WeaponSkill;
	if(value < abilities[ShieldUse]
		&& wears[MeleeWeaponOffhand]
		&& wears[MeleeWeaponOffhand].is(ShieldUse))
		return ShieldUse;
	return (ability_s)0;
}

void creature::attack(creature& enemy, wear_s v, int bonus, int damage_multiplier) {
	last_hit = d100(); last_hit_result = 0;
	last_parry = d100(); last_parry_result = 0;
	bonus += get(wears[v].geti().ability);
	last_hit_result = (bonus - last_hit) / 10;
	if(last_hit > 5 && last_hit > bonus) {
		if(show_detail_info)
			act(getnm("AttackMiss"), last_hit, bonus);
		return;
	}
	auto parry = enemy.matchparry(v, get(Brawl), last_parry);
	if(parry) {
		last_parry_result = (get(parry) - last_parry) / 10;
		if(last_parry_result > last_hit_result) {
			if(show_detail_info)
				act(getnm("AttackHitButParry"), last_hit, bonus, last_parry, enemy.get(parry), enemy.getname(), getnm(bsdata<abilityi>::elements[parry].id));
			return;
		}
	}
	if(show_detail_info) {
		if(parry)
			act(getnm("AttackHitButParry"), last_hit, bonus, last_parry, enemy.get(parry), enemy.getname(), getnm(bsdata<abilityi>::elements[parry].id));
		else
			act(getnm("AttackHit"), last_hit, bonus);
	}
	auto result_damage = getdamage(v) + last_hit_result + last_parry_result;
	if(result_damage > 0) {
		result_damage -= get(DamageReduciton);
		result_damage = result_damage * damage_multiplier / 100;
		enemy.damage(result_damage);
	}
}

void creature::damage(int v) {
	if(v < 0) {
		if(show_detail_info)
			act(getnm("ArmorNegateDamage"));
		return;
	}
	fixvalue(-v);
	abilities[Hits] -= v;
	if(abilities[Hits] <= 0) {
		if(show_detail_info)
			act(getnm("ApplyKill"), v);
		fixeffect("HitVisual");
		fixremove();
		clear();
	} else {
		if(show_detail_info)
			acts(getnm("ApplyDamage"), v);
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
		if(!roll(Brawl)) {
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
	auto result = 1 + (rand() % 20);
	if(result == 1)
		return true;
	else if(result == 20)
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
		if(roll(Brawl))
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
	auto& ci = getclass();
	// Apply skills value
	for(auto i = (ability_s)0; i < Hits; i = (ability_s)(i + 1)) {
		auto n = bsdata<abilityi>::elements[i].basic;
		if(n)
			abilities[i] += abilities[n] / 2;
	}
	abilities[DamageMelee] += abilities[Brawl] / 10;
	abilities[DamageThrown] += abilities[Brawl] / 10;
	abilities[Speed] += 20;
	auto level = abilities[Level];
	if(level > ci.cap)
		level = ci.cap;
	abilities[HitsMaximum] += getbonus(Brawl) * level;
	if(abilities[HitsMaximum] < level)
		abilities[HitsMaximum] = level;
	abilities[ManaMaximum] += abilities[Wits];
	if(abilities[ManaMaximum] < 0)
		abilities[ManaMaximum] = 0;
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
	if(!player || player==this || area.is(player->getposition(), Visible))
		actv(console, format, xva_start(format), getname(), is(Female));
}

void creature::acts(const char* format, ...) const {
	if(!player || player == this || area.is(player->getposition(), Visible))
		actv(console, format, xva_start(format), getname(), is(Female), ' ');
}