#include "crt.h"
#include "draw.h"

///////////////////////////////////////////////////////////
// RICH COMMAND FORMAT EXAMPLE
//
// $left 64 image Bitmap 0 'art/images'
// $right 64 tips !-'$centered 'Central Header' Some tips with fully featured tips'-! image AnotherBitmap 0 'art/images'

using namespace draw;

int draw::tab_pixels = 0;
static const char* text_start_string;
static int text_start_horiz;
static point maxcaret;

static void apply_line_feed(int x1, int dy) {
	if(maxcaret.x < caret.x)
		maxcaret.x = caret.x;
	caret.x = x1;
	caret.y += dy;
	if(maxcaret.y < caret.y)
		maxcaret.y = caret.y;
}

static bool equaln(const char*& p, const char* name) {
	int n = zlen(name);
	if(memcmp(p, name, n) != 0)
		return false;
	switch(p[n]) {
	case '\n': case ' ': case '\t':
		p = skipsp(p + n);
		return true;
	default:
		return false;
	}
}

static const char* read_special_string(const char* p, stringbuilder& sb) {
	while(*p) {
		if(p[0] == '-' && p[1] == '!' && p[2] == '\'')
			p = read_special_string(p + 3, sb);
		else if(p[0] == '\'' && p[1] == '-' && p[2] == '!') {
			p = skipsp(p + 3);
			break;
		}
	}
	return p;
}

static const char* getparam(const char*& p, stringbuilder& sb) {
	auto pb = sb.get();
	if(p[0] == '-' && p[1] == '!' && p[2] == '\'')
		p = read_special_string(p + 3, sb);
	else if(p[0] == '\'')
		p = sb.psstr(p + 1, '\'');
	else if(p[0] == '\"')
		p = sb.psstr(p + 1, '\"');
	else if(ischa(*p))
		p = sb.psidf(p);
	sb.addsz();
	p = skipsp(p);
	return pb;
}

static int getparam(const char*& p) {
	if(isnum(p[0]) || (p[0] == '-' && isnum(p[1]))) {
		int result = 0;
		p = stringbuilder::read(p, result);
		p = skipsp(p);
		return result;
	}
	return 0;
}

static const char* skip_line(const char* p) {
	for(; *p && p[0] != 10 && p[0] != 13; p++);
	return skipcr(p);
}

static void border_color(fnevent proc) {
	auto push_fore = fore;
	fore = colors::border;
	proc();
	fore = push_fore;
}

static void paint_image(const char* name, int id, const char* folder) {
	//auto ps = draw::gres(name, folder);
	//if(!ps)
	//	return;
	//width = ps->get(id).sx;
	//height = ps->get(id).sy;
	//if(!clipping) // Perfomance optimiation
	//	return;
	//image(ps, id, ImageNoOffset);
	//border_color(rectb);
}

static void paint_centered(const char* name) {
	auto push_caret = caret;
	auto push_width = width;
	auto push_maxcaret = maxcaret;
	auto push_text_string = text_start_string;
	auto push_text_origin = text_start_horiz;
	textfs(name);
	caret.x += (push_width - width) / 2;
	textf(name);
	text_start_horiz = push_text_origin;
	text_start_string = push_text_string;
	maxcaret = push_maxcaret;
	width = push_width;
	caret = push_caret;
}

static const char* parse_widget_command(const char* p) {
	auto next_line = true;
	auto push_caret = caret;
	auto push_width = width;
	auto push_height = height;
	const char* tips = 0;
	char temp[8192]; stringbuilder sb(temp);
	height = 0;
	while(*p) {
		if(equaln(p, "left")) {
			auto widget_width = getparam(p);
			if(widget_width) {
				width = widget_width;
				push_width -= widget_width;
				push_caret.x += widget_width;
				next_line = false;
			}
			continue;
		} else if(equaln(p, "right")) {
			auto widget_width = getparam(p);
			if(widget_width) {
				width = widget_width;
				caret.x = caret.x + push_width - width;
				push_width -= widget_width;
				next_line = false;
			}
			continue;
		} else if(equaln(p, "tips")) {
			tips = getparam(p, sb);
			continue;
		} else if(equaln(p, "image")) {
			auto name = getparam(p, sb);
			auto id = getparam(p);
			auto folder = getparam(p, sb);
			paint_image(name, id, folder);
		} else if(equaln(p, "centered")) {
			auto name = getparam(p, sb);
			paint_centered(name);
		} else if(equaln(p, "tab"))
			tab_pixels = getparam(p);
		if(tips && ishilite())
			tips_sb.add(tips);
		p = skip_line(p);
		break;
	}
	if(caret.x + width > maxcaret.x)
		maxcaret.x = caret.x + width;
	if(caret.y + height > maxcaret.y)
		maxcaret.y = caret.y + height;
	caret = push_caret;
	width = push_width;
	if(next_line) {
		caret.y += height + metrics::padding;
		if(caret.y > maxcaret.y)
			maxcaret.y = caret.y;
	}
	height = push_height;
	return p;
}

