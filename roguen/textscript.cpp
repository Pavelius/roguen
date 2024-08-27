#include "creature.h"
#include "textscript.h"
#include "speech.h"

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

bool parse_speech(stringbuilder& sb, const char* id);

static array gamelog(1);

static const char* get_player_name() {
	if(!player)
		return 0;
	return player->getname();
}

static bool is_player_female() {
	if(!player)
		return false;
	return player->is(Female);
}

static void logc(const char* format) {
	if(!format || format[0] == 0)
		return;
	auto i = zlen(format);
	gamelog.reserve(gamelog.getcount() + i + 1);
	auto m = gamelog.getcount();
	if(m) {
		memcpy(gamelog.ptr(m - 1), format, i + 1);
		gamelog.count += i;
	} else {
		memcpy(gamelog.ptr(0), format, i + 1);
		gamelog.count += i + 1;
	}
}

void logv(const char* format) {
	if(!format || !format[0])
		return;
	if(gamelog.getcount() > 0)
		logc("\n");
	logc(format);
}

static void named_say(stringbuilder& sb, const char* format) {
	sb.addsep('\n');
	auto pb = sb.get();
	sb.add("[%1] \"", get_player_name());
	sb.addv(format, 0);
	sb.add("\"");
	logv(pb);
}

static const char* skipncr(const char* p) {
	while(*p && !(*p == 10 || *p == 13))
		p++;
	return skipspcr(p);
}

void sayv(stringbuilder& sb, const char* format) {
	char dialog_text[512];
	while(format[0]) {
		auto pe = skipncr(format);
		auto len = pe - format;
		if(format[len] == 0) {
			named_say(sb, format);
			break;
		} else {
			if(len > sizeof(dialog_text) - 16)
				len = sizeof(dialog_text) - 16;
			memcpy(dialog_text, format, len);
			dialog_text[len] = 0;
			while(len > 0 && (dialog_text[len - 1] == 10 || dialog_text[len - 1] == 13))
				dialog_text[--len] = 0;
			named_say(sb, dialog_text);
			pause();
			format = skipspcr(format + len);
		}
	}
}

static bool parse_name(stringbuilder& sb, const char* id) {
	auto name = get_player_name();
	if(!name || !name[0])
		return false;
	if(equal(id, "герой") || equal(id, "name"))
		sb.add(name);
	else if(equal(id, "героя"))
		sb.addof(name);
	else if(equal(id, "герою"))
		sb.addto(name);
	else
		return false;
	return true;
}

static bool parse_gender(stringbuilder& sb, const char* id) {
	auto female = is_player_female();
	for(auto& e : player_gender) {
		if(!equal(e.female, id))
			continue;
		if(female)
			sb.add(e.female);
		else
			sb.add(e.male);
		return true;
	}
	return false;
}

static bool parse_script(stringbuilder& sb, const char* id) {
	for(auto& e : bsdata<textscript>()) {
		if(!equal(e.id, id))
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
	if(parse_speech(sb, id))
		return;
	sb.add(getnm(id));
}

const char* getlog() {
	return gamelog.begin();
}

void logv(const char* format, const char* format_param) {
	char temp[1024]; stringbuilder sb(temp); sb.clear();
	sb.addv(format, format_param);
	logv(temp);
}

void actv(stringbuilder& sb, const char* format, const char* format_param, char separator) {
	if(!format)
		return;
	sb.addsep(separator);
	auto pb = sb.get();
	sb.addv(format, format_param);
	logv(pb);
}

static const char* getformat(const char* id, const char* action) {
	if(!id)
		return getnme(action);
	char temp[64]; stringbuilder sb(temp);
	sb.add(id);
	sb.add(action);
	return getnme(temp);
}

bool actn(stringbuilder& sb, const char* id, const char* action, const char* format_param, char separator) {
	auto format = getformat(id, action);
	if(!format)
		return false;
	actv(sb, format, format_param, separator);
	return true;
}

void actvf(stringbuilder& sb, char separator, const char* format, ...) {
	sb.addx(separator, format, xva_start(format));
}

void sayv(stringbuilder& sb, const char* format, const char* format_param) {
	char temp[4096]; stringbuilder sba(temp);
	sba.addv(format, format_param);
	sayv(sb, temp);
}

void initialize_strings() {
	stringbuilder::custom = custom_string;
}