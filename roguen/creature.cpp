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

creature* creature::create(indext index, variant kind) {
	if(!kind)
		return 0;
	auto p = bsdata<creature>::add();
	p->setindex(index);
	p->setkind(kind);
	monsteri* pm = kind;
	if(pm)
		copy(p->basic, *pm);
	p->basic.create();
	p->finish();
	p->advance(kind, 0);
	return p;
}

bool creature::isactive() const {
	return this == player;
}

bool creature::isenemy(const creature& opponent) const {
	return opponent.is(is(Enemy) ? Ally : Enemy);
}

void creature::movestep(direction_s v) {
	setdirection(v);
	movestep(to(getindex(), v));
}

void creature::interaction(indext index) {
}

void creature::interaction(creature& opponent) {
	if(opponent.isenemy(*this))
		attackmelee(opponent);
}

void creature::attackmelee(creature& enemy) {
	fixaction();
}

void creature::movestep(indext ni) {
	if(ni == Blocked) {
		if(isactive()) {
			fixaction();
			act(getnm("CantGoThisWay"));
		}
		wait();
		return;
	}
	auto index = getindex();
	if(area.is(index, Webbed)) {
		wait(2);
		if(!roll(Strenght, -2)) {
			act(getnm("WebEntagled"));
			wait();
			return;
		}
		act(getnm("WebBreak"));
		area.remove(index, Webbed);
	}
	if(area.is(index, Iced)) {
		wait(2);
		if(!roll(Strenght, -2)) {
			act(getnm("WebEntagled"));
			wait();
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
	enemies = creatures;
	//enemies.match(*this, Hostile, false);
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
	//creature* enemy = 0;
	//if(enemies) {
	//	// Combat situation - need eliminate enemy
	//	enemies.sort(getposition());
	//	enemy = enemies[0];
	//}
	if(isactive())
		adventure_mode();
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
	feats = basic.feats;
}

void creature::update_abilities() {
	abilities[Speed] += 20;
	abilities[HitsMaximum] += 10;
}

void creature::update() {
	update_basic();
	update_wears();
	update_abilities();
}