#include "archive.h"
#include "areapiece.h"
#include "boost.h"
#include "draw.h"
#include "draw_object.h"
#include "game.h"
#include "greatneed.h"
#include "creature.h"
#include "io_memory.h"

//static vector<creature> saved_creatures;
static const char* save_folder = "save";

//static void save_monsters() {
//	saved_creatures.clear();
//	for(auto& e : bsdata<creature>()) {
//		if(!e.isvalid())
//			continue;
//		if(e.is(Local)) {
//			e.unlink();
//			saved_creatures.add(e);
//			e.clear();
//		}
//	}
//}
//
//static void restore_monsters() {
//	for(auto& e : saved_creatures) {
//		auto p = bsdata<creature>::addz();
//		if(p)
//			*p = e;
//	}
//	saved_creatures.clear();
//}
//
//static void after_serial_game() {
//	restore_monsters();
//	update_ui();
//}
//
//static void before_serial_game() {
//	save_monsters();
//}

template<> void archive::set<areapiece>(areapiece& e) {
	set(*static_cast<areaheadi*>(&e));
	set(*static_cast<areamap*>(&e));
	setc<roomi>(e.rooms);
	setc<itemground>(e.items);
}

static void serial_game(const char* url, bool write_mode) {
	io::file file(url, write_mode ? StreamWrite : StreamRead);
	if(!file)
		return;
	archive a(file, write_mode);
	if(!a.signature("SAV"))
		return;
	if(!a.version(1, 0))
		return;
	a.set(game);
	a.set(bsdata<boosti>::source);
	a.set(bsdata<creature>::source);
	a.set(bsdata<greatneed>::source);
	a.setc<areapiece>(bsdata<areapiece>::source);
}

static void serial_game_name(const char* id, bool write_mode) {
	char temp[260]; stringbuilder sb(temp);
	sb.add("saves/%1.sav", id);
	serial_game(temp, write_mode);
}

void gamei::write(const char* name) {
	serial_game_name(name, true);
}

//static void cleanup_saves() {
//	char temp[260]; stringbuilder sb(temp);
//	for(io::file::find file(save_folder); file; file.next()) {
//		auto pn = file.name();
//		if(pn[0] == '.')
//			continue;
//		sb.clear();
//		sb.add("%1/%2", save_folder, pn);
//		io::file::remove(temp);
//	}
//}

void gamei::writelog() {
	io::file file("logs.txt", StreamWrite | StreamText);
	if(!file)
		return;
	auto p = actable::getlog();
	if(p)
		file << p;
}