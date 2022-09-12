#include "answers.h"

const char* answers::header;
const char* answers::prompt;
const char* answers::prompa;
const char* answers::resid;
bool answers::show_tips = true;
bool answers::interactive = true;
int answers::column_count = 1;
stringbuilder* answers::console;
fnevent answers::beforepaint;
fnevent answers::afterpaint;
answers::fnpaint answers::paintcell;

int answers::compare(const void* v1, const void* v2) {
	return strcmp(((answers::element*)v1)->text, ((answers::element*)v2)->text);
}

void answers::addv(const void* value, const char* text, const char* format) {
	auto p = elements.add();
	p->value = value;
	p->text = sc.get();
	sc.addv(text, format);
	sc.addsz();
}

void answers::sort() {
	qsort(elements.data, elements.count, sizeof(elements.data[0]), compare);
}

void answers::modal(const char* title, const char* cancel) const {
	auto proc = (fnevent)choose(title, cancel);
	if(proc)
		proc();
}

void* answers::random() const {
	if(!elements.count)
		return 0;
	return (void*)elements.data[rand() % elements.count].value;
}

const char* answers::getname(void* v) {
	for(auto& e : elements) {
		if(e.value == v)
			return e.text;
	}
	return 0;
}

void answers::clear() {
	elements.clear();
	sc.clear();
}

void draw::pause() {
	pause(getnm("Continue"));
}

void draw::pause(const char* title, ...) {
	if(answers::console) {
		if(!(*answers::console))
			return;
	}
	char temp[260]; stringbuilder sb(temp);
	answers an; sb.addv(title, xva_start(title));
	an.choose(0, temp, true);
	if(answers::console)
		answers::console->clear();
}

void draw::pausenc(const char* title, ...) {
	char temp[260]; stringbuilder sb(temp);
	answers an; sb.addv(title, xva_start(title));
	an.choose(0, temp, true);
}

bool draw::yesno(const char* title, ...) {
	char temp[260]; stringbuilder sb(temp);
	sb.addv(title, xva_start(title));
	answers an;
	an.add((void*)1, getnm("Yes"));
	an.add((void*)0, getnm("No"));
	return an.choose(temp);
}

void draw::information(const char* format, ...) {
	if(!answers::console)
		return;
	answers::console->addn("[+");
	answers::console->addv(format, xva_start(format));
	answers::console->add("]");
}

void draw::warning(const char* format, ...) {
	if(!answers::console)
		return;
	answers::console->addn("[-");
	answers::console->addv(format, xva_start(format));
	answers::console->add("]");
}

void draw::output(const char* format, ...) {
	if(!answers::console)
		return;
	answers::console->addx("\n", format, xva_start(format));
}

static const char* find_separator(const char* pb) {
	auto p = pb;
	while(*p) {
		if(*p == '-' && p[1] == '-' && p[2] == '-' && (p[3] == 10 || p[3] == 13) && p > pb && (p[-1] == 10 || p[-1] == 13))
			return p;
		p++;
	}
	return 0;
}

void answers::message(const char* format) {
	if(!format)
		return;
	answers an;
	auto push_prompt = answers::prompt;
	while(true) {
		auto p = find_separator(format);
		if(!p)
			break;
		auto pn = skipspcr(p + 3);
		while(p < format && (p[-1] == 10 || p[-1] == 13))
			p--;
		char temp[4096];
		auto count = p - format;
		if(count > sizeof(temp) / sizeof(temp[0]) - 1)
			count = sizeof(temp) / sizeof(temp[0]) - 1;
		memcpy(temp, format, count);
		temp[count] = 0;
		an.prompt = temp;
		an.choose(0, getnm("Continue"), 1);
		format = pn;
	}
	an.prompt = format;
	an.choose(0, getnm("Continue"), 1);
	answers::prompt = push_prompt;
}