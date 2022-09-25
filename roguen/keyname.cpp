#include "crt.h"
#include "draw.h"

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
	{"Ctrl", Ctrl},
	{"Alt", Alt},
};
}

unsigned findkeybyname(const char* p) {
	for(auto& e : names) {
		if(equal(e.id, p))
			return e.key;
	}
	return 0;
}

const char* findnamebykey(unsigned key) {
	for(auto& e : names) {
		if(e.key == key)
			return e.id;
	}
	return 0;
}

static void add_prefix_key(stringbuilder& sb, unsigned& key, int prefix) {
	if((key & prefix)==0)
		return;
	auto pn = findnamebykey(prefix);
	if(!pn)
		return;
	sb.add(pn);
	key = key & (~prefix);
	if(key)
		sb.add("+");
}

void addkeyname(stringbuilder& sb, unsigned key) {
	add_prefix_key(sb, key, Ctrl);
	add_prefix_key(sb, key, Alt);
	add_prefix_key(sb, key, Shift);
	auto pn = findnamebykey(key);
	if(pn)
		sb.add(pn);
	else
		sb.addsym(key);
}