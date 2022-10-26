#include "archive.h"
#include "draw.h"
#include "draw_object.h"
#include "main.h"

static adat<creature, 16> allies;
static creature* allies_reference[16];
static adat<boosti, 32> boost;
static const char* save_folder = "save";

void create_area(variant tile);

static short unsigned getindex(const creature* v) {
	for(auto& e : allies) {
		if(&e == v)
			return allies.indexof(&e);
	}
	return 0xFFFF;
}

static void after_read_allies(point m) {
	memset(allies_reference, 0, sizeof(allies_reference));
	for(auto& e : allies) {
		auto p = bsdata<creature>::add();
		if(!p)
			continue;
		*p = e;
		allies_reference[allies.indexof(&e)] = p;
		if(area.isvalid(m))
			p->place(m);
	}
	if(game.player_id != 0xFFFF)
		player = allies_reference[game.player_id];
}

static void before_write_allies() {
	game.player_id = 0xFFFF;
	allies.clear();
	for(auto& e : bsdata<creature>()) {
		if(!e.is(Ally))
			continue;
		if(allies.getcount() == allies.getmaximum())
			e.remove(Ally);
		else {
			allies.add(e);
			if(&e == player)
				game.player_id = allies.getcount() - 1;
			e.unlink();
			e.clear();
		}
	}
}

static void update_creatures() {
	for(auto& e : bsdata<creature>())
		if(e)
			e.fixappear();
}

static void after_serial_game(point start) {
	after_read_allies(start);
	bsdata<draw::object>::source.clear();
	update_creatures();
}

static void before_serial_game() {
	before_write_allies();
}

static void serial_game(bool write_mode) {
	io::file file("save/game.sav", write_mode ? StreamWrite : StreamRead);
	if(!file)
		return;
	archive a(file, write_mode);
	a.set(game);
	a.set(boost);
	a.set(allies);
}

static bool serial_area(const char* url, bool write_mode) {
	io::file file(url, write_mode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, write_mode);
	a.set(loc);
	a.set(area);
	a.set(bsdata<itemground>::source);
	a.set(bsdata<creature>::source);
	a.set(bsdata<boosti>::source);
	a.set(bsdata<roomi>::source);
	return true;
}

static void serial_area(bool write_mode) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("%4/AR%1.2h%2.2h%3.2h.sav", game.position.y, game.position.x, game.level, save_folder);
	if(!write_mode) {
		if(!serial_area(temp, false)) {
			game.createarea();
			if(!serial_area(temp, false))
				return;
		}
	} else
		serial_area(temp, write_mode);
}

void gamei::write() {
	if(!position)
		return;
	before_serial_game();
	serial_game(true);
	serial_area(true);
}

void gamei::read() {
	serial_game(false);
	serial_area(false);
	after_serial_game({-1000, -1000});
}

void gamei::enter(point m, int level, feature_s feature, direction_s appear_side) {
	before_serial_game();
	if(position) {
		serial_game(true);
		serial_area(true);
	}
	this->position = m;
	this->level = level;
	serial_area(false);
	point start = {-1000, -1000};
	if(feature)
		start = area.find(feature);
	if(start == point{-1000, -1000})
		start = area.bordered(round(appear_side, South));
	after_serial_game(start);
	draw::setnext(game.play);
}

void gamei::writelog() {
	io::file file("logs.txt", StreamWrite | StreamText);
	if(!file)
		return;
	auto p = actable::getlog();
	if(p)
		file << p;
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
	game.enter(start_village, 0, StairsDown, NorthEast);
}