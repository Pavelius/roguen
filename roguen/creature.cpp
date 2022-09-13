#include "main.h"

creature* player;

creature* creature::create(indext index, variant kind) {
	if(!kind)
		return 0;
	auto p = bsdata<creature>::add();
	p->setindex(index);
	p->setkind(kind);
	monsteri* pm = kind;
	if(pm) {
		p->setgender(pm->gender);
		memcpy(&p->basic, static_cast<statable*>(pm), sizeof(p->basic));
	}
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i+1))
		p->basic.abilities[i] = 8 + rand() % 5;
	p->finish();
	p->advance(kind, 0);
	return p;
}

bool creature::isactive() const {
	return this == player;
}

void creature::movestep(direction_s v) {
	setdirection(v);
	movestep(to(getindex(), v));
}

void creature::movestep(indext ni) {
	if(ni == Blocked)
		return;
	auto index = getindex();
	if(area.is(index, Webbed)) {
		wait(2);
		if(!roll(Strenght)) {
			act(getnm("WebEntagled"));
			return;
		}
		area.remove(index, Webbed);
	}
	setindex(ni);
	fixmovement();
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

bool creature::roll(ability_s v) const {
	auto value = get(v);
	auto result = 1 + (rand() % 20);
	return result <= value;
}

void adventure_mode();

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
	//creaturea creatures;
	//creatures.select(getposition(), getlos());
	//creaturea enemies = creatures;
	//enemies.match(*this, Hostile, false);
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

void creature::update() {
	memcpy(abilities, basic.abilities, Hits * sizeof(abilities[0]));
	statable::update();
}