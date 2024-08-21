#include "draw.h"

/// <summary>
/// Format like this lines
/// #+ f1 c0x12312323 b0x123923902 aBC
/// #-
/// </summary>

using namespace draw;

static const char*	p;
static rect			box;
static color		back;
static unsigned		flags;

struct pushtextform : rectpush {
	color			fore, oback;
	const sprite*	font;
	int				tab_pixels;
	rect			obox;
	unsigned		oflags;
	pushtextform() : rectpush(), fore(draw::fore), oback(back), font(draw::font), tab_pixels(draw::tab_pixels),
		obox(box), oflags(flags) {
		box.set(caret.x, caret.y, caret.x + width, caret.y + height);
	}
	~pushtextform() {
		draw::font = font;
		draw::fore = fore;
		draw::tab_pixels = tab_pixels;
		back = oback;
		box = obox;
		flags = oflags;
	}
};

static void endline() {
	while(p[0] && *p != 10 && *p != 13)
		p++;
}

static void skiplinefeed() {
	while(p[0] && (*p == 10 || *p == 13))
		p++;
}

static void skipws() {
	while(p[0] && (*p == 20 || *p == 9))
		p++;
}

static void skipws(int n) {
	p += n;
	skipws();
}

static bool header(char symbol) {
	if(p[0] == '#' && p[1] == symbol && p[2] == ' ') {
		p += 3;
		return true;
	}
	return false;
}

static void parse_param(char symbol, int& value) {
	if(p[0] != symbol)
		return;
	p = stringbuilder::read(p, value);
	skipws();
}

static void parse_param(char symbol, int& value, int proportial) {
	auto p_push = p;
	parse_param(symbol, value);
	if(p_push == p)
		return;
	if(value < 0)
		value = proportial * (-value) / 100;
}

static void cell() {

}

static void parse_font(char symbol, const sprite* v) {
	if(p[0] == 'f' && p[1] == '1') {
		font = v;
		skipws(2);
	}
}

static void parse_params() {
	while(p[0]) {
		auto p1 = p;
		parse_param('w', width, box.x2 - box.x1);
		if(p1 != p)
			continue;
		parse_param('h', height, box.y2 - box.y1);
		if(p1 != p)
			continue;
		parse_param('t', tab_pixels, box.x2 - box.x1);
		if(p1 != p)
			continue;
		parse_font('1', metrics::h1);
		if(p1 != p)
			continue;
		parse_font('2', metrics::h2);
		if(p1 != p)
			continue;
		parse_font('3', metrics::h3);
		if(p1 != p)
			continue;
		parse_font('t', metrics::font);
		if(p1 != p)
			continue;
		parse_font('s', metrics::small);
		if(p1 != p)
			continue;
		break;
	}
	endline();
	skiplinefeed();
}

static void block() {
	pushtextform push;
	parse_params();
	while(p[0]) {
		if(header('+')) // Header small
			block();
		else if(header('-')) {
			endline();
			skiplinefeed();
			break;
		} else
			cell();
	}
}

void textform(const char* format) {
	auto push_p = p; p = format;
	block();
	p = push_p;
}