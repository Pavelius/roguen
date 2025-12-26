#include "variant.h"

static const char* match(const char* text, const char* name) {
	while(*name) {
		if(*name++ != *text++)
			return 0;
	}
	return text;
}

const char* variant::getname() const {
	auto& e = to();
	if(!e.source)
		return getnm("NoVariant");
	return e.getname(getpointer());
}

const char* variant::getid() const {
	auto& e = to();
	if(!e.source)
		return "NoVariant";
	return e.getid(getpointer());
}

template<> variant::variant(const void* v) : u(0) {
	for(auto& e : bsdata<varianti>()) {
		if(!e.source)
			continue;
		auto i = e.source->indexof(v);
		if(i != -1) {
			value = i;
			type = &e - bsdata<varianti>::elements;
			break;
		}
	}
}

int varianti::found(const char* id, size_t size) const {
	return isnamed() ? source->indexof(source->findv(id, 0, size)) : -1;
}

const varianti* varianti::getsource(const char* id) {
	if(id) {
		for(auto& e : bsdata<varianti>()) {
			if(!e.source || !e.id)
				continue;
			if(equal(e.id, id))
				return &e;
		}
	}
	return 0;
}

const char* varianti::getname(const void* drawobject) const {
	if(isnamed()) {
		auto id = *((const char**)drawobject);
		if(id)
			return getnm(id);
	}
	return getnm("NoName");
}

const char* varianti::getid(const void* drawobject) const {
	if(isnamed())
		return *((const char**)drawobject);
	return "NoName";
}

template<> variant::variant(const char* v) : u(0) {
	if(v) {
		auto size = zlen(v) + 1;
		if(size <= 1)
			return;
		for(auto& e : bsdata<varianti>()) {
			if(!e.source || !e.metadata || e.key_count != 1)
				continue;
			int i = e.found(v, size);
			if(i != -1) {
				value = i;
				type = &e - bsdata<varianti>::elements;
				counter = 0;
				break;
			}
		}
	}
}