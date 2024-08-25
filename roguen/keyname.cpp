#include "crt.h"
#include "draw.h"

namespace {
struct hotname {
	const char*		id;
	unsigned		key;
	const char*		format;
};
hotname names[] = {
	{"Backspace", KeyBackspace},
	{"Down", KeyDown},
	{"End", KeyEnd},
	{"Esc", KeyEscape},
	{"Home", KeyHome},
	{"Left", KeyLeft},
	{"Right", KeyRight},
	{"PageUp", KeyPageUp, "Page Up"},
	{"PageDown", KeyPageDown, "Page Down"},
	{"Up", KeyUp},
	{"Ctrl", Ctrl},
	{"Alt", Alt},
};
}

static const hotname* findhot(unsigned key) {
	for(auto& e : names) {
		if(e.key == key)
			return &e;
	}
	return 0;
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
	auto pn = findhot(key);
	if(pn)
		sb.add(pn->format ? pn->format : pn->id);
	else
		sb.add(key);
}