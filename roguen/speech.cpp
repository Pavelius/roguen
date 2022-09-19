#include "speech.h"
#include "logparse.h"

using namespace log;

BSDATAC(speech, 1024)

void speecha::select(const char* id) {
	auto ps = data;
	auto pe = endof();
	id = szdup(id);
	for(auto& e : bsdata<speech>()) {
		if(e.id != id)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	count = ps - data;
}

const char* speecha::getrandom() const {
	if(!count)
		return getnm("NoRandomSpeech");
	return data[rand() % count]->name;
}

static const char* read_line(const char* p, stringbuilder& sb) {
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
		p = readidn(p+1, sb);
		auto block_id = szdup(temp);
		if(!checksym(p, '\n'))
			break;
		p = skipwscr(p);
		while(allowparse && *p && *p != '#') {
			auto pn = bsdata<speech>::add();
			pn->clear();
			pn->id = block_id;
			p = read_line(skipwscr(p), sb);
			p = skipwscr(p);
			pn->name = szdup(temp);
		}
	}
	log::close();
}