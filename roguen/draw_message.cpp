#include "crt.h"
#include "draw.h"
#include "screenshoot.h"

using namespace draw;

static surface			dialog_bitmap;
static char				dialog_text[512];

static void center_bitmap(const surface& bm) {
	auto push_width = width;
	auto push_height = height;
	auto push_x = caret.x;
	width = bm.width - 1;
	height = bm.height;
	caret.x = (getwidth() - width) / 2;
	canvas->blit(caret.x, caret.y, bm.width, bm.height, 0, bm, 0, 0);
	caret.y += height + texth();
	caret.x = push_x;
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
	caret.x = 20;
	caret.y = 40;
	width = getwidth() - 20 * 2;
	height = texth() * 6;
	center_bitmap(dialog_bitmap);
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
	while(*id==*pb) {
		id++; pb++;
	}
	if(*id==0) {
		p = skipsp(pb);
		return true;
	}
	return false;
}

static const char* read_url(const char* p, stringbuilder& sb) {
	sb.clear();
	while(*p && (ischa(*p) || isnum(*p) || *p == '_' || *p == '/' || *p == '\\' || *p=='.'))
		sb.addch(*p++);
	return p;
}

static const char* read_idf(const char* p, stringbuilder& sb) {
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
	while(*p) {
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

static void add_format(const char* format, const char* format_param) {
	format = parse_format(format);
	stringbuilder sb(dialog_text);
	sb.addv(format, format_param);
}

void dialog_message(const char* format) {
	if(!format)
		return;
	add_format(format, 0);
	if(!dialog_bitmap)
		return;
	//rectpush push;
	//caret.clear();
	//width = getwidth();
	//height = getheight();
	//fillwindow();
	dialog_modal(dialog_scene);
}