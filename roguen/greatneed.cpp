#include "bsreq.h"
#include "greatneed.h"
#include "log.h"

void check_description(const char* id, const char** psuffix);

BSDATA(needni) = {
	{"NeedAccepted"},
	{"NeedSpecialApplied"},
	{"NeedCompleted"},
	{"NeedFinished"},
	{"NeedFail"},
	{"NeedSuccess"},
};
assert_enum(needni, NeedSuccess)
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
	BSREQ(coins),
	BSREQ(fail), BSREQ(success),
	{}};
BSDATAC(greatneedi, 32)
BSDATAC(greatneed, 64)

greatneed* greatneed::find(variant owner) {
	if(!owner)
		return 0;
	for(auto& e : bsdata<greatneed>()) {
		if(e.owner == owner)
			return &e;
	}
	return 0;
}

greatneed* greatneed::find(variant owner, needn f) {
	if(!owner)
		return 0;
	for(auto& e : bsdata<greatneed>()) {
		if(e.owner == owner && e.is(f))
			return &e;
	}
	return 0;
}

greatneed* greatneed::add(const greatneedi* type, variant owner, unsigned deadline) {
	//auto p = find(owner);
	//if(p)
	//	return p;
	auto p = bsdata<greatneed>::addz();
	p->clear();
	p->type = type - bsdata<greatneedi>::elements;
	p->owner = owner;
	p->deadline = deadline;
	return p;
}

void greatneed::shrink() {
	auto ps = bsdata<greatneed>::elements;
	for(auto& e : bsdata<greatneed>()) {
		if(!e)
			continue;
		*ps++ = e;
	}
	bsdata<greatneed>::source.count = ps - bsdata<greatneed>::elements;
}

void check_need_loading() {
	static const char* suffix[] = {"Completed", "Fail", "Partial", "Success", 0};
	log::seturl("Descriptions.txt");
	for(auto& e : bsdata<greatneedi>())
		check_description(e.id, suffix);
}