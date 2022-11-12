#include "bsreq.h"
#include "greatneed.h"

BSDATA(needni) = {
	{"NeedAccepted"},
};
assert_enum(needni, NeedAccepted)
BSMETA(needni) = {
	BSREQ(id),
	{}};
BSMETA(greatneedi) = {
	BSREQ(id),
	BSREQ(need),
	BSREQ(targets),
	BSREQ(special),
	BSREQ(level),
	BSREQ(flags),
	BSREQ(fail), BSREQ(success),
	{}};
BSDATAC(greatneedi, 32)
BSDATAC(greatneed, 32)

greatneed* greatneed::find(variant owner) {
	if(!owner)
		return 0;
	for(auto& e : bsdata<greatneed>()) {
		if(e.owner == owner)
			return &e;
	}
	return 0;
}

greatneed* greatneed::add(const greatneedi* type, variant owner, unsigned deadline) {
	auto p = find(owner);
	if(p)
		return p;
	p = bsdata<greatneed>::addz();
	p->clear();
	p->type = type - bsdata<greatneedi>::elements;
	p->random = rand();
	p->owner = owner;
	p->deadline = deadline;
	return p;
}