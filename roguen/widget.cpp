#include "bsreq.h"
#include "crt.h"
#include "widget.h"

BSDATAC(widget, 128)
BSMETA(widget) = {
	BSREQ(id),
	{}};

const widget* widget::last;

void widget::paint() const {
	auto push_last = last;
	last = this; proc();
	last = push_last;
}

void widget::add(const char* id, fnevent proc, fnevent click) {
	auto p = (widget*)bsdata<widget>::source.addfind(id);
	p->proc = proc;
	p->click = click;
}