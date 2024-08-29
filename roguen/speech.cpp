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

int speech_random;

static int rand_number() {
	speech_random = speech_random * 1103515245 + 12345;
	return (unsigned int)(speech_random / 65536) % 32768;
}

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

const char* speech_get_id(int index) {
	return bsdata<speechi>::elements[index].id;
}

const char* speech_get(int index) {
	auto p = bsdata<speechi>::elements + index;
	auto n = p->source.size();
	if(!n)
		return 0;
	return p->source.begin()[rand_number() % n].name;
}

const char* speech_get(const char* id) {
	auto p = bsdata<speechi>::find(id);
	if(!p || !p->source)
		return 0;
	if(!speech_random)
		speech_random = rand();
	auto n = rand_number() % p->source.size();
	return p->source.begin()[n].name;
}

void speech_get(const char*& result, const char* id, const char* action, const char* middle) {
	if(!result) {
		char temp[64]; stringbuilder sb(temp);
		sb.addv(id, 0);
		sb.addv(action, 0);
		sb.addv(middle, 0);
		result = speech_get(temp);
	}
}

bool parse_speech(stringbuilder& sb, const char* id) {
	auto p = speech_get(id);
	if(!p)
		return false;
	sb.addv(p, 0);
	return true;
}