static bool match(const char** string, const char* name) {
	int n = zlen(name);
	if(memcmp(*string, name, n) != 0)
		return false;
	(*string) += n;
	return true;
}

static const char* glink(const char* p, char* result, unsigned result_maximum) {
	result[0] = 0;
	if(p[0] == '(' && p[1] == '{') {
		p = skipspcr(p + 2);
		auto ps = result;
		auto pe = ps + result_maximum;
		while(p[0]) {
			if(p[0] == '}' && p[1] == ')') {
				p = skipspcr(p + 2);
				break;
			}
			if(ps < pe)
				*ps++ = *p;
			p++;
		}
		*ps++ = 0;
	} else if(*p == '\"') {
		auto sym = *p++;
		stringbuilder sb(result, result + result_maximum);
		p = sb.psstr(p, sym);
	} else if(*p == '(') {
		p = skipspcr(p + 1);
		auto ps = result;
		auto pe = ps + result_maximum;
		while(*p && *p != ')') {
			if(ps < pe)
				*ps++ = *p;
			p++;
		}
		*ps++ = 0;
		if(*p == ')')
			p++;
	}
	return p;
}

static int gettabwidth() {
	return tab_pixels ? tab_pixels : textw(' ') * 4;
}

static const char* textspc(const char* p, int x1) {
	int tb;
	while(true) {
		switch(p[0]) {
		case ' ':
			p++;
			caret.x += draw::textw(' ');
			continue;
		case '\t':
			p++;
			tb = gettabwidth();
			if(tb < 0)
				caret.x = x1 + width + tb;
			else
				caret.x = x1 + ((caret.x - x1 + tb) / tb) * tb;
			continue;
		}
		break;
	}
	return p;
}

static const char* wholeline(const char* p) {
	while(*p && *p != 10 && *p != 13)
		p++;
	return p;
}

static const char* word(const char* text) {
	if(text[0] == ':')
		return text + 1;
	while(((unsigned char)*text) > 0x20 && *text != '*' && *text != '[' && *text != ']' && *text != ':')
		text++;
	return text;
}

static const char* citate(const char* p, int x1, int x2, color new_fore, const sprite* new_font) {
	auto push_fore = draw::fore;
	auto push_font = draw::font;
	draw::fore = new_fore;
	draw::font = new_font;
	caret.x = x1;
	while(p[0]) {
		if(p[0] == '-' && p[1] == '#' && p[2] == '-' && (p[3] == 13 || p[3] == 10 || p[3] == 0)) {
			p = skipspcr(p + 3);
			break;
		}
		auto p2 = skipspcr(wholeline(p));
		auto w = textw(p, p2 - p);
		text(p, p2 - p);
		p = p2;
		apply_line_feed(x1, texth());
	}
	apply_line_feed(caret.x, 0);
	draw::fore = push_fore;
	draw::font = push_font;
	return p;
}

