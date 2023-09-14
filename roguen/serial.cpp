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

static void serial_game(bool write_mode) {
	io::file file("save/game.sav", write_mode ? StreamWrite : StreamRead);
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
}

//static bool serial_area(const char* url, bool write_mode) {
//	io::file file(url, write_mode ? StreamWrite : StreamRead);
//	if(!file)
//		return false;
//	archive a(file, write_mode);
//	a.set(*static_cast<areaheadi*>(area));
//	a.set(*static_cast<areamap*>(area));
//	a.set(saved_creatures);
//	a.set(bsdata<itemground>::source);
//	return true;
//}

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