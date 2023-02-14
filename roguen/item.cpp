#include "areamap.h"
#include "item.h"

extern areamap area;
item* last_item;

static_assert(sizeof(item) == 4, "Structure `item` must 4 bytes");

const char* item::getname() const {
	auto& ei = geti();
	auto id = identified ? ei.id : ei.no_identifier;
	if(!id)
		id = ei.id;
	return getnm(id);
}

int item::getcost() const {
	return geti().cost;
}

int item::getcostall() const {
	auto& ei = geti();
	auto c = getcount();
	if(ei.count > 1) {
		c = c / ei.count;
		if(c <= 0)
			c = 1;
	}
	return ei.cost * c;
}

void item::setcount(int v) {
	if(v <= 0)
		clear();
	else if(iscountable())
		count = v - 1;
}

int item::getcount() const {
	return iscountable() ? 1 + count : 1;
}

void item::add(item& v) {
	if(type != v.type || stats != v.stats)
		return;
	if(!iscountable())
		return;
	unsigned n1 = count + v.count + 1;
	if(n1 >= 0xFF) {
		count = 0xFF;
		v.count = n1 - count - 1;
	} else {
		count = n1;
		v.clear();
	}
}

bool item::canequip(wear_s v) const {
	auto w = geti().wear;
	switch(w) {
	case FingerRight: case FingerLeft:
		return v == FingerLeft || v == FingerRight;
	default:
		return v == w;
	}
}

void item::createpower(int chance_power) {
	if(iscountable())
		return;
	auto& ei = geti();
	if(ei.powers && ei.powers->elements.count > 0) {
		auto chance = ei.chance_power;
		if(!chance)
			chance = 10;
		chance += chance_power;
		if(d100() < chance)
			power = 1 + (rand() % ei.powers->elements.count);
	}
}

void item::create(const itemi* pi, int count) {
	if(!pi)
		return;
	clear();
	type = pi - bsdata<itemi>::elements;
	if(pi->count)
		setcount(count * pi->count);
	else
		setcount(count);
}

bool item::is(wear_s v) const {
	auto wear = geti().wear;
	switch(v) {
	case FingerLeft: case FingerRight:
		return (wear == FingerRight);
	default:
		return wear == v;
	}
}

void item::damage() {
	if(is(Natural))
		return;
	else if(is(Blessed) && d100() < 60)
		return;
	else if(iscountable() || broken >= 7)
		setcount(getcount() - 1);
	else
		broken++;
}

bool item::is(feat_s v) const {
	return geti().feats.is(v);
}

void item::drop(point m) {
	if(!area.isvalid(m))
		return;
	for(auto& e : bsdata<itemground>()) {
		if(e && e.position == m) {
			e.add(*this);
			if(!(*this))
				return;
		}
	}
	auto pi = bsdata<itemground>::add();
	memcpy(static_cast<item*>(pi), static_cast<item*>(this), sizeof(item));
	pi->position = m;
	clear();
}

const char*	item::getfullname(int price_percent) const {
	static char temp[260];
	stringbuilder sb(temp);
	auto count = getcount();
	auto pn = getname();
	auto vw = stringbuilder::getgender(pn);
	sb.adds("%1", getname());
	if(!iscountable() && identified && power) {
		auto power = getpower();
		sb.addsep(' ');
		sb.addof(power.getname());
		sb.add("%+1i", power.counter);
	}
	if(count > 1)
		sb.adds("%1i %-Pieces", count);
	sb.lower();
	temp[0] = stringbuilder::upper(temp[0]);
	if(price_percent) {
		auto cost = getcost() * price_percent / 100;
		sb.adds("%-Cost %1i %-Coins", cost);
	}
	return temp;
}

int	item::getweight() const {
	return geti().weight * getcount();
}

variants item::getuse() const {
	auto& ei = geti();
	auto result = ei.use;
	if(is(Blessed) && ei.use_blessed)
		result = ei.use_blessed;
	if(is(Cursed) && ei.use_cursed)
		result = ei.use_cursed;
	return result;
}

variant	item::getpower() const {
	if(!power)
		return variant();
	auto& ei = geti();
	if(!ei.powers)
		return variant();
	auto p = ei.powers;
	if(power > p->elements.count)
		return variant();
	return p->elements.begin()[power - 1];
}