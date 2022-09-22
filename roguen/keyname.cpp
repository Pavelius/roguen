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