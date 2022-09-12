#include "crt.h"
#include "draw.h"

using namespace draw;

static char text_params_data[4096];
int draw::tab_pixels = 0;
long draw::text_params[16];
static const char* text_start_string;
static int text_start_horiz;
static point maxcaret;

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

static void apply_line_feed(int x1, int dy) {
	if(maxcaret.x < caret.x)
		maxcaret.x = caret.x;
	caret.x = x1;
	caret.y += dy;
	if(maxcaret.y < caret.y)
		maxcaret.y = caret.y;
}

static const char* citate(const char* p, int x1, int x2, color new_fore, const sprite* new_font) {
	auto push_fore = draw::fore;
	auto push_font = draw::font;
	draw::fore = new_fore;
	draw::font = new_font;
	caret.x = x1;
	while(p[0]) {
		if(p[0] == '-' && p[1] == '#' && p[2] == '-' && (p[3] == 13 || p[3] == 10 || p[3]==0)) {
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
				auto h = metrics::icons->get(0).sy;
				image(caret.x + 1, caret.y + (font->ascend - h) / 2 + 1, metrics::icons, index, ImageNoOffset);
				w = metrics::icons->get(0).sx + 2;
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

static const char* text_block(const char* p, int original_x1, int original_x2);

static const char* parse_parameters(const char* p, const char*& id, unsigned& align) {
	memset(text_params, 0, sizeof(text_params));
	stringbuilder sb(text_params_data);
	auto count = 0; id = 0; align = AlignCenter;
	while(*p && !(*p == 13 || *p == 10)) {
		auto p1 = p;
		auto is_text = false;
		long value = 0;
		p = stringbuilder::read(p, value);
		if(p == p1) {
			is_text = true;
			value = (long)sb.get();
			if(p[0] == '$' && p[1] == '{')
				p = sb.psline(skipspcr(p + 2));
			else if(p[0] == '\"' || p[0] == '\'')
				p = sb.psstr(p + 1, p[0]);
			else
				p = sb.psidf(p);
			sb.addsz();
		}
		p = skipsp(p);
		if(p == p1)
			break;
		if(is_text) {
			auto pid = (const char*)value;
			if(!count) {
				if(equal(pid, "left")) {
					align = AlignLeftCenter;
					continue;
				} else if(equal(pid, "right")) {
					align = AlignRightCenter;
					continue;
				}
			}
			if(!id) {
				id = pid;
				continue;
			}
		}
		text_params[count++] = value;
	}
	return skipspcr(p);
}

static void execute_center() {
	auto name = (const char*)text_params[0];
	auto tips = (const char*)text_params[1];
}

static void execute_tab() {
	tab_pixels = text_params[0];
}

static void execute_command(const char* id) {
	if(equal(id, "center"))
		execute_center();
	else if(equal(id, "tab"))
		execute_tab();
}

static const char* parse_command(const char* p, int x1, int x2) {
	unsigned align; const char* id;
	p = parse_parameters(p, id, align);
	if(!id)
		return p;
	auto push_width = width;
	auto push_height = height;
	auto push_clipping = clipping;
	width = x2 - x1; height = 0;
	switch(align) {
	case AlignLeftCenter:
		execute_command(id);
		if(height) {
			auto y2 = caret.y + height;
			caret.x += width + metrics::border;
			p = text_block(p, caret.x, x2);
			if(caret.y < y2)
				caret.y = y2;
			apply_line_feed(x1, 0);
		}
		break;
	case AlignRightCenter:
		clipping.clear();
		execute_command(id);
		clipping = push_clipping;
		if(height && width) {
			auto w = width;
			caret.x = x2 - width;
			execute_command(id);
			caret.x = x1;
			auto y2 = caret.y + height;
			p = text_block(p, x1, x2 - w - metrics::border);
			if(caret.y < y2)
				caret.y = y2;
			apply_line_feed(x1, 0);
		}
		break;
	default:
		execute_command(id);
		if(height)
			caret.y += height + metrics::border;
		apply_line_feed(x1, 0);
		height = push_height;
		width = push_width;
		break;
	}
	clipping = push_clipping;
	return p;
}

static const char* text_block(const char* p, int x1, int x2) {
	while(p[0]) {
		caret.x = x1; width = x2 - x1;
		if(caret.y < clipping.y1) {
			text_start_string = p;
			text_start_horiz = caret.y - clipping.y1;
		}
		if(match(&p, "$end\n")) {
			p = skipspcr(p);
			break;
		} else if(match(&p, "#--")) // Header small
			p = textfln(skipsp(p), x1, x2, colors::h3, metrics::small);
		else if(match(&p, "###")) // Header 3
			p = textfln(skipsp(p), x1, x2, colors::h3, metrics::h3);
		else if(match(&p, "##")) // Header 2
			p = textfln(skipsp(p), x1, x2, colors::h2, metrics::h2);
		else if(match(&p, "#")) // Header 1
			p = textfln(skipsp(p), x1, x2, colors::h1, metrics::h1);
		else if(match(&p, "-#-")) // Citate
			p = citate(skipspcr(p), x1 + metrics::padding * 2, x2, colors::special.mix(colors::window, 192), metrics::font);
		else if(match(&p, "---")) { // Line
			p = skipspcr(p);
			auto push_x = caret.x;
			caret.y += 2;
			caret.x = x1 - metrics::border;
			auto push_fore = fore;
			fore = colors::border;
			line(x2 + metrics::border, caret.y);
			fore = push_fore;
			caret.x = push_x;
			caret.y += 2;
		} else if(match(&p, "* ")) {
			// —писок
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
		} else if(match(&p, "$"))
			p = parse_command(p, x1, x2);
		else
			p = textfln(p, x1, x2, fore, font);
	}
	return p;
}

void draw::textf(const char* p) {
	auto push_width = width;
	auto push_height = height;
	auto push_tab = tab_pixels;
	maxcaret.clear();
	text_start_string = 0;
	text_start_horiz = 0;
	auto x0 = caret.x; auto y0 = caret.y;
	p = text_block(p, x0, x0 + width);
	maxcaret.x -= x0; maxcaret.y -= y0;
	tab_pixels = push_tab;
	width = push_width;
	height = push_height;
}

void draw::textfs(const char* string) {
	auto push_caret = caret;
	auto push_clipping = clipping;
	clipping.clear(); caret = {};
	textf(string);
	clipping = push_clipping;
	caret = push_caret;
	width = maxcaret.x;
	height = maxcaret.y;
}