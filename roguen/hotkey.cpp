#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "dialog.h"
#include "hotkey.h"
#include "keyname.h"
#include "log.h"
#include "script.h"
#include "stringbuilder.h"

BSDATAC(hotkey, 256)
BSMETA(hotkey) = {
	BSREQ(keyid), BSREQ(id),
	{}};

static unsigned parse_key(const char* p) {
	unsigned result = 0;
	while(*p) {
		if(ischa(*p)) {
			char temp[64]; stringbuilder sb(temp);
			p = sb.psidf(p);
			if(temp[1] == 0)
				result |= temp[0];
			else {
				auto key = findkeybyname(temp);
				if(!key) {
					log::error(0, "Wrong key name '%1' in hotkey", temp);
					return 0;
				} else
					result |= key;
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
		if(e.keyid) {
			e.key = parse_key(e.keyid);
			if(!e.key)
				continue;
		}
		e.data = e.id;
		if(!e.data)
			log::error(0, "Can't find script or dialog with name '%1'", e.id);
	}
}