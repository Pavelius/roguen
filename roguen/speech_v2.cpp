#include "bsreq.h"
#include "crt.h"
#include "logparse.h"
#include "nameable.h"

using namespace log;

struct speechv2 {
	struct element {
		const char*	name;
	};
	typedef sliceu<element> elementa;
	const char*	id;
	elementa	source;
};

BSDATAD(speechv2::element)
BSDATAC(speechv2, 1024)

BSMETA(speechv2) = {
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
		p = readidn(p + 1, sb);
		auto pr = bsdata<speechv2>::add();
		pr->id = szdup(temp);
		if(!checksym(p, '\n'))
			break;
		p = skipwscr(p);
		auto psb = bsdata<speechv2::element>::source.count;
		while(allowparse && *p && *p != '#') {
			p = sb.psstrlf(skipwscr(p));
			p = skipwscr(p);
			speechv2::element e = {szdup(temp)};
			bsdata<speechv2::element>::source.add(&e);
		}
		if(psb != bsdata<speechv2::element>::source.count)
			pr->source.set((speechv2::element*)bsdata<speechv2::element>::source.ptr(psb), bsdata<speechv2::element>::source.count - psb);
	}
	log::close();
}

const char* speech_get(const char* id) {
	auto p = bsdata<speechv2>::find(id);
	if(!p || !p->source)
		return 0;
	auto n = rand() % p->source.size();
	return p->source.begin()[n].name;
}

void speech_initialize() {
	readl("Chats", speech_read);
}