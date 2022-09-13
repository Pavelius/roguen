#include "main.h"

creature* player;

creature* creature::create(indext index, const monsteri* pv) {
	auto p = bsdata<creature>::add();
	p->setindex(index);
	p->setkind(pv);
	p->setgender(pv->gender);
	p->finish();
	return p;
}

creature* creature::create(indext index, const racei* pv, gender_s gender) {
	auto p = bsdata<creature>::add();
	p->setindex(index);
	p->setkind(pv);
	p->setgender(gender);
	p->finish();
	p->advance(p->getkind(), 0);
	return p;
}

void creature::movestep(direction_s v) {
	setdirection(v);
	movestep(to(getindex(), v));
}

void creature::movestep(indext ni) {
	if(ni == Blocked)
		return;
	setindex(ni);
	fixmovement();
}

void creature::finish() {
	hp = 10;
	fixappear();
}

void creature::aimove() {
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