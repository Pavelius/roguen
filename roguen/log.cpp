#include "io_stream.h"
#include "log.h"
#include "stringbuilder.h"

bool log::allowparse = true;
static int error_count;
static const char* current_url;
static const char* current_file;

void log::setfile(const char* v) {
	current_file = v;
}

void log::seturl(const char* v) {
	current_url = v;
}

const char* log::read(const char* url, bool error_if_not_exist) {
	auto p_alloc = loadt(url);
	if(!p_alloc) {
		current_url = 0;
		if(error_if_not_exist)
			errorp(0, "Can't find file '%1'", url);
		return 0;
	}
	seturl(url);
	setfile(p_alloc);
	return p_alloc;
}

void log::close() {
	if(current_file)
		delete current_file;
	current_file = 0;
}

const char* endline(const char* p) {
	while(*p && !(*p == 10 || *p == 13))
		p++;
	return p;
}

int getline(const char* pb, const char* pc) {
	auto p = pb;
	auto r = 0;
	while(*p && p < pc) {
		p = endline(p);
		p = skipcr(p);
		r++;
	}
	return r;
}

void log::errorv(const char* position, const char* format) {
	static io::file file("errors.txt", StreamWrite | StreamText);
	if(!file)
		return;
	error_count++;
	if(position)
		file << " Line " << getline(current_file, position) << ": ";
	file << format << "\n";
}

void log::errorp(const char* position, const char* format, ...) {
	char temp[4096]; stringbuilder sb(temp);
	if(current_url) {
		sb.add("In file `%1`:", current_url);
		current_url = 0;
		errorv(0, temp);
		sb.clear();
	}
	sb.addv(format, xva_start(format));
	errorv(position, temp);
}

int log::geterrors() {
	return error_count;
}

const char* log::skipws(const char* p) {
	while(true) {
		if(*p == ' ' || *p == 9) {
			p++;
			continue;
		}
		if(p[0] == '\\' && (p[1] == 10 || p[1] == 13)) { // Trail symbol
			p++;
			while(*p == 10 || *p == 13)
				p++;
			continue;
		}
		if(p[0] == '/' && p[1] == '/') { // End line comment
			p += 2;
			while(*p && *p != 10 && *p != 13)
				p++;
		}
		if(p[0] == '/' && p[1] == '*') { // Complex comment
			p += 2;
			while(*p && !(p[0] == '*' && p[1] == '/'))
				p++;
		}
		break;
	}
	return p;
}

const char* log::skipwscr(const char* p) {
	while(true) {
		p = skipws(p);
		if(*p == 10 || *p == 13) {
			while(*p == 10 || *p == 13)
				p++;
			continue;
		}
		break;
	}
	return p;
}

void log::readf(fnread proc, const char* folder, const char* filter) {
	for(io::file::find find(folder); find; find.next()) {
		auto pn = find.name();
		if(!pn || pn[0] == '.')
			continue;
		if(filter) {
			if(!szpmatch(pn, filter))
				continue;
		}
		char temp2[260];
		find.fullname(temp2);
		proc(temp2);
	}
}

void log::readlf(fnread proc, const char* folder, const char* filter) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%1/%2", folder, current_locale);
	readf(proc, temp, filter);
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