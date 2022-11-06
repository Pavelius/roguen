#include "stringact.h"
#include "main.h"

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

const char* actable::getlog() {
	return gamelog.begin();
}

void actable::logv(const char* format, const char* format_param, const char* name, bool female) {
	char temp[1024]; stringbuilder sb(temp); sb.clear();
	stringact sa(sb, name, female);
	if(gamelog.getcount() > 0)
		sa.add("\n");
	sa.addv(format, format_param);
	add_log(temp);
}

void actable::actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female, char separator) {
	if(!format)
		return;
	stringact sa(sb, name, female);
	sa.addsep(separator);
	auto pb = sa.get();
	sa.addv(format, format_param);
	sb = sa;
	logv(pb, 0, 0, false);
}

void actable::actvf(stringbuilder& sb, const char* name, bool female, char separator, const char* format, ...) {
	stringact sa(sb, name, female);
	sa.addsep(separator);
	auto pb = sa.get();
	sa.addv(format, xva_start(format));
	sb = sa;
}

void actable::sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const {
	if(!name)
		name = getname();
	if(format[0] == '>') {
		actv(sb, format + 1, format_param, name, female);
		return;
	}
	stringact sa(sb, getname(), female);
	sa.addsep('\n');
	auto pb = sa.get();
	sa.add("[%1] \"", name);
	sa.addv(format, format_param);
	sa.add("\"");
	sb = sa;
	logv(pb, 0, 0, false);
}

bool actable::confirm(const char* format, ...) {
	console.clear();
	console.addv(format, xva_start(format));
	answers an;
	an.add((void*)1, getnm("Yes"));
	an.add((void*)0, getnm("No"));
	auto result = an.choose();
	console.clear();
	return result != 0;
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
	return kind.iskind<racei>();
}

const char* actable::getname() const {
	if(name_id != 0xFFFF)
		return charname::getname(name_id);
	return kind.getname();
}

struct monsteri* actable::getmonster() const {
	return kind;
}