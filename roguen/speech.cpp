#include "speech.h"
#include "logparse.h"

using namespace log;

BSDATAC(speech, 1024)

void speecha::select(const char* id) {
	if(!id || id[0] == 0)
		return;
	auto ps = data;
	auto pe = endof();
	for(auto& e : bsdata<speech>()) {
		if(strcmp(e.id, id)!=0)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	count = ps - data;
}

const char* speecha::random() const {
	if(!count)
		return getnm("NothingToSay");
	return data[rand() % count]->name;
}

void store_begin(sliceu<variant>& result);
void store_end(sliceu<variant>& result);

static const char* read_line(const char* p, stringbuilder& sb, variants& elements) {
	if(*p == '{') {
		p = p + 1;
		store_begin(elements);
		while(*p && *p != '}') {
			auto pv = bsdata<variant>::add();
			pv->clear();
			p = readval(skipsp(p), sb, *pv);
		}
		if(*p == '}')
			p = skipws(p + 1);
		store_end(elements);
	}
	sb.clear();
	return sb.psstrlf(p);
}

void speech::read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	while(allowparse && *p) {
		if(!checksym(p, '#'))
			break;
		p = readidn(p + 1, sb);
		auto block_id = szdup(temp);
		if(!checksym(p, '\n'))
			break;
		p = skipwscr(p);
		while(allowparse && *p && *p != '#') {
			auto pn = bsdata<speech>::add();
			pn->clear();
			pn->id = block_id;
			p = read_line(skipwscr(p), sb, pn->condition);
			p = skipwscr(p);
			pn->name = szdup(temp);
		}
	}
	log::close();
}