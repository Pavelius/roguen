#include "archive.h"
#include "main.h"

static adat<creature, 16> allies;
static creature* allies_reference[16];
static adat<boosti, 32> boost;

static void after_read_allies() {
	memset(allies_reference, 0, sizeof(allies_reference));
	for(auto& e : allies) {
		auto p = bsdata<creature>::add();
		if(!p)
			continue;
		*p = e;
		allies_reference[allies.indexof(&e)] = p;
	}
}

static void before_write_allies() {
	allies.clear();
	for(auto& e : bsdata<creature>()) {
		if(!e.is(Ally))
			continue;
		if(allies.getcount() == allies.getmaximum())
			e.remove(Ally);
		else {
			allies.add(e);
			e.unlink();
			e.clear();
		}
	}
}

static void after_serial_game() {
	after_read_allies();
}

static void before_serial_game() {
	before_write_allies();
}

static void serial_game(bool write_mode) {
	io::file file("save/game.sav", write_mode ? StreamWrite : StreamRead);
	if(!file)
		return;
	archive a(file, write_mode);
	if(write_mode)
		before_serial_game();
	a.set(game);
	a.set(allies);
	a.set(boost);
	if(!write_mode)
		after_serial_game();
}

static bool serial_area(const char* url, bool write_mode) {
	io::file file(url, write_mode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	archive a(file, write_mode);
	a.set(area);
	a.set(bsdata<itemground>::source);
	a.set(bsdata<creature>::source);
	a.set(bsdata<boosti>::source);
	return true;
}

static void create_area(geoposition v) {
}

static void serial_area(geoposition v, bool write_mode) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("AR%1.2h%2.2h%3.2h.sav", v.position.y, v.position.x, v.level);
	if(!write_mode) {
		if(!serial_area(temp, false)) {
			create_area(v);
			if(!serial_area(temp, false))
				return;
		}
	} else
		serial_area(temp, write_mode);
}

void gamei::write() {
	serial_game(true);
	serial_area(*this, true);
}

void gamei::read() {
	serial_game(false);
	serial_area(*this, false);
}

void gamei::writelog() {
	io::file file("logs.txt", StreamWrite|StreamText);
	if(!file)
		return;
	auto p = actable::getlog();
	if(p)
		file << p;
}