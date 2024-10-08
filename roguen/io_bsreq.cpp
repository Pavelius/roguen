#include "bsreq.h"
#include "log.h"
#include "logvalue.h"
#include "pushvalue.h"
#include "stringbuilder.h"
#include "variant.h"

using namespace log;

static char	temp[512];
static int last_bonus;
static const char* p;

static bool compare(const void* p, const bsreq* requisit, const valuei& value) {
	auto pv = requisit->ptr(p);
	if(requisit->is(KindText)) {
		auto v1 = *((const char**)pv);
		auto v2 = value.text;
		if(!v1 && !v2)
			return true;
		if(v1 && v2)
			return equal(v1, v2);
	} else {
		auto v1 = requisit->get(pv);
		return value.number == v1;
	}
	return false;
}

static bool compare(const void* p, const bsreq* type, const valuei* keys, int key_count) {
	for(int i = 0; i < key_count; i++) {
		if(!compare(p, type + i, keys[i]))
			return false;
	}
	return true;
}

static void next() {
	p = skipws(p);
}

static bool isequal(const char* pn) {
	auto i = 0;
	while(pn[i]) {
		if(pn[i] != p[i])
			return false;
		i++;
	}
	p += i;
	return true;
}

static void skip(const char* command) {
	if(allowparse && !isequal(command)) {
		errorp(p, "Expected token `%1`", command);
		allowparse = false;
	}
}

static bool isvalue() {
	return (p[0] == '-' && isnum(p[1]))
		|| (p[0] == '\"')
		|| isnum(p[0])
		|| ischa(p[0]);
}

static bool islevel(int level) {
	auto push_p = p;
	p = skipsp(p);
	auto i = p - push_p;
	if(i == level) {
		p = skipws(p);
		return true;
	}
	p = push_p;
	return false;
}

static void readid() {
	stringbuilder sb(temp); temp[0] = 0;
	if(!ischa(*p)) {
		errorp(p, "Expected identifier");
		allowparse = false;
	} else
		p = sb.psidf(p);
	next();
}

static const char* readbonus(const char* p) {
	if(*p == '-')
		p = psnum(p, last_bonus);
	else if(*p == '+')
		p = psnum(p + 1, last_bonus);
	else
		last_bonus = 0;
	return p;
}

static void readidbonus() {
	stringbuilder sb(temp); temp[0] = 0;
	if(!ischa(*p)) {
		errorp(p, "Expected identifier");
		allowparse = false;
	} else {
		p = sb.psidf(p);
		p = readbonus(p);
	}
	next();
}

static const char* skipallcr(const char* p) {
	while(*p && (*p == 10 || *p == 13))
		p++;
	return p;
}

static void skiplinefeed() {
	if(*p && *p != 10 && *p != 13) {
		errorp(p, "Expected line feed");
		allowparse = false;
	}
	while(*p && (*p == 10 || *p == 13)) {
		p = skipallcr(p);
		if(*p == ' ' || *p == 9)
			break;
		p = skipws(p);
	}
}

static void skipline() {
	while(*p && *p != 10 && *p != 13) {
		auto p1 = p;
		p = skipws(p);
		if(p1 != p)
			continue;
		p++;
	}
}

static varianti* find_type(const char* id) {
	for(auto& e : bsdata<varianti>()) {
		if(!e.source || !e.id)
			continue;
		if(equal(e.id, id))
			return &e;
	}
	return 0;
}

static const bsreq* find_requisit(const bsreq* type, const char* id) {
	if(!type)
		return 0;
	auto req = type->find(temp);
	if(!req)
		errorp(p, "Not found requisit `%1`", id);
	return req;
}

static const bsreq* find_key(const bsreq* type) {
	return type->find("id", bsmeta<const char*>::meta);
}

static void* find_object(array* source, const bsreq* type, valuei* keys, int key_count) {
	auto pe = source->end();
	for(auto p = source->begin(); p < pe; p += source->element_size) {
		if(compare(p, type, keys, key_count))
			return p;
	}
	return 0;
}

static void clear_object(void* object, const bsreq* type) {
	for(auto req = type; *req; req++) {
		auto p = req->ptr(object);
		memset(p, 0, req->lenght);
	}
}

