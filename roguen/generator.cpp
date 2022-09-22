#include "bsreq.h"
#include "generator.h"
#include "list.h"
#include "logparse.h"

using namespace log;

BSMETA(generatori) = {
	BSREQ(id),
	BSREQ(chance),
	BSREQ(value),
	{}};
BSDATAC(generatori, 256)

typedef slice<generatori> generatora;

void generatori::clear() {
	memset(this, 0, sizeof(*this));
}

static generatora find_elements(const char* id) {
	generatori* p1 = 0;
	generatori* p2 = 0;
	for(auto& e : bsdata<generatori>()) {
		if(!p1) {
			if(equal(e.id, id)) {
				p1 = &e;
				p2 = &e;
			}
		} else if(!equal(e.id, id))
			break;
		else
			p2 = &e;
	}
	return generatora(p1, p2 + 1);
}

static int total_chance(const generatora& elements) {
	auto result = 0;
	for(auto& e : elements)
		result += e.chance;
	return result;
}

static variant random_value_raw(const char* id) {
	auto elements = find_elements(id);
	auto total = total_chance(elements);
	if(total) {
		auto result = rand() % total;
		for(auto& e : elements) {
			if(result < e.chance)
				return e.value;
			result -= e.chance;
		}
	}
	return variant();
}

variant random_list_value(variant value) {
	if(value.iskind<listi>()) {
		auto pi = bsdata<listi>::elements + value.value;
		if(!pi->elements)
			return variant();
		return random_list_value(pi->elements.begin()[(rand() % pi->elements.count)]);
	}
	return value;
}

variant random_value(const char* id) {
	return random_list_value(random_value_raw(id));
}

static const char* readid(const char* p, const char*& result) {
	result = 0;
	char temp[260]; stringbuilder sb(temp);
	p = sb.psidf(p);
	result = szdup(temp);
	return skipws(p);
}

static const char* readval(const char* p, char& value) {
	int v;
	p = stringbuilder::read(p, v);
	value = v;
	return skipws(p);
}

static const char* readval(const char* p, variant& value) {
	char temp[260]; stringbuilder sb(temp);
	p = readval(p, sb, value);
	return skipws(p);
}

static const char* read_table(const char* p, const char* id) {
	while(p && p[0] && p[0] != '#') {
		auto pn = bsdata<generatori>::add();
		pn->clear();
		pn->id = id;
		p = readval(p, pn->chance);
		p = readval(p, pn->value);
		p = skipspcr(p);
	}
	return p;
}

void generatori::read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	allowparse = true;
	while(allowparse && *p) {
		if(!checksym(p, '#'))
			break;
		const char* id;
		p = readid(p + 1, id);
		if(!checksym(p, '\n'))
			break;
		p = skipspcr(p);
		p = read_table(p, id);
	}
	log::close();
}