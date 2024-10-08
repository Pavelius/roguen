#include "answers.h"
#include "rand.h"

const char* answers::footer;
const char* answers::header;
const char* answers::prompt;
const char* answers::prompa;
const char* answers::resid;
const char* answers::cancel_text;
bool answers::show_tips = true;
bool answers::choosing = false;
bool answers::interactive = true;
int answers::column_count = 1;
fnevent answers::beforepaint;
fnevent answers::afterpaint;
answers::fnpaint answers::paintcell;
answers an;

int answers::compare(const void* v1, const void* v2) {
	return szcmp(((answers::element*)v1)->text, ((answers::element*)v2)->text);
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

void pause() {
	pause(getnm("Continue"));
}

void pause(const char* title, ...) {
	if(!(console))
		return;
	answers an;
	an.addv((void*)1, title, xva_start(title));
	an.choose();
	console.clear();
}

void pausenc(const char* title, ...) {
	char temp[260]; stringbuilder sb(temp);
	answers an; sb.addv(title, xva_start(title));
	an.choose(0, temp, true);
}

bool yesno(const char* title, ...) {
	if(title) {
		console.addsep(' ');
		console.addv(title, xva_start(title));
	}
	answers an;
	an.add((void*)1, getnm("Yes"));
	an.add((void*)0, getnm("No"));
	return an.choose();
}

void information(const char* format, ...) {
	console.addn("[+");
	console.addv(format, xva_start(format));
	console.add("]");
}

void warning(const char* format, ...) {
	console.addn("[-");
	console.addv(format, xva_start(format));
	console.add("]");
}

void output(const char* format, ...) {
	console.addx("\n", format, xva_start(format));
}