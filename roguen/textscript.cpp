#include "crt.h"
#include "textscript.h"

const char* last_name;
bool		last_female;

namespace {
struct change_string {
	const char*	female;
	const char*	male;
	const char*	multiply;
};
static change_string player_gender[] = {
	{"а", "", "и"},
	{"ла", "", "ли"},
	{"ась", "ся", "ись"},
	{"ая", "ый", "ые"},
	{"ей", "ему", "им"},
	{"нее", "него", "них"},
	{"она", "он", "они"},
	{"ее", "его", "их"},
};
}

static bool parse_name(stringbuilder& sb, const char* id) {
	if(!last_name)
		return false;
	if(equal(id, "герой") || equal(id, "name"))
		sb.add(last_name);
	else if(strcmp(id, "героя") == 0)
		sb.addof(last_name);
	else if(strcmp(id, "герою") == 0)
		sb.addto(last_name);
	else
		return false;
	return true;
}

static bool parse_gender(stringbuilder& sb, const char* id) {
	for(auto& e : player_gender) {
		if(strcmp(e.female, id) != 0)
			continue;
		if(last_female)
			sb.add(e.female);
		else
			sb.add(e.male);
		return true;
	}
	return false;
}

static bool parse_script(stringbuilder& sb, const char* id) {
	for(auto& e : bsdata<textscript>()) {
		if(strcmp(e.id, id) != 0)
			continue;
		e.proc(sb);
		return true;
	}
	return false;
}

static void custom_string(stringbuilder& sb, const char* id) {
	if(parse_name(sb, id))
		return;
	if(parse_gender(sb, id))
		return;
	if(parse_script(sb, id))
		return;
	sb.defidentifier(sb, id);
}

void textscript::initialize() {
	stringbuilder::custom = custom_string;
}