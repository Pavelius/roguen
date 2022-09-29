#include "shape.h"
#include "stringbuilder.h"
#include "logparse.h"

using namespace log;

BSDATAC(shapei, 128)

static const char* shape_symbols = {"UX.1234567890\r\n"};

pointm shapei::find(char sym) const {
	auto m = size.maximum();
	for(size_t i = 0; i < m; i++) {
		if(content[i] == sym)
			return i2m(i);
	}
	return pointm();
}

pointm shapei::translate(pointm c, pointm v, direction_s d) const {
	switch(d) {
	case North: return pointm(c.x + v.x, c.y + v.y);
	case South: return pointm(c.x + v.x, c.y - v.y);
	case West: return pointm(c.x - v.y, c.y + v.x);
	case East: return pointm(c.x + v.y, c.y - v.x);
	default: return pointm();
	}
}

static bool isallowed(char sym) {
	return zchr(shape_symbols, sym) != 0;
}

static const char* read_block(const char* p, shapei& e, stringbuilder& sb) {
	sb.clear();
	p = skipspcr(p);
	auto pb = p;
	e.size.x = 0;
	e.size.y = 1;
	e.origin.x = 0;
	e.origin.y = 0;
	if(!isallowed(*p))
		error(p, "Expected shape data");
	while(allowparse && *p && isallowed(*p)) {
		if((*p == '\n') || (*p == '\r')) {
			if(e.size.x == 0)
				e.size.x = p - pb;
			else {
				auto n = p - pb;
				if(n != e.size.x)
					error(p, "Shape row must be %1i characters", e.size.x);
			}
			p = skipspcr(p);
			pb = p;
			e.size.y++;
		} else
			sb.add(*p++);
	}
	size_t mr = e.size.maximum();
	size_t mn = sb.maximum();
	if(mr > mn)
		error(pb, "Shape size %1ix%2i is too big. Try make it smallest. Multiplied sized of shape must be not greater that %3i.", e.size.x, e.size.y, mn);
	e.content = szdup(sb.begin());
	for(auto sym : "0123456789")
		e.points[sym - '0'] = e.find(sym);
	e.origin.x = -e.points[0].x;
	e.origin.y = -e.points[0].y;
	return p;
}

void shapei::read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[4096]; stringbuilder sb(temp);
	allowparse = true;
	while(allowparse && *p) {
		if(!checksym(p, '#'))
			break;
		auto ps = bsdata<shapei>::add();
		p = readidn(p + 1, sb);
		ps->id = szdup(temp);
		if(!checksym(p, '\n'))
			break;
		p = read_block(p, *ps, sb);
	}
	log::close();
}