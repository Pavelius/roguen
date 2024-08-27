#include "bsreq.h"
#include "charname.h"
#include "log.h"
#include "rand.h"
#include "stringbuilder.h"

using namespace log;

struct charname {
	const char* id;
	const char*	name;
};
BSDATAD(charname)
BSMETA(charname) = {
	BSREQ(id),
	BSREQ(name),
	{}};

const char* get_charname(unsigned short v) {
	if(v == 0xFFFF)
		return "No name";
	return ((charname*)bsdata<charname>::source.ptr(v))->name;
}

short unsigned select_charname(short unsigned* pb, short unsigned* pe, const char* pattern) {
	auto ps = pb;
	for(auto& e : bsdata<charname>()) {
		if(!szpmatch(e.id, pattern))
			continue;
		if(ps < pe)
			*ps++ = bsdata<charname>::source.indexof(&e);
	}
	return ps - pb;
}

short unsigned random_charname(const char* pattern) {
	short unsigned temp[512];
	auto count = select_charname(temp, temp + sizeof(temp) / sizeof(temp[0]), pattern);
	if(count)
		return temp[rand() % count];
	return 0xFFFF;
}

static const char* read_line(const char* p, const char* id, stringbuilder& sb) {
	while(ischa(*p)) {
		auto pe = bsdata<charname>::add();
		pe->id = id;
		sb.clear(); p = sb.psparam(skipws(p));
		pe->name = szdupz(sb);
		p = skipws(p);
		if(*p == 13 || *p == 10 || *p == 0)
			break;
		if(!checksym(p, ','))
			break;
		p = skipwscr(p + 1);
	}
	return p;
}

void read_charname(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	while(allowparse && *p) {
		if(!checksym(p, '#'))
			break;
		auto pn = p;
		p = psidf(p + 1, sb);
		if(temp[0] == 0)
			errorp(pn, "Expected identifier");
		auto id = szdup(temp);
		if(!checksym(p, '\n'))
			break;
		p = read_line(skipwscr(p), id, sb);
		p = skipwscr(p);
	}
	log::close();
}