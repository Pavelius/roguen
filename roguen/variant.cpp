#include "variant.h"

static const char* match(const char* text, const char* name) {
	while(*name) {
		if(*name++ != *text++)
			return 0;
	}
	return text;
}

const char* variant::getdescription() const {
	auto& e = bsdata<varianti>::elements[type];
	if(!e.source || !e.isnamed())
		return 0;
	return ::getdescription(getid());
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
			type = (variant_s)(&e - bsdata<varianti>::elements);
			break;
		}
	}
}

int varianti::found(const char* id, size_t size) const {
	int i = -1;
	if(isnamed())
		i = source->findps(id, 0, size);
	return i;
}

const varianti* varianti::getsource(const char* id) {
	if(id) {
		for(auto& e : bsdata<varianti>()) {
			if(!e.source || !e.id)
				continue;
			if(strcmp(e.id, id) == 0)
				return &e;
		}
	}
	return 0;
}

const char* varianti::getname(const void* object) const {
	if(pgetname)
		return pgetname(object);
	if(isnamed()) {
		auto id = *((const char**)object);
		if(id)
			return getnm(id);
	}
	return getnm("NoName");
}

const char* varianti::getid(const void* object) const {
	if(isnamed())
		return *((const char**)object);
	return "NoName";
}

const varianti* varianti::getmetadata(const void* object) {
	if(object) {
		for(auto& e : bsdata<varianti>()) {
			if(e.source && e.source->have(object))
				return &e;
		}
	}
	return 0;
}

const char* getvalues(const char* p, size_t& size, int& counter) {
	auto pb = p;
	while(ischa(*p) || isnum(*p) || *p == '_')
		p++;
	size = p - pb;
	if(*p == '-')
		stringbuilder::read(p, counter);
	else if(*p == '+')
		stringbuilder::read(p + 1, counter);
	else
		counter = 0;
	return p;
}

template<> variant::variant(const char* v) : u(0) {
	if(v) {
		auto size = zlen(v);
		for(auto& e : bsdata<varianti>()) {
			if(!e.source || !e.metadata || e.key_count != 1)
				continue;
			int i = e.found(v, size);
			if(i != -1) {
				value = i;
				type = (variant_s)(&e - bsdata<varianti>::elements);
				counter = 0;
				break;
			}
		}
	}
}