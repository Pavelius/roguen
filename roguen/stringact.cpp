#include "crt.h"
#include "stringact.h"

namespace {
struct change_string {
	const char*	id;
	const char*	female;
	const char*	male;
	const char*	multiply;
};
static change_string player_gender[] = {{"а", "а", "", "и"},
	{"ла", "ла", "", "ли"},
	{"ась", "ась", "ся", "ись"},
	{"ая", "ая", "ый", "ые"},
	{"ей", "ей", "ему", "им"},
	{"нее", "нее", "него", "них"},
	{"она", "она", "он", "они"},
	{"ее", "ее", "его", "их"},
	{"стерва", "стерва", "ублюдок", "ублюдки"},
	{"леди", "леди", "лорды", "лорды"},
	{"миледи", "миледи", "милорд", "милорды"},
	{"госпожа", "госпожа", "господин", "господа"},
	{"женщина", "женщина", "мужчина", "господа"},
};
}

void stringact::addidentifier(const char* identifier) {
	if(name && equal(identifier, "герой") || equal(identifier, "name"))
		add(name);
	else if(name && strcmp(identifier, "героя") == 0)
		addof(name);
	else if(name && strcmp(identifier, "герою") == 0)
		addto(name);
	else {
		for(auto& e : player_gender) {
			if(strcmp(e.female, identifier) != 0)
				continue;
			if(female)
				add(e.female);
			else
				add(e.male);
			return;
		}
		stringbuilder::addidentifier(identifier);
	}
}