static const char* textfln(const char* p, int x1, int x2, color new_fore, const sprite* new_font) {
	auto push_fore = fore;
	auto push_font = font;
	char temp[4096]; temp[0] = 0;
	unsigned flags = 0;
	fore = new_fore;
	font = new_font;
	while(true) {
		if(p[0] == '*' && p[1] == '*') {
			p += 2;
			if(flags & TextBold)
				flags &= ~TextBold;
			else
				flags |= TextBold;
			continue;
		} else if(p[0] == '*') {
			p++;
			if(flags & TextItalic)
				flags &= ~TextItalic;
			else {
				if((flags & TextItalic) == 0)
					caret.x += texth() / 3;
				flags |= TextItalic;
			}
			continue;
		} else if(p[0] == '[' && p[1] == '[')
			p++;
		else if(p[0] == ']' && p[1] == ']')
			p++;
		else if(p[0] == '[') {
			p++;
			switch(*p) {
			case '~':
				p++;
				fore = colors::text.mix(colors::window, 64);
				break;
			case '+':
				p++;
				fore = colors::green;
				break;
			case '-':
				p++;
				fore = colors::red;
				break;
			case '!':
				p++;
				fore = colors::h3;
				break;
			case '#':
				p++;
				flags |= TextUscope;
				fore = colors::special;
				break;
			default:
				fore = colors::special;
				break;
			}
			p = glink(p, temp, sizeof(temp) / sizeof(temp[0]) - 1);
		} else if(p[0] == ']') {
			p++; temp[0] = 0;
			flags &= ~TextUscope;
			fore = new_fore;
		}
		// ќбработаем пробелы и табул€цию
		p = textspc(p, x1);
		auto w = 0;
		if(p[0] == ':' && p[1] >= '0' && p[1] <= '9') {
			auto index = 0;
			p = stringbuilder::read(p + 1, index);
			if(*p == ':')
				p++;
			if(metrics::icons) {
				w = metrics::icons->get(index).sx;
				image(caret.x, caret.y + (font->height + 1) / 2, metrics::icons, index, 0);
			}
		} else {
			const char* p2 = word(p);
			w = textw(p, p2 - p);
			if(caret.x + w > x2)
				apply_line_feed(x1, texth());
			text(p, p2 - p, flags);
			p = p2;
		}
		if(temp[0] && ishilite({caret.x, caret.y, caret.x + w, caret.y + texth()})) {
			if(!tips_sb)
				tips_sb.addv(temp, 0);
		}
		caret.x += w;
		p = textspc(p, x1);
		if(p[0] == 0 || p[0] == 10 || p[0] == 13) {
			p = skipcr(p);
			apply_line_feed(x1, texth());
			break;
		}
	}
	apply_line_feed(caret.x, 0);
	fore = push_fore;
	font = push_font;
	return p;
}

static void separator(int x2) {
	auto push_x = caret.x;
	caret.y += 2;
	caret.x -= metrics::border;
	auto push_fore = fore;
	fore = colors::border;
	line(x2 + metrics::border - 1, caret.y);
	fore = push_fore;
	caret.x = push_x;
	caret.y += 2;
}

static const char* bullet_list(const char* p, int x2) {
	auto dx = texth() / 2;
	auto rd = texth() / 6;
	auto push_caret = caret;
	caret.x += dx + 2;
	caret.y += dx;
	circlef(rd);
	circle(rd);
	caret = push_caret;
	caret.x += texth();
	p = textfln(p, caret.x, x2, fore, font);
	caret.x = push_caret.x;
	return p;
}

void draw::textf(const char* p) {
	auto push_width = width;
	auto push_height = height;
	auto push_tab = tab_pixels;
	maxcaret.clear();
	text_start_string = 0;
	text_start_horiz = 0;
	int x1 = caret.x, y1 = caret.y, x2 = caret.x + width;
	while(p[0]) {
		if(caret.y < clipping.y1) {
			text_start_string = p;
			text_start_horiz = caret.y - clipping.y1;
		}
		if(match(&p, "#--")) // Header small
			p = textfln(skipsp(p), caret.x, x2, colors::h3, metrics::small);
		else if(match(&p, "###")) // Header 3
			p = textfln(skipsp(p), caret.x, x2, colors::h3, metrics::h3);
		else if(match(&p, "##")) // Header 2
			p = textfln(skipsp(p), caret.x, x2, colors::h2, metrics::h2);
		else if(match(&p, "#")) // Header 1
			p = textfln(skipsp(p), caret.x, x2, colors::h1, metrics::h1);
		else if(match(&p, "-#-")) // Citate
			p = citate(skipspcr(p), caret.x + metrics::padding * 2, x2, colors::special.mix(colors::window, 192), metrics::font);
		else if(match(&p, "---")) { // Line
			p = skipspcr(p);
			separator(x2);
		} else if(match(&p, "* "))
			p = bullet_list(p, x2);
		else if(match(&p, "$"))
			p = parse_widget_command(p);
		else
			p = textfln(p, caret.x, x2, fore, font);
	}
	maxcaret.x -= x1; maxcaret.y -= y1;
	tab_pixels = push_tab;
	width = push_width;
	height = push_height;
}

void draw::textfs(const char* string, int& cashe_string, int& cashe_origin) {
	auto push_caret = caret;
	auto push_clipping = clipping;
	clipping.clear(); caret = {};
	textf(string);
	cashe_string = text_start_string - string;
	cashe_origin = text_start_horiz;
	clipping = push_clipping;
	caret = push_caret;
}

void draw::textfs(const char* string) {
	auto push_caret = caret;
	auto push_clipping = clipping;
	auto push_maxcaret = maxcaret;
	clipping.clear(); caret = {};
	textf(string);
	clipping = push_clipping;
	caret = push_caret;
	width = maxcaret.x;
	height = maxcaret.y;
	maxcaret = push_maxcaret;
}