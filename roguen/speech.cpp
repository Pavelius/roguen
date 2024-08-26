#include "bsreq.h"
#include "log.h"
#include "nameable.h"
#include "rand.h"
#include "speech.h"
#include "stringbuilder.h"

using namespace log;

struct speechi {
	struct element {
		const char*	name;
	};
	typedef sliceu<element> elementa;
	const char*	id;
	elementa	source;
};

BSDATAD(speechi::element)
BSDATAC(speechi, 1024)

BSMETA(speechi) = {
	BSREQ(id),
	{}};

void speech_read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	while(allowparse && *p) {
		if(!checksym(p, '#'))
			break;
		p = psidf(p + 1, sb);
		auto pr = bsdata<speechi>::add();
		pr->id = szdup(temp);
		if(!checksym(p, '\n'))
			break;
		p = skipwscr(p);
		pr->source.setbegin();
		while(allowparse && *p && *p != '#') {
			sb.clear();
			p = sb.psstrlf(skipwscr(p));
			p = skipwscr(p);
			speechi::element e = {szdup(temp)};
			bsdata<speechi::element>::source.add(&e);
		}
		pr->source.setend();
	}
	log::close();
}

const char* speech_getid(int index) {
	return bsdata<speechi>::elements[index].id;
}

const char* speech_get(int index, int random) {
	auto p = bsdata<speechi>::elements + index;
	if(random==-1)
		random = rand() % p->source.size();
	return p->source.begin()[random].name;
}

const char* speech_get(const char* id) {
	auto p = bsdata<speechi>::find(id);
	if(!p || !p->source)
		return 0;
	auto n = rand() % p->source.size();
	return p->source.begin()[n].name;
}

void speech_get(const char*& result, const char* id, const char* action, const char* middle, const char* postfix) {
	if(result)
		return;
	char temp[64]; stringbuilder sb(temp);
	sb.addv(id, 0);
	sb.addv(action, 0);
	sb.addv(middle, 0);
	sb.addv(postfix, 0);
	auto p = speech_get(temp);
	if(p)
		result = p;
}

bool parse_speech(stringbuilder& sb, const char* id) {
	auto p = speech_get(id);
	if(!p)
		return false;
	sb.addv(p, 0);
	return true;
}

void speech_initialize() {
	log::readlf(speech_read, "speech", "*.txt");
}