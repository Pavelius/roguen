#include "archive.h"
#include "areapiece.h"
#include "boost.h"
#include "draw.h"
#include "draw_object.h"
#include "game.h"
#include "greatneed.h"
#include "creature.h"
#include "io_memory.h"

static const char* save_folder = "save";

static void capture_game_screen(draw::surface& dc) {
	dc.resize(640 - 130, 360, 32, true);
	auto push_canvas = draw::canvas;
	draw::canvas = &dc;
	if(area)
		draw::paintobjects();
	else
		draw::rectf();
	draw::canvas = push_canvas;
}

static void capture_game_screen_small(draw::surface& dc, int scale_x, int scale_y) {
	draw::surface dc_copy; capture_game_screen(dc_copy);
	dc.resize(dc_copy.width / 2, dc_copy.height / 2, dc_copy.bpp, true);
	dc.blit(0, 0, dc.width, dc.height, false, dc_copy, 0, 0, dc_copy.width, dc_copy.height);
}

void make_game_map_screenshoot() {
	draw::surface dc;
	capture_game_screen_small(dc, 3, 3);
	dc.write("images/screenshoots/00001.bmp", 0);
}

template<> void archive::set<draw::surface>(draw::surface& e) {
	if(writemode) {
		set(e.width);
		set(e.height);
		set(e.bits, e.scanline * e.height);
	} else {
		int b_width, b_height;
		set(b_width);
		set(b_height);
		e.resize(b_width, b_height, 32, true);
	}
}

template<> void archive::set<areapiece>(areapiece& e) {
	set(*static_cast<areaheadi*>(&e));
	set(*static_cast<areamap*>(&e));
	setc<roomi>(e.rooms);
	setc<itemground>(e.items);
}

static bool serial_game(const char* url, bool write_mode, bool head_only, draw::surface& dc) {
	io::file file(url, write_mode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, write_mode);
	if(!a.signature("SAV"))
		return false;
	if(!a.version(1, 0))
		return false;
	a.set(game);
	a.set(dc);
	if(head_only)
		return true;
	a.set(bsdata<boosti>::source);
	a.set(bsdata<creature>::source);
	a.set(bsdata<greatneed>::source);
	a.setc<areapiece>(bsdata<areapiece>::source);
	return true;
}

static void serial_game_name(const char* id, bool write_mode, bool head_only, draw::surface& dc) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("saves/%1.sav", id);
	serial_game(temp, write_mode, head_only, dc);
}

void gamei::write(const char* name) {
	draw::surface dc; capture_game_screen_small(dc, 3, 3);
	serial_game_name(name, true, false, dc);
}

void gamei::read(const char* name) {
	draw::surface dc; capture_game_screen_small(dc, 3, 3);
	serial_game_name(name, false, false, dc);
}

void gamei::writelog() {
	io::file file("logs.txt", StreamWrite | StreamText);
	if(!file)
		return;
	auto p = getlog();
	if(p)
		file << p;
}