#include "draw.h"
#include "pushvalue.h"
#include "shortcut.h"
#include "slice.h"

using namespace draw;

static const char* header;
static const char* content;
static int content_origin, content_maximum, content_page, cash_string;

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

static bool is_not_home() {
	return content_origin > 0;
}

static bool is_not_end() {
	return content_origin < content_maximum - content_page;
}

static void manual_page_up() {
	content_origin -= content_page;
	cash_string = -1;
}

static void manual_page_down() {
	content_origin += content_page;
	cash_string = -1;
}

static void manual_home() {
	content_origin = 0;
	cash_string = -1;
}

static void manual_end() {
	content_origin = 256 * 256 * 256;
	cash_string = -1;
}

static void manual_up() {
	content_origin -= texth();
	cash_string = -1;
}

static void manual_down() {
	content_origin += texth();
	cash_string = -1;
}

static void button(unsigned key, fnevent proc) {
	if(button(key, -1))
		execute(proc);
}

static void button_line(const slice<shortcuti>& source) {
	auto push_caret = draw::caret;
	draw::caret.y = getheight() - texth() - metrics::padding;
	for(auto& e : source) {
		if(e.visible && !e.visible())
			continue;
		button(e.key, e.proc);
	}
	draw::caret = push_caret;
	height = getheight() - texth() - metrics::padding * 2 - push_caret.y;
}

static void clipped_string(const char* format, int& origin, int& cash_text_offset, int& maximum) {
	if(!format || !format[0])
		return;
}

void show_manual() {
	static int cash_origin;
	static shortcuti keys[] = {
		{KeyEscape, buttoncancel},
		{KeyBackspace, manual_back},
		{KeyUp, manual_up, is_not_home},
		{KeyDown, manual_down, is_not_end},
		{KeyHome, manual_home, is_not_home},
		{KeyEnd, manual_end, is_not_end},
		{KeyPageUp, manual_page_up, is_not_home},
		{KeyPageDown, manual_page_down, is_not_end},
	};
	if(header) {
		paint_canter(header, paint_header);
		bottom_border();
	}
	button_line(keys);
	auto push_clip = clipping; setclipall();
	content_page = height;
	textf(content, content_origin, content_maximum, cash_origin, cash_string);
	clipping = push_clip;
}

static void scene_dialog(fnevent proc) {
	while(ismodal()) {
		fillform();
		setoffset(metrics::padding, metrics::padding);
		proc();
		domodal();
	}
}

void open_manual(const char* manual_header, const char* manual_content) {
	auto push_header = header; header = manual_header;
	auto push_content = content; content = manual_content;
	content_maximum = 0;
	manual_end();
	scene_dialog(show_manual);
	content = push_content;
	header = push_header;
}