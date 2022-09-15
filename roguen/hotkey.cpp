#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "hotkey.h"
#include "log.h"
#include "script.h"
#include "stringbuilder.h"

BSDATAC(hotkey, 256)
BSDATAC(hotkeylist, 32)

BSMETA(hotkey) = {
	BSREQ(id), BSREQ(keyid), BSREQ(key),
	{}};
BSMETA(hotkeylist) = {
	BSREQ(id),
	BSREQ(elements),
	{}};

namespace {
struct hotname {
	const char*		id;
	unsigned		key;
};
hotname names[] = {
	{"Down", KeyDown},
	{"End", KeyEnd},
	{"Esc", KeyEscape},
	{"Home", KeyHome},
	{"Left", KeyLeft},
	{"Right", KeyRight},
	{"PageUp", KeyPageUp},
	{"PageDown", KeyPageDown},
	{"Up", KeyUp},
};
}

static hotname* findname(const char* p) {
	for(auto& e : names) {
		if(equal(e.id, p))
			return &e;
	}
	return 0;
}

static unsigned parse_key(const char* p) {
	unsigned result = 0;
	while(*p) {
		if(ischa(*p)) {
			char temp[64]; stringbuilder sb(temp);
			p = sb.psidf(p);
			if(temp[1] == 0)
				result |= temp[0];
			else {
				auto pn = findname(temp);
				if(!pn) {
					log::error(0, "Wrong key name '%1' in hotkey", temp);
					return 0;
				} else
					result |= pn->key;
			}
		} else if(isnum(*p))
			result |= *p++;
		else if(*p == '+') {
			p++;
			continue;
		} else {
			p++;
			log::error(0, "Wrong key symbol '%1' in hotkey", p);
			return 0;
		}
	}
	return result;
}

void hotkey::initialize() {
	for(auto& e : bsdata<hotkey>()) {
		if(e.key)
			continue;
		if(!e.keyid) {
			log::error(0, "Empthy key mapping for script '%1'", e.id);
			continue;
		}
		auto ps = bsdata<script>::find(e.id);
		if(!ps) {
			log::error(0, "Can't find script named '%1' in hotkey mapping", e.id);
			continue;
		}
		e.proc = ps->proc;
		e.key = parse_key(e.keyid);
	}
}