#include "logparse.h"

const char* log::readstr(const char* p, stringbuilder& result) {
	result.clear();
	return result.psstrlf(p);
}

const char* log::readidn(const char* p, stringbuilder& result) {
	result.clear();
	return result.psidf(p);
}

const char* log::readname(const char* p, stringbuilder& result) {
	result.clear();
	while(*p) {
		if(*p == '\n' || *p == '\r' || *p == ',')
			break;
		result.addch(*p++);
	}
	return p;
}

const char* log::readval(const char* p, stringbuilder& sb, variant& result) {
	p = readidn(p, sb);
	result = (const char*)sb.begin();
	if(!result)
		log::errorp(p, "Unknown variant identifier `%1`", sb.begin());
	int bonus;
	p = readbon(p, bonus);
	result.counter = bonus;
	return p;
}

const char* log::readbon(const char* p, int& bonus) {
	if(*p == '-')
		p = psnum(p, bonus);
	else if(*p == '+')
		p = psnum(p + 1, bonus);
	else
		bonus = 0;
	return p;
}

const char* log::getstring(stringbuilder& sb) {
	auto p = sb.begin();
	if(!p[0])
		return 0;
	return szdup(p);
}

static const char* example(const char* p, stringbuilder& sb) {
	while(*p && *p != '\n' && *p != '\r') {
		if(sb.isfull())
			break;
		sb.add(*p++);
	}
	return sb.begin();
}

bool log::checksym(const char* p, char sym) {
	if(!allowparse)
		return false;
	if(sym == '\n') {
		if(*p != '\n' && *p != '\r') {
			log::errorp(p, "Expected symbol line feed");
			allowparse = false;
			return false;
		}
	} else if(*p != sym) {
		char result[] = {sym, 0};
		char string[16]; stringbuilder sb(string); sb.clear();
		log::errorp(p, "Expected symbol `%1`, but you have string `%2`", result, example(p, sb));
		allowparse = false;
		return false;
	}
	return true;
}

const char* log::valuei::read(const char* p, stringbuilder& sb) {
	clear();
	if(*p == '\"') {
		sb.clear(); p = sb.psstr(p + 1, *p);
		text = szdup(sb.begin());
	} else if(*p == '-' || isnum(*p)) {
		auto minus = false;
		if(*p == '-') {
			minus = true;
			p++;
		}
		p = psnum(p, number);
		if(minus)
			number = -number;
	} else if(ischa(p[0])) {
		sb.clear(); p = sb.psidf(p);
		identifier = (const char*)sb.begin();
		if(identifier) {
			int bonus; p = readbon(p, bonus);
			identifier.counter = bonus;
			data = identifier.getpointer();
		} else
			text = szdup(sb.begin());
	}
	return p;
}