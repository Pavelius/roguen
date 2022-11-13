#include "io_stream.h"
#include "logparse.h"
#include "script.h"
#include "talk.h"

using namespace log;

BSDATAD(phrasei)
BSDATAD(talki)

static bool isanswer(const char* p) {
	return isnum(*p);
}

static bool isevent(const char* p) {
	return p[0] == '#' && isnum(p[1]);
}

static const char* read_string(const char* p, stringbuilder& result) {
	result.clear();
	if(p[0] == '#' || isnum(p[0]))
		return p;
	while(*p) {
		if(*p == '\n' || *p == '\r') {
			p = skipwscr(p);
			if(p[0] == '#' || isanswer(p))
				break;
			result.addch('\n');
		} else
			result.addch(*p++);
	}
	return p;
}

static void add(variants& e, variant v) {
	if(!e.count)
		e.start = bsdata<variant>::source.getcount();
	auto p = (variant*)bsdata<variant>::source.add();
	*p = v;
	e.count++;
}

static const char* read_variants(const char* p, stringbuilder& result, variants& source, phrasei* pe) {
	while(*p && allowparse) {
		p = skipws(p);
		if(!ischa(*p))
			break;
		p = readidn(p, result);
		auto pn = result.begin();
		int bonus; p = readbon(p, bonus);
		p = skipws(p);
		variant v = (const char*)result.begin();
		if(!v)
			log::error(p, "Can't find variant `%1`", result.begin());
		else
			v.counter = bonus;
		add(source, v);
	}
	return p;
}

static const char* read_params(const char* p, short& v) {
	if(!checksym(p, '('))
		return p;
	p = skipws(p + 1);
	p = stringbuilder::read(p, v);
	p = skipws(p);
	if(!checksym(p, ')'))
		return p;
	p = skipws(p + 1);
	return p;
}

static const char* read_params(const char* p, stringbuilder& result) {
	if(!checksym(p, '('))
		return p;
	p = skipws(p + 1);
	if(p[0] == '\"') {
		result.clear();
		p = result.psstr(p + 1, p[0]);
	} else
		p = readidn(p, result);
	p = skipws(p);
	if(!checksym(p, ')'))
		return p;
	p = skipws(p + 1);
	return p;
}

static const char* read_event(const char* p, short& parent, stringbuilder& sb) {
	if(!allowparse)
		return p;
	p = stringbuilder::read(skipws(p), parent);
	auto pe = bsdata<phrasei>::add(); pe->clear();
	pe->index = parent;
	pe->next = -1;
	p = read_variants(skipws(p), sb, pe->elements, pe);
	p = read_string(skipwscr(p), sb);
	pe->text = getstring(sb);
	return p;
}

static const char* read_answers(const char* p, short parent, stringbuilder& sb) {
	while(allowparse && isanswer(p)) {
		auto pe = bsdata<phrasei>::add(); pe->clear();
		pe->index = parent;
		p = stringbuilder::read(p, pe->next);
		p = read_variants(skipws(p), sb, pe->elements, pe);
		if(!checksym(p, ')'))
			break;
		p = read_string(skipws(p + 1), sb);
		pe->text = getstring(sb);
	}
	return p;
}

void talki::read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	auto ptb = bsdata<talki>::add();
	ptb->clear();
	ptb->id = szdup(szfnamewe(temp, url));
	auto n1 = bsdata<phrasei>::source.getcount();
	while(allowparse && *p) {
		if(!isevent(p)) {
			log::error(p, "Expected symbol `#` followed by event number");
			break;
		}
		short event_parent = -1; sb.clear();
		p = read_event(p + 1, event_parent, sb);
		p = read_answers(p, event_parent, sb);
	}
	log::close();
	auto n2 = bsdata<phrasei>::source.getcount();
	if(n1 < n2)
		ptb->elements.set(&bsdata<phrasei>::get(n1), n2 - n1);
}

void talki::read() {
	char temp[260]; stringbuilder sb(temp);
	sb.addlocaleurl("talk");
	for(io::file::find file(temp); file; file.next()) {
		auto p = file.name();
		if(p[0] == '.')
			continue;
		if(!szpmatch(p, "*.txt"))
			continue;
		char url[260]; file.fullname(url);
		read(url);
	}
}

phrasei* talki::find(short v) const {
	for(auto& e : elements) {
		if(e.index == v && !e.isanswer() && ifscript(e.elements))
			return &e;
	}
	return 0;
}

const phrasei* phrasei::nextanswer() const {
	auto pt = talki::owner(this);
	if(!pt)
		return 0;
	auto pb = this + 1;
	auto pe = pt->elements.end();
	while(pb < pe) {
		if(pb->index != index || !pb->isanswer())
			break;
		if(ifscript(pb->elements))
			return pb;
		pb++;
	}
	return 0;
}

talki* talki::owner(const phrasei* p) {
	for(auto& e : bsdata<talki>()) {
		if(p >= e.elements.begin() && p < e.elements.end())
			return &e;
	}
	return 0;
}