#include "areamap.h"
#include "item.h"

extern areamap area;
item* last_item;

static_assert(sizeof(item) == 4, "Structure `item` must 4 bytes");

const char* item::getname() const {
	auto pid = geti().id;
	if(szstart(pid, "Cursed"))
		pid += 6;
	else if(szstart(pid, "Blessed"))
		pid += 7;
	return getnm(pid);
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
	else
		count = v - 1;
}

int item::getcount() const {
	return type ? (iscountable() ? count + 1 : 1) : 0;
}

void item::add(item& v) {
	if(type != v.type || stats != v.stats || !iscountable())
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
	if(broken >= 3)
		setcount(getcount() - 1);
	else {
		if(is(Blessed) && d100() < 50)
			return;
		broken++;
	}
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
	if(count > 1)
		sb.adds("%1i %-Pieces", count);
	sb.lower();
	if(price_percent) {
		auto cost = getcost() * price_percent / 100;
		sb.adds("%-Cost %1i %-Coins", cost);
	}
	return temp;
}

int	item::getweight() const {
	return geti().weight * getcount();
}

static const itemi* find_type(const itemi* parent, feat_s v) {
	for(auto& e : bsdata<itemi>()) {
		if(e.parent == parent && e.is(v))
			return &e;
	}
	return 0;
}