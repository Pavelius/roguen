#include "io_stream.h"
#include "log.h"
#include "script.h"
#include "talk.h"

using namespace log;

BSDATAD(phrasei)
BSDATAD(talki)

const phrasei* last_phrase;

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
		p = psidf(p, result);
		auto pn = result.begin();
		int bonus; p = psbon(p, bonus);
		p = skipws(p);
		variant v = (const char*)result.begin();
		if(!v)
			log::errorp(p, "Can't find variant `%1`", result.begin());
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
	p = psnum(p, v);
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
	result.clear();
	if(p[0] == '\"')
		p = result.psstr(p + 1, p[0]);
	else
		p = result.psidf(p);
	p = skipws(p);
	if(!checksym(p, ')'))
		return p;
	p = skipws(p + 1);
	return p;
}

static const char* read_event(const char* p, short& parent, stringbuilder& sb) {
	if(!allowparse)
		return p;
	p = psnum(skipws(p), parent);
	auto pe = bsdata<phrasei>::add(); pe->clear();
	pe->index = parent;
	pe->next = -1;
	p = read_variants(skipws(p), sb, pe->elements, pe);
	p = read_string(skipwscr(p), sb);
	pe->text = szdupz(sb);
	return p;
}

static const char* read_answers(const char* p, short parent, stringbuilder& sb) {
	while(allowparse && isanswer(p)) {
		auto pe = bsdata<phrasei>::add(); pe->clear();
		pe->index = parent;
		p = psnum(p, pe->next);
		p = read_variants(skipws(p), sb, pe->elements, pe);
		if(!checksym(p, ')'))
			break;
		p = read_string(skipws(p + 1), sb);
		pe->text = szdupz(sb);
	}
	return p;
}

phrasei* talki::find(short v) const {
	auto push_last = last_phrase;
	for(auto& e : elements) {
		if(e.index == v && !e.isanswer()) {
			if(script_allow(e.elements)) {
				last_phrase = push_last;
				return &e;
			}
		}
	}
	last_phrase = push_last;
	return 0;
}

const phrasei* phrasei::nextanswer() const {
	auto pt = find_talk(this);
	if(!pt)
		return 0;
	auto pb = this + 1;
	auto pe = pt->elements.end();
	auto push_last = last_phrase;
	while(pb < pe) {
		if(pb->index != index || !pb->isanswer())
			break;
		last_phrase = pb;
		if(script_allow(pb->elements)) {
			last_phrase = push_last;
			return pb;
		}
		pb++;
	}
	last_phrase = push_last;
	return 0;
}

talki* find_talk(const phrasei* p) {
	for(auto& e : bsdata<talki>()) {
		if(p >= e.elements.begin() && p < e.elements.end())
			return &e;
	}
	return 0;
}

void read_talk(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	auto ptb = bsdata<talki>::add();
	ptb->clear();
	ptb->id = szdup(szfnamewe(temp, url));
	ptb->elements.setbegin();
	while(allowparse && *p) {
		if(!isevent(p)) {
			log::errorp(p, "Expected symbol `#` followed by event number");
			break;
		}
		short event_parent = -1; sb.clear();
		p = read_event(p + 1, event_parent, sb);
		p = read_answers(p, event_parent, sb);
	}
	log::close();
	ptb->elements.setend();
}