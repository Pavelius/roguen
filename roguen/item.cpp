#include "areamap.h"
#include "item.h"
#include "magic.h"

extern areamap area;
item* last_item;

static_assert(sizeof(item) == 4, "Structure `item` must 4 bytes");

const char* item::getname() const {
	auto& ei = geti();
	auto id = identified ? ei.id : ei.unidentified;
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

typedef adat<char> powera;

static void add_powers(powera& result, const listi& source, bool noncursed, bool cursed) {
	result.clear();
	for(auto& v : source.elements) {
		if(v.counter < 0) {
			if(!cursed)
				continue;
		} else {
			if(!noncursed)
				continue;
		}
		result.add(&v - source.elements.begin());
	}
}

static int random_power(powera& result) {
	if(!result.count)
		return 0;
	return 1 + result.data[rand() % result.count];
}

void item::createpower(int chance_power, int chance_cursed) {
	if(iscountable())
		return;
	auto& ei = geti();
	if(ei.powers && ei.powers->elements.count > 0) {
		auto chance = ei.chance_power;
		if(!chance)
			chance = 10;
		chance += chance_power;
		if(d100() < chance) {
			powera result;
			if(d100() < chance_cursed)
				add_powers(result, *ei.powers, false, true);
			else
				add_powers(result, *ei.powers, true, false);
			power = random_power(result);
		}
	}
}

void item::create(const itemi* pi, int count) {
	if(!pi)
		return;
	clear();
	type = pi - bsdata<itemi>::elements;
	if(count)
		setcount(count);
	else if(pi->count > 1)
		setcount(xrand(1, pi->count));
	else
		setcount(1);
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

const char*	item::getfullname(int price_percent, bool uppercase) const {
	static char temp[260];
	stringbuilder sb(temp);
	auto count = getcount();
	auto pn = getname();
	auto vw = stringbuilder::getgender(pn);
	sb.adds(getname());
	if(!iscountable() && identified && power) {
		auto power = getpower();
		sb.addsep(' ');
		if(power.counter < 0) {
			// Cursed version
			auto tid = getnme(str("Cursed%1", power.getid()));
			if(!tid)
				tid = getnme(power.getid());
			sb.addof(tid);
		} else
			sb.addof(power.getname());
		sb.add("%+1i", power.counter);
	}
	if(count > 1)
		sb.adds("%1i %-Pieces", count);
	if(price_percent) {
		auto cost = getcost() * price_percent / 100;
		sb.adds("%-Cost %1i %-Coins", cost);
	}
	sb.lower();
	if(uppercase)
		temp[0] = stringbuilder::upper(temp[0]);
	return temp;
}

int	item::getweight() const {
	return geti().weight * getcount();
}

variants item::getuse() const {
	return geti().use;
}

variant	item::getpower() const {
	if(iscountable() || !power)
		return variant();
	auto& ei = geti();
	if(!ei.powers)
		return variant();
	auto p = ei.powers;
	if(power > p->elements.count)
		return variant();
	return p->elements.begin()[power - 1];
}

bool item::iscursed() const {
	auto v = getpower();
	if(v && v.counter < 0)
		return true;
	return false;
}

bool item::ismagical() const {
	if(iscountable())
		return false;
	return power != 0;
}