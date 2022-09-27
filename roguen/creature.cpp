#include "main.h"

creature* player;

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static creature* findalive(indext index) {
	for(auto& e : bsdata<creature>()) {
		if(!e)
			continue;
		if(e.getindex() == index)
			return &e;
	}
	return 0;
}

static ability_s attack_ability(wear_s v) {
	switch(v) {
	case RangedWeapon: return ToHitRanged;
	case ThrownWeapon: return ToHitThrown;
	default: return ToHitMelee;
	}
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

creature* creature::create(indext index, variant kind) {
	if(!kind)
		return 0;
	auto p = bsdata<creature>::add();
	p->setindex(index);
	p->setkind(kind);
	monsteri* pm = kind;
	if(pm) {
		copy(p->basic, *pm);
		p->feats = pm->feats;
	}
	p->basic.create();
	p->advance(kind, 0);
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
	if(area.is(getindex(), Iced)) {
		if(!roll(Dexterity)) {
			act(getnm("IcedSlice"));
			v = round(v, (d100() < 50) ? West : East);
			wait();
		}
	}
	setdirection(v);
	movestep(to(getindex(), v));
}

static void drop_item(indext index, const char* id) {
	if(index == Blocked || !id)
		return;
	itemi* pi = random_value(id);
	if(!pi)
		return;
	item it; it.create(pi);
	it.drop(index);
}

void creature::interaction(indext index) {
	if(index == Blocked)
		return;
	auto f = area.features[index];
	if(f) {
		auto& ei = bsdata<featurei>::elements[f];
		if(ei.is(AllowActivate)) {
			if(!area.is(index, Activated))
				area.set(index, Activated);
			else
				area.remove(index, Activated);
		} else if(ei.is(Woods) && wears[MeleeWeapon].geti().is(CutWoods)) {
			auto& wei = wears[MeleeWeapon].geti();
			int chance = 1;
			if(wei.is(TwoHanded))
				chance = wei.weapon.damage.max;
			else
				chance = wei.weapon.damage.max / 2;
			if(d100() < chance) {
				act(getnm("YouCutWood"), getnm(ei.id));
				area.features[index] = NoFeature;
				drop_item(index, "WoodenLagsTable");
			}
		}
	}
}

void creature::interaction(creature& opponent) {
	if(opponent.isenemy(*this))
		attackmelee(opponent);
}

dice creature::getdamage(wear_s v) const {
	dice result = wears[v].getdamage();
	result += get(damage_ability(v));
	result.correct();
	return result;
}

void creature::attack(creature& enemy, wear_s v, int bonus, int damage_multiplier) {
	bonus += get(attack_ability(v));
	bonus -= enemy.get(ParryValue);
	auto result = rand() % 20 + 1;
	if(result == 1 || (result != 20 && result < (10 - bonus))) {
		act(getnm("AttackMiss"));
		return;
	}
	auto result_damage = getdamage(v).roll();
	result_damage = result_damage * damage_multiplier / 100;
	enemy.damage(result_damage);
}

void creature::damage(int v) {
	fixvalue(-v);
	abilities[Hits] -= v;
	if(abilities[Hits] <= 0) {
		act(getnm("ApplyKill"), v);
		fixeffect("HitVisual");
		fixremove();
		clear();
	} else
		act(getnm("ApplyDamage"), v);
}

void creature::attackmelee(creature& enemy) {
	fixaction();
	attack(enemy, MeleeWeapon, 0, 100);
}

void creature::fixcantgo() const {
	fixaction();
	act(getnm("CantGoThisWay"));
}

void creature::movestep(indext ni) {
	if(ni == Blocked) {
		if(isactive())
			fixcantgo();
		wait();
		return;
	}
	auto index = getindex();
	if(area.is(index, Webbed)) {
		wait(2);
		if(!roll(Strenght, -2)) {
			act(getnm("WebEntagled"));
			wait();
			fixaction();
			return;
		}
		act(getnm("WebBreak"));
		area.remove(index, Webbed);
	}
	auto opponent = findalive(ni);
	if(opponent)
		interaction(*opponent);
	else if(area.isfree(ni)) {
		setindex(ni);
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
	creatures.select(getindex(), getlos());
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
		if(roll(Constitution))
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
		enemies.sort(getindex());
		enemy = enemies[0];
	}
	if(isactive()) {
		area.setlos(getindex(), getlos());
		adventure_mode();
	} else if(enemy)
		moveto(enemy->getindex());
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
	abilities[Speed] += 20;
	auto level = abilities[Level];
	if(level > ci.cap)
		level = ci.cap;
	abilities[HitsMaximum] += getbonus(Constitution) * level;
	if(abilities[HitsMaximum] < level)
		abilities[HitsMaximum] = level;
	abilities[ManaMaximum] += abilities[Intellect] + abilities[Concentration] * level;
	if(abilities[ManaMaximum] < 0)
		abilities[ManaMaximum] = 0;
}

void creature::update() {
	update_basic();
	update_wears();
	update_abilities();
}

void creature::moveto(indext ni) {
	area.clearpath();
	area.blockwalls();
	area.blockfeatures();
	area.makewave(getindex());
	area.blockzero();
	auto i0 = getindex();
	auto i1 = area.getnext(i0, ni);
	if(i1 == Blocked)
		return;
	movestep(area.getdirection(i2m(i0), i2m(i1)));
}

void creature::unlink() {
	boosti::remove(this);
}