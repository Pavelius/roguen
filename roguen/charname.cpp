#include "charname.h"
#include "logparse.h"

using namespace log;

BSDATAD(charname)

bool charname::match(variant v) const {
	for(auto e : conditions) {
		if(e == v)
			return true;
	}
	return false;
}

bool charname::match(const slice<variant>& source) const {
	for(auto e : source) {
		if(!e)
			break;
		if(!match(e))
			return false;
	}
	return true;
}

const char* charname::getname(unsigned short v) {
	if(v == 0xFFFF)
		return "No name";
	return ((charname*)bsdata<charname>::source.ptr(v))->name;
}

unsigned charname::select(short unsigned* pb, short unsigned* pe, const slice<variant>& source) {
	auto ps = pb;
	for(auto& e : bsdata<charname>()) {
		if(!e.match(source))
			continue;
		if(ps < pe)
			*ps++ = bsdata<charname>::source.indexof(&e);
	}
	return ps - pb;
}

short unsigned charname::random(const slice<variant>& source) {
	short unsigned temp[512];
	auto count = select(temp, temp + sizeof(temp) / sizeof(temp[0]), source);
	if(count)
		return temp[rand() % count];
	return 0xFFFF;
}

static bool ischax(unsigned char u) {
	return (u >= 'A' && u <= 'Z')
		|| (u >= 'a' && u <= 'z')
		|| (u >= 0xC0 && u <= 0xFF)
		|| (u >= 0x410 && u <= 0x44F);
}

static const char* read_line(const char* p, variant* conditions, stringbuilder& sb) {
	while(ischax(*p)) {
		auto pe = bsdata<charname>::add();
		memset(pe, 0, sizeof(*pe));
		memcpy(pe->conditions, conditions, sizeof(pe->conditions));
		p = readname(skipws(p), sb);
		pe->name = getstring(sb);
		p = skipws(p);
		if(*p == 13 || *p == 10 || *p == 0)
			break;
		if(!checksym(p, ','))
			break;
		p = skipwscr(p + 1);
	}
	return p;
}

static const char* read_conditions(const char* p, stringbuilder& sb, variant* pb, const variant* pe) {
	auto count = pe - pb;
	while(ischa(p[0])) {
		p = readidn(p, sb);
		variant v = (const char*)sb.begin();
		if(!v)
			log::error(p, "Can't find variant `%1`", sb.begin());
		else if(pb >= pe)
			log::error(p, "Too many conditions when save variant %1 (only %2i allowed)", v.getid(), count);
		else
			*pb++ = v;
		p = skipws(p);
	}
	return p;
}

void charname::read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	while(allowparse && *p) {
		if(!checksym(p, '#'))
			break;
		variant conditions[4] = {};
		p = read_conditions(skipws(p + 1), sb, conditions, conditions + sizeof(conditions) / sizeof(conditions[0]));
		if(!checksym(p, '\n'))
			break;
		p = read_line(skipwscr(p), conditions, sb);
		p = skipwscr(p);
	}
	log::close();
}