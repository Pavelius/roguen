#include "areamap.h"
#include "item.h"

extern areamap area;
item* last_item;

static_assert(sizeof(item) == 4, "Structure `item` must 4 bytes");

static int random_count(const itemvariety* p) {
	auto pe = p->elements + sizeof(itemvariety::elements) / sizeof(itemvariety::elements[0]);
	auto count = 0;
	for(auto pb = p->elements; pb < pe && *pb; pb++)
		count++;
	return count;
}

static int find_upgrade(const itemvariety* p, const itemstat* upgrade) {
	if(!upgrade)
		return 0;
	auto pe = p->elements + sizeof(itemvariety::elements) / sizeof(itemvariety::elements[0]);
	auto count = 0;
	for(auto pb = p->elements; pb < pe && *pb; pb++) {
		if(*pb == upgrade)
			return count + 1;
		count++;
	}
	return 0;
}

static int random_upgrade(const itemvariety* p, int level) {
	if(!p)
		return 0;
	auto count = random_count(p);
	if(!count)
		return 0;
	return 1 + rand() % count;
}

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
	if(broken >= 3) {
		if(is(Blessed))
			return;
		setcount(getcount() - 1);
	} else
		broken++;
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
	if(isidentified()) {
		auto p = getprefix();
		if(p) {
			sb.addsep(' ');
			sb.adjective(getnm(p->id), vw);
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

const itemstat* item::getprefix() const {
	if(!prefix || iscountable())
		return 0;
	return geti().prefix->elements[prefix - 1];
}

const itemstat* item::getsuffix() const {
	if(!suffix || iscountable())
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

void item::setupgrade(const itemstat* pv) {
	if(!pv)
		return;
	switch(pv->upgrade) {
	case 1: prefix = find_upgrade(geti().prefix, pv); break;
	default: suffix = find_upgrade(geti().suffix, pv); break;
	}
}

void item::upgrade(int chance_prefix, int chance_suffix, int level) {
	if(iscountable())
		return;
	if(d100() < chance_prefix)
		prefix = random_upgrade(geti().prefix, level);
	if(d100() < chance_suffix)
		suffix = random_upgrade(geti().suffix, level);
}

static const itemi* find_type(const itemi* parent, feat_s v) {
	for(auto& e : bsdata<itemi>()) {
		if(e.parent == parent && e.is(v))
			return &e;
	}
	return 0;
}

void item::upgrade(feat_s v) {
	auto nt = find_type(&geti(), v);
	if(!nt)
		return;
	type = nt - bsdata<itemi>::elements;
}