static void read_value(valuei& e, const bsreq* req) {
	e.clear();
	if(*p == '\"') {
		stringbuilder sb(temp);
		p = sb.psstr(p + 1, *p);
		e.text = szdup(temp);
	} else if(isequal("true"))
		e.number = 1;
	else if(isequal("false"))
		e.number = 0;
	else if(*p == '-' || isnum(*p)) {
		auto minus = false;
		if(*p == '-') {
			minus = true;
			p++;
		}
		p = psnum(p, e.number);
		if(minus)
			e.number = -e.number;
	} else if(ischa(p[0])) {
		readidbonus();
		if(!req) {
			// Nothing to do
		} else if(req->is(KindText))
			e.text = szdup(temp);
		else if(req->type == bsmeta<variant>::meta) {
			variant v1 = (const char*)temp;
			if(!v1)
				errorp(p, "Can't find variant `%1`", temp);
			v1.counter = last_bonus;
			e.number = v1.u;
		} else {
			auto shift = 0;
			auto pk = find_key(req->type);
			if(pk)
				shift = pk->offset;
			if(!req->source)
				errorp(p, "Invalid source array where read identifier `%1`", temp);
			else {
				e.number = req->source->indexof(req->source->findv(temp, shift));
				if(e.number == -1) {
					errorp(p, "Not found identifier `%1`", temp);
					e.number = 0;
				} else
					e.data = req->source->ptr(e.number);
			}
		}
	}
	next();
}

static void write_value(void* object, const bsreq* req, int index, const valuei& v) {
	if(!req)
		return;
	auto p1 = req->ptr(object, index);
	if(req->is(KindSlice)) {
		auto ps = (sliceu<int>*)req->ptr(object);
		if(req->source == bsdata<variant>::source_ptr) {
			auto ci = req->source->getcount();
			req->source->add();
			if(!ps->count)
				ps->start = ci;
			p1 = (char*)req->source->ptr(ps->start + (ps->count++));
			req->set(p1, v.number);
		} else
			ps->count++;
	} else if(req->is(KindNumber) || req->is(KindEnum) || req->type == bsmeta<variant>::meta)
		req->set(p1, v.number);
	else if(req->is(KindText))
		req->set(p1, (long)szdup(v.text));
	else if(req->is(KindScalar))
		write_value(req->ptr(object), req->type + index, 0, v);
	else if(req->is(KindADat)) {
		auto p2 = (char*)req->ptr(object);
		auto pc = (int*)(p2 + FO(adat<char>, count));
		auto pd = p2 + FO(adat<char>, data);
		if(index > (int)req->count)
			index = 0;
		req->set(pd + index * req->size, v.number);
		if(index + 1 > *pc)
			*pc = index + 1;
	} else if(req->is(KindDSet))
		req->set(p1, v.number);
	else if(req->is(KindFlags)) {
		auto data = (unsigned char*)req->ptr(object);
		data[v.number / 8] |= 1 << (v.number % 8);
	} else if(req->is(KindReference))
		req->set(p1, (long)v.data);
	else
		errorp(p, "Unknown type in requisit `%1`", req->id);
}

static void fill_object(void* object, const bsreq* type, const valuei* keys, int key_count) {
	for(int i = 0; i < key_count; i++)
		write_value(object, type + i, 0, keys[i]);
}

static void read_dset(void* object, const bsreq* req) {
	auto index = 0;
	while(allowparse && isvalue()) {
		valuei v;
		readid();
		index = req->source->indexof(req->source->findv(temp, 0));
		if(index == -1) {
			index = 0;
			errorp(p, "Not found field `%1` in dataset `%2`", temp, req->id);
		}
		skip("(");
		read_value(v, req);
		skip(")");
		next();
		write_value(object, req, index++, v);
	}
}

static void read_element(void* object, const bsreq* type, int level) {
	valuei v;
	auto req = type;
	while(allowparse && isvalue()) {
		v.clear();
		read_value(v, req);
		write_value(object, req, 0, v);
		req++;
	}
}

static void read_slice(void* object, const bsreq* req, int level) {
	auto ps = (sliceu<int>*)req->ptr(object);
	if(!ps->count)
		ps->start = req->source->getcount();
	while(allowparse && isvalue()) {
		auto element = req->source->add();
		clear_object(element, req->type);
		read_element(element, req->type, level);
		ps->count++;
	}
	// TODO: After reading slice, check if we have same records
}

static void read_array(void* object, const bsreq* req) {
	auto index = 0;
	while(allowparse && isvalue()) {
		valuei v;
		read_value(v, req);
		write_value(object, req, index++, v);
	}
}

static void read_dictionary(void* object, const bsreq* type, int level, bool need_linefeed = true) {
	while(allowparse && ischa(*p)) {
		readid();
		auto req = find_requisit(type, temp);
		skip("(");
		read_array(object, req);
		skip(")");
		next();
	}
	if(need_linefeed) {
		skiplinefeed();
		auto index = 0;
		while(allowparse && islevel(level + 1)) {
			if(*p != 10 && *p != 13) {
				readid();
				auto req = type->find(temp);
				if(!req) {
					errorp(p, "Not found requisit `%1`", temp);
					allowparse = false;
				} else if(req->is(KindDSet))
					read_dset(object, req);
				else if(req->is(KindSlice)) {
					if(req->source == bsdata<variant>::source_ptr)
						read_array(object, req);
					else
						read_slice(object, req, level + 1);
				} else if(req->is(KindScalar) && req->count > 0)
					read_dictionary(req->ptr(object, index), req->type, level + 1, false);
				else
					read_array(object, req);
				index++;
			}
			skiplinefeed();
		}
	}
}

static void* read_object(const bsreq* type, array* source, int key_count, int level, const char* common_initialize) {
	if(!isvalue()) {
		errorp(p, "Expected value");
		allowparse = false;
	}
	valuei keys[8] = {};
	void* object = 0;
	if(key_count) {
		for(auto i = 0; i < key_count; i++)
			read_value(keys[i], type + i);
		object = find_object(source, type, keys, key_count);
	}
	if(!object) {
		object = source->add();
		clear_object(object, type);
		fill_object(object, type, keys, key_count);
		if(common_initialize) {
			auto push_p = p; p = common_initialize;
			read_dictionary(object, type + key_count, level);
			p = push_p;
		}
	}
	read_dictionary(object, type + key_count, level);
	return object;
}

static const char* read_string(bool required = false) {
	valuei value;
	auto pn = p;
	read_value(value, 0);
	if(required && !value.text) {
		errorp(pn, "Expected string value");
		allowparse = false;
		return 0;
	}
	return value.text;
}

static fnread read_reader() {
	stringbuilder sb(temp);
	auto pn = p;
	sb.clear(); p = sb.psidf(p);
	next();
	if(!sb)
		return 0;
	if(equal(temp, "Default"))
		return 0;
	auto p = find_type(temp);
	if(!p) {
		errorp(pn, "Can't find type `%1`", temp);
		allowparse = false;
		return 0;
	}
	if(!p->pread) {
		errorp(pn, "In type `%1` don't defined file reader", temp);
		allowparse = false;
		return 0;
	}
	return p->pread;
}

static bool parse_directives() {
	if(equal(temp, "include")) {
		auto url = read_string(true);
		if(!url)
			return true;
		auto reader = read_reader();
		auto mask_name = read_string();
		if(!reader)
			reader = bsreq::read;
		char temp[260]; stringbuilder sb(temp);
		sb.add(url, current_locale);
		if(!allowparse)
			return true;
		if(mask_name)
			readf(reader, temp, mask_name);
		else
			reader(szdup(temp));
		return true;
	} else if(equal(temp, "setlocale")) {
		auto url = read_string(true);
		if(!url)
			return true;
		stringbuilder sb(current_locale);
		sb.addv(url, 0);
		return true;
	}
	return false;
}

static void parse() {
	while(*p && allowparse) {
		skip("#");
		if(!allowparse)
			break;
		readid();
		if(parse_directives()) {
			skipline();
			skiplinefeed();
			continue;
		}
		auto pd = find_type(temp);
		if(!pd) {
			if(temp[0])
				errorp(p, "Not find data type for `%1`", temp);
			return;
		}
		auto common_initialize = p;
		skipline();
		skiplinefeed();
		while(allowparse && isvalue())
			read_object(pd->metadata, pd->source, pd->key_count, 0, common_initialize);
	}
}

void bsreq::read(const char* url) {
	pushvalue push_context(context);
	auto push_p = p;
	p = log::read(url);
	if(p) {
		allowparse = true;
		p = log::skipwscr(p);
		parse();
		log::close();
	}
	p = push_p;
}