#include "areapiece.h"
#include "math.h"
#include "item.h"
#include "magic.h"
#include "rand.h"

item* last_item;

static_assert(sizeof(item) == 4, "Structure `item` must 4 bytes");

const char* item::getname() const {
	auto& ei = geti();
	auto id = ei.id;
	if(identified) {
		if(magic == Cursed && ei.cursed)
			id = ei.cursed->id;
	} else {
		if(ei.unidentified)
			id = ei.unidentified;
	}
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

void item::setcount(int v, const char* interactive) {
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
	last_item = this;
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

typedef adat<char, 32> powera;

static void add_powers(powera& result, const listi& source, size_t maximum_count = 0) {
	result.clear();
	for(auto& v : source.elements) {
		result.add(&v - source.elements.begin());
		if(maximum_count && result.count >= maximum_count)
			break;
	}
}

static int random_power(powera& result) {
	if(!result.count)
		return 0;
	return 1 + result.data[rand() % result.count];
}

void item::createmagical(int magical, int cursed, int artifact) {
	if(!getpower())
		return;
	if(d100() < magical) {
		if(d100() < cursed)
			set(Cursed);
		else if(d100() < artifact)
			set(Artifact);
		else
			set(Blessed);
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
		if(d100() < chance) {
			powera result;
			auto level_cap = 0;
			if(area && area->level)
				level_cap = 4 + (iabs(area->level) - 1);
			add_powers(result, *ei.powers, level_cap);
			power = random_power(result);
		}
	}
}

void item::create(const itemi* pi, int count) {
	if(!pi)
		return;
	clear();
	type = pi - bsdata<itemi>::elements;
	if(count <= 1)
		count = 1;
	if(pi->count > 1)
		count = xrand(count * 1, count * pi->count);
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

void item::damage(int bonus) {
	if(is(Natural))
		return;
	if(getmagic() == Artifact)
		return;
	if(iscountable())
		setcount(getcount() - 1); // Countable items break always
	else if(bonus >= 0) {
		// Damage item state
		if(broken >= 7)
			setcount(getcount() - 1);
		else
			broken++;
	} else {
		// Repair items
		if(broken > 0)
			broken--;
	}
}

bool item::is(feat_s v) const {
	return geti().feats.is(v);
}

void item::drop(point m) {
	for(auto& e : area->items) {
		if(e && e.position == m) {
			e.add(*this);
			if(!(*this))
				return;
		}
	}
	auto pi = area->items.add();
	pi->position = m;
	memcpy(static_cast<item*>(pi), static_cast<item*>(this), sizeof(item));
	last_item = static_cast<item*>(pi);
	clear();
}

const char*	item::getfullname(int price_percent, bool uppercase) const {
	static char temp[260];
	stringbuilder sb(temp);
	auto count = getcount();
	auto pn = getname();
	sb.adds(pn);
	if(!iscountable() && identified && power) {
		auto power = getpower();
		sb.addsep(' ');
		if(iscursed()) {
			// Cursed version
			auto tid = getnme(str("Cursed%1", power.getid()));
			if(!tid)
				tid = getnme(power.getid());
			sb.addof(tid);
		} else
			sb.addof(power.getname());
	}
	if(count > 1)
		sb.adds("%1i %-Pieces", count);
	if(price_percent) {
		auto cost = getcost() * price_percent / 100;
		sb.adds("%-Cost %1i %-Coins", cost);
	}
	sb.lower();
	if(uppercase)
		temp[0] = upper_symbol(temp[0]);
	return temp;
}

int	item::getweight() const {
	return geti().weight * getcount();
}

variants item::getuse() const {
	auto& ei = geti();
	if(iscursed() && ei.cursed)
		return ei.cursed->elements;
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

bool item::ismagical() const {
	if(iscountable())
		return false;
	return power != 0;
}

void item::use() {
	setcount(getcount() - 1);
}

int	item::geteffect() const {
	switch(magic) {
	case Cursed: return -1;
	case Blessed:
		if(identified)
			return 2;
		return 1;
	case Artifact:
		if(identified)
			return 4;
		return 1;
	default: return 1;
	}
}

int	item::chance_consume() const {
	auto chance = geti().chance_consume;
	if(!chance)
		return 0;
	switch(magic) {
	case Blessed: chance = chance * 2 / 3; break;
	case Artifact: chance = chance / 4; break;
	}
	if(chance < 3)
		chance = 3;
	return chance;
}