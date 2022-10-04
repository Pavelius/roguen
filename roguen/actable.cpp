#include "stringact.h"
#include "main.h"

void actable::actv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female, char separator) const {
	if(!name)
		name = getname();
	stringact sa(sb, getname(), female);
	sa.addsep(separator);
	sa.addv(format, format_param);
	sb = sa;
}

void actable::sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const {
	if(!name)
		name = getname();
	stringact sa(sb, getname(), female);
	sa.addn("[%1] \"", name);
	sa.addv(format, format_param);
	sa.add("\"");
	sb = sa;
}

bool actable::confirm(const char* format, ...) {
	console.addsep('\n');
	console.addv(format, xva_start(format));
	answers an;
	an.add((void*)1, getnm("Yes"));
	an.add((void*)0, getnm("No"));
	auto result = an.choose();
	console.clear();
	return result != 0;
}

void actable::pressspace() {
	answers an;
	an.add((void*)1, getnm("Continue"));
	auto result = an.choose();
	console.clear();
}