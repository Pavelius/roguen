#include "areamap.h"
#include "item.h"

extern areamap area;
item* last_item;

static_assert(sizeof(item) == 4, "Invalid size of `item` structure");

static int d100() {
	return rand() % 100;
}

static int random_count(const itemvariety* p) {
	auto pe = p->elements + sizeof(itemvariety::elements) / sizeof(itemvariety::elements[0]);
	auto count = 0;
	for(auto pb = p->elements; pb < pe && *pb; pb++)
		count++;
	return count;
}

static int random_upgrade(const itemvariety* p, int level) {
	if(!p)
		return 0;
	auto count = random_count(p);
	if(!count)
		return 0;
	return 1 + rand() % count;
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
	return type ? (iscountable() ? count + 1 : 1) : 0;
}

void item::add(item& v) {
	if(type != v.type)
		return;
	if(iscountable()) {
		unsigned n1 = count + v.count + 1;
		if(n1 >= 0xFFFF) {
			count = 0xFFFF;
			v.count = n1 - count - 1;
		} else {
			count = n1;
			v.clear();
		}
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

bool item::is(feat_s v) const {
	if(geti().feats.is(v))
		return true;
	if(!iscountable()) {
		auto p = getprefix();
		if(p && p->feats.is(v))
			return true;
		p = getsuffix();
		if(p && p->feats.is(v))
			return true;
	}
	return false;
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
	if(!iscountable()) {
		auto magic = getmagic();
		if(isidentified()) {
			sb.addsep(' ');
			sb.adjective(getnm(bsdata<magici>::elements[magic].id), vw);
			auto p = getprefix();
			if(p) {
				sb.addsep(' ');
				sb.adjective(getnm(p->id), vw);
			}
		}
	}
	sb.adds("%1", getname());
	if(isidentified()) {
		auto sf = getsuffix();
		if(sf) {
			sb.addsep(' ');
			sb.addof(getnm(sf->id));
		}
	}
	if(count > 1)
		sb.adds("%1i %-Pieces", count);
	sb.lower();
	if(price_percent) {
		auto cost = getcost() * price_percent / 100;
		sb.adds("%-Cost %1i %-Coins", cost);
	}
	return temp;
}

void item::set(magic_s v) {
	if(iscountable())
		return;
	magic = v;
}

const itemstat* item::getprefix() const {
	if(iscountable() || !prefix)
		return 0;
	return geti().prefix->elements[prefix - 1];
}

const itemstat* item::getsuffix() const {
	if(iscountable() || !suffix)
		return 0;
	return geti().suffix->elements[suffix - 1];
}

int	item::getweight() const {
	auto& ei = geti();
	auto result = ei.weight;
	if(!ei.iscountable()) {
		auto p = getprefix();
		if(p && p->weight)
			result = result * p->weight / 100;
		p = getsuffix();
		if(p && p->weight)
			result = result * p->weight / 100;
	}
	return result * getcount();
}

int	item::get(unsigned fo) const {
	auto& ei = geti();
	int result = *((char*)&ei + fo);
	if(!ei.iscountable()) {
		auto prefix = getprefix();
		if(prefix)
			result += *((char*)prefix + fo);
		auto suffix = getsuffix();
		if(suffix)
			result += *((char*)suffix + fo);
	}
	return result;
}

void item::upgrade(int chance_prefix, int chance_suffix, int level) {
	if(iscountable())
		return;
	if(d100() < chance_prefix)
		prefix = random_upgrade(geti().prefix, level);
	if(d100() < chance_suffix)
		suffix = random_upgrade(geti().suffix, level);
}