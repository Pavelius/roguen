#include "crt.h"
#include "draw.h"
#include "screenshoot.h"

using namespace draw;

static surface	dialog_bitmap;
static char		dialog_text[128];

static void center_bitmap(const surface& bm) {
	auto push_width = width;
	auto push_height = height;
	width = bm.width - 1;
	height = bm.height;
	caret.x = (getwidth() - width) / 2;
	caret.y = (getheight() - height) / 2 - texth() * 2;
	canvas->blit(caret.x, caret.y, bm.width, bm.height, 0, bm, 0, 0);
	caret.y += height + texth();
	width = push_width;
	height = push_height;
}

static void dialog_scene() {
	rectpush push;
	caret.clear();
	width = getwidth();
	height = getheight();
	fillwindow();
	auto push_font = font;
	font = metrics::h2;
	center_bitmap(dialog_bitmap);
	caret.x = 20;
	width = getwidth() - 20 * 2;
	height = texth() * 6;
	texta(dialog_text, AlignCenter);
	font = push_font;
}

static void dialog_modal(fnevent proc) {
	screenshoot::fade(proc, 1000);
	while(ismodal()) {
		domodal();
		switch(hot.key) {
		case KeyEnter:
		case KeyEscape:
		case KeySpace:
		case MouseLeft:
			breakmodal(1);
			break;
		}
	}
}

static bool command(const char*& p, const char* id) {
	auto pb = p;
	while(*id == *pb) {
		id++; pb++;
	}
	if(*id == 0) {
		p = skipsp(pb);
		return true;
	}
	return false;
}

static const char* read_url(const char* p, stringbuilder& sb) {
	sb.clear();
	while(*p && (ischa(*p) || isnum(*p) || *p == '_' || *p == '/' || *p == '\\' || *p == '.'))
		sb.addch(*p++);
	return p;
}

static const char* read_idf(const char* p, stringbuilder& sb) {
	sb.clear();
	while(*p && (ischa(*p) || isnum(*p) || *p == '_'))
		sb.addch(*p);
	return p;
}

static const char* skipncr(const char* p) {
	while(*p && !(*p == 10 || *p == 13))
		p++;
	return skipspcr(p);
}

static const char* parse_format(const char* p) {
	char temp[512]; stringbuilder sb(temp);
	while(*p == '/') {
		if(command(p, "/IMAGE")) {
			p = read_url(p, sb);
			if(temp[0])
				dialog_bitmap.read(temp);
		} else
			break;
		p = skipncr(p);
	}
	return p;
}

static const char* parse_string(const char* format) {
	auto pe = skipncr(format);
	auto len = pe - format;
	if(len > sizeof(dialog_text) - 16)
		len = sizeof(dialog_text) - 16;
	memcpy(dialog_text, format, len);
	dialog_text[len] = 0;
	return format + len;
}

static const char* add_format(const char* format) {
	format = parse_format(format);
	return parse_string(format);
}

void dialog_message(const char* format) {
	if(!format)
		return;
	while(format[0]) {
		format = add_format(format);
		if(!dialog_bitmap)
			return;
		dialog_modal(dialog_scene);
	}
}