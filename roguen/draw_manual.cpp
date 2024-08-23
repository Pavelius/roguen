#include "crt.h"
#include "draw.h"
#include "dialog.h"
#include "pushvalue.h"
#include "shortcut.h"

using namespace draw;

static const char* header;
static const char* content;
static int content_origin, content_maximum, content_text_origin;

bool button(unsigned key, int format_width);

static void paint_header(const char* format) {
	auto push_font = font;
	font = metrics::h2;
	textcj(format);
	caret.y += texth();
	font = push_font;
}

static void paint_canter(const char* format, fnoutput proc) {
	auto push_caret = caret;
	caret.x += width / 2;
	proc(format);
	caret.x = push_caret.x;
}

static void bottom_border() {
	pushvalue push_fore(draw::fore, colors::border);
	auto push_caret = caret;
	caret.y += metrics::padding;
	caret.x -= metrics::padding;
	line(caret.x + width + metrics::padding * 2, caret.y);
	caret = push_caret;
	caret.y += metrics::padding * 2;
}

static void manual_back() {
}

static void manual_page_up() {
}

static void manual_page_down() {
}

static void manual_home() {
	content_origin = 0;
	content_text_origin = -1;
}

static void manual_end() {
}

static void manual_up() {
	content_origin -= texth();
	if(content_origin < 0)
		content_origin = 0;
	content_text_origin = -1;
}

static void manual_down() {
	content_origin += texth();
	content_text_origin = -1;
}

static void button(unsigned key, fnevent proc) {
	if(button(key, -1))
		execute(proc);
}

static void button_line(const slice<shortcuti>& source) {
	auto push_caret = draw::caret;
	draw::caret.y = getheight() - texth() - metrics::padding;
	for(auto& e : source)
		button(e.key, e.proc);
	draw::caret = push_caret;
	height = getheight() - texth() - metrics::padding * 2 - push_caret.y;
}

static void clipped_string(const char* format, int& origin, int& format_origin, int& maximum) {
	if(!format || !format[0])
		return;
	rectpush push;
	if(format_origin < 0) {
		textfs(format);
		maximum = height;
		height = push.height;
		width = push.width;
		caret = push.caret;
		if(format_origin == -2) {
			if(maximum > height)
				caret.y -= maximum - height;
		}
		auto push_clip = clipping; setclipall();
		textf(format, origin, format_origin);
		clipping = push_clip;
	} else {
		auto push_clip = clipping; setclipall();
		caret.y -= origin;
		textf(format, origin, format_origin);
		clipping = push_clip;
	}
}

void show_manual() {
	static shortcuti keys[] = {
		{KeyEscape, buttoncancel},
		{KeyBackspace, manual_back},
		{KeyUp, manual_up},
		{KeyDown, manual_down},
		{KeyHome, manual_home},
		{KeyEnd, manual_end},
		{KeyPageUp, manual_page_up},
		{KeyPageDown, manual_page_down},
	};
	if(header) {
		paint_canter(header, paint_header);
		bottom_border();
	}
	button_line(keys);
	clipped_string(content, content_origin, content_text_origin, content_maximum);
}

void open_manual(const char* manual_header, const char* manual_content) {
	auto push_header = header; header = manual_header;
	auto push_content = content; content = manual_content;
	content_text_origin = -2;
	open_dialog("ShowManual");
	content = push_content;
	header = push_header;
}

