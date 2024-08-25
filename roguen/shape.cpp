#include "direction.h"
#include "shape.h"
#include "stringbuilder.h"
#include "imath.h"
#include "logparse.h"

using namespace log;

BSDATAC(shapei, 128)

static const char* shape_symbols = {"UX.1234567890 "};

point shapei::find(char sym) const {
	for(short y = 0; y < size.y; y++) {
		for(short x = 0; x < size.x; x++) {
			point m = {x, y};
			if((*this)[m] == sym)
				return m;
		}
	}
	return {-1000, -1000};
}

point shapei::translate(point c, point v, direction_s d) const {
	switch(d) {
	case North: return c.to(origin.x + v.x, origin.y + v.y);
	case South: return c.to(origin.x + v.x, -origin.y - v.y);
	case West: return c.to(-origin.y - v.y, v.x);
	case East: return c.to(v.y, -origin.x - v.x);
	default: return c;
	}
}

rect shapei::bounding(point c, direction_s d) const {
	point ul = translate(c, origin, d);
	point dr = translate(c, origin + size, d);
	auto x1 = imin(ul.x, dr.x);
	auto x2 = imax(ul.x, dr.x);
	auto y1 = imin(ul.y, dr.y);
	auto y2 = imax(ul.y, dr.y);
	return {x1, y1, x2, y2};
}

rect shapei::bounding(rect rc, direction_s d) const {
	auto r1 = bounding(point(0, 0), d);
	rc.x1 -= r1.x1;
	rc.y1 -= r1.y1;
	rc.x2 -= r1.x2;
	rc.y2 -= r1.y2;
	return rc;
}

static bool isallowed(char sym) {
	return zchr(shape_symbols, sym) != 0;
}

static const char* read_block(const char* p, shapei& e, stringbuilder& sb) {
	while(*p == '\n' || *p == '\r')
		p = skipcr(p);
	sb.clear();
	auto ps = sb.get();
	auto pb = p;
	e.size.x = 0;
	e.size.y = 0;
	e.origin.x = 0;
	e.origin.y = 0;
	if(!isallowed(*p))
		errorp(p, "Expected shape data");
	while(allowparse && *p && (isallowed(*p) || *p == '\n' || *p == '\r')) {
		if((*p == '\n') || (*p == '\r')) {
			if(e.size.x == 0)
				e.size.x = p - pb;
			else {
				auto n = p - pb;
				if(n != e.size.x)
					errorp(p, "Shape row '%2' must be %1i characters", e.size.x, ps);
			}
			while(*p == '\n' || *p == '\r')
				p = skipcr(p);
			pb = p;
			ps = sb.get();
			e.size.y++;
		} else
			sb.add(*p++);
	}
	size_t mr = e.size.x * e.size.y;
	size_t mn = sb.getmaximum();
	if(mr > mn)
		errorp(pb, "Shape size %1ix%2i is too big. Try make it smallest. Multiplied width and height of shape must be not greater that %3i.", e.size.x, e.size.y, mn);
	e.content = szdup(sb.begin());
	for(auto sym : "0123456789")
		e.points[sym - '0'] = e.find(sym);
	e.origin.x = -e.points[0].x;
	e.origin.y = -e.points[0].y;
	return skipspcr(p);
}

void shapei::read(const char* url) {
	auto p = log::read(url);
	if(!p)
		return;
	char temp[8192]; stringbuilder sb(temp);
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