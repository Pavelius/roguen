#include "archive.h"
#include "boost.h"
#include "draw.h"
#include "draw_object.h"
#include "main.h"

static vector<creature> saved_creatures;
static const char* save_folder = "save";

static void remove_summoned() {
	for(auto& e : bsdata<creature>()) {
		if(!e.isvalid())
			continue;
		if(e.is(Summoned)) {
			e.unlink();
			e.clear();
		}
	}
}

static void save_monsters() {
	saved_creatures.clear();
	for(auto& e : bsdata<creature>()) {
		if(!e.isvalid())
			continue;
		if(e.is(Local)) {
			e.unlink();
			saved_creatures.add(e);
			e.clear();
		}
	}
}

static void restore_monsters() {
	for(auto& e : saved_creatures) {
		auto p = bsdata<creature>::addz();
		if(p)
			*p = e;
	}
	saved_creatures.clear();
}

static void update_ui() {
	bsdata<draw::object>::source.clear();
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid())
			e.fixappear();
	}
}

static void set_allies_position(geoposition ogp, geoposition ngp, point m) {
	if(!area.isvalid(m))
		return;
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos == ogp && e.is(Ally)) {
			e.place(m);
			e.worldpos = ngp;
		}
	}
}

static void after_serial_game() {
	restore_monsters();
	update_ui();
}

static void before_serial_game() {
	save_monsters();
}

static void serial_game(bool write_mode) {
	io::file file("save/game.sav", write_mode ? StreamWrite : StreamRead);
	if(!file)
		return;
	archive a(file, write_mode);
	a.set(game);
	a.set(bsdata<boosti>::source);
	a.set(bsdata<creature>::source);
	a.set(bsdata<roomi>::source);
}

static bool serial_area(const char* url, bool write_mode) {
	io::file file(url, write_mode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, write_mode);
	a.set(areahead);
	a.set(area);
	a.set(saved_creatures);
	a.set(bsdata<itemground>::source);
	return true;
}

static void serial_area(bool write_mode) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%4/AR%1.2h%2.2h%3.2h.sav", game.position.y, game.position.x, game.level, save_folder);
	if(!write_mode) {
		if(!serial_area(temp, false)) {
			saved_creatures.clear();
			game.createarea();
		}
	} else
		serial_area(temp, write_mode);
}

void gamei::enter(point m, int level, feature_s feature, direction_s appear_side) {
	before_serial_game();
	remove_summoned();
	geoposition old_game = game;
	if(game.isvalid(position)) {
		serial_game(true);
		serial_area(true);
	}
	this->position = m;
	this->level = level;
	serial_area(false);
	point start = {-1000, -1000};
	if(feature)
		start = area.find(feature);
	if(!game.isvalid(start))
		start = area.bordered(round(appear_side, South));
	set_allies_position(old_game, game, start);
	after_serial_game();
	draw::setnext(game.play);
}

static void cleanup_saves() {
	char temp[260]; stringbuilder sb(temp);
	for(io::file::find file(save_folder); file; file.next()) {
		auto pn = file.name();
		if(pn[0] == '.')
			continue;
		sb.clear();
		sb.add("%1/%2", save_folder, pn);
		io::file::remove(temp);
	}
}

void gamei::newgame() {
	cleanup_saves();
	game.randomworld();
	game.enter(start_village, 0, StairsDown, NorthEast);
}

void gamei::writelog() {
	io::file file("logs.txt", StreamWrite | StreamText);
	if(!file)
		return;
	auto p = actable::getlog();
	if(p)
		file << p;
}