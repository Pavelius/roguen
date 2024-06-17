#include "actable.h"
#include "answers.h"
#include "charname.h"
#include "monster.h"
#include "pushvalue.h"
#include "textscript.h"

static array gamelog(1);

static void add_log(const char* format) {
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

static void named_say(stringbuilder& sb, const char* format, const char* name) {
	sb.addsep('\n');
	auto pb = sb.get();
	sb.add("[%1] \"", name);
	sb.addv(format, 0);
	sb.add("\"");
	logv(pb);
}

static const char* skipncr(const char* p) {
	while(*p && !(*p == 10 || *p == 13))
		p++;
	return skipspcr(p);
}

static void complex_say(stringbuilder& sb, const char* format, const char* name) {
	char dialog_text[512];
	while(format[0]) {
		auto pe = skipncr(format);
		auto len = pe - format;
		if(format[len] == 0) {
			named_say(sb, format, name);
			break;
		} else {
			if(len > sizeof(dialog_text) - 16)
				len = sizeof(dialog_text) - 16;
			memcpy(dialog_text, format, len);
			dialog_text[len] = 0;
			while(len > 0 && (dialog_text[len - 1] == 10 || dialog_text[len - 1] == 13))
				dialog_text[--len] = 0;
			named_say(sb, dialog_text, name);
			draw::pause();
			format = skipspcr(format + len);
		}
	}
}

static void say_format(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool isfemale) {
	pushvalue push_name(last_name, name);
	pushvalue push_female(last_female, isfemale);
	char temp[4096]; stringbuilder sba(temp);
	sba.addv(format, format_param);
	complex_say(sb, temp, name);
}

const char* getlog() {
	return gamelog.begin();
}

void logv(const char* format) {
	if(!format || format[0] == 0)
		return;
	if(gamelog.getcount() > 0)
		add_log("\n");
	add_log(format);
}

void logv(const char* format, const char* format_param, const char* name, bool female) {
	pushvalue push_name(last_name, name);
	pushvalue push_female(last_female, female);
	char temp[1024]; stringbuilder sb(temp); sb.clear();
	sb.addv(format, format_param);
	logv(temp);
}

void actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female, char separator) {
	if(!format)
		return;
	pushvalue push_name(last_name, name);
	pushvalue push_female(last_female, female);
	sb.addsep(separator);
	auto pb = sb.get();
	sb.addv(format, format_param);
	logv(pb);
}

void actvf(stringbuilder& sb, const char* name, bool female, char separator, const char* format, ...) {
	pushvalue push_name(last_name, name);
	pushvalue push_female(last_female, female);
	sb.addsep(separator);
	auto pb = sb.get();
	sb.addv(format, xva_start(format));
}

void actable::sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const {
	if(!name)
		name = getname();
	if(format[0] == '>')
		actv(sb, format + 1, format_param, name, female, '\n');
	else
		say_format(sb, format, format_param, name, female);
}

bool actable::iskind(variant v) const {
	if(v.iskind<monsteri>()) {
		if(!kind.iskind<monsteri>())
			return false;
		auto pn = bsdata<monsteri>::elements + v.value;
		auto pt = bsdata<monsteri>::elements + kind.value;
		return (pn == pt) || (pt->parent == pn);
	}
	return false;
}

bool actable::ischaracter() const {
	return !kind.iskind<monsteri>();
}

const char* actable::getname() const {
	if(name_id != 0xFFFF)
		return charname::getname(name_id);
	return kind.getname();
}

struct monsteri* actable::getmonster() const {
	return kind;
}