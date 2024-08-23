#pragma once

#include "direction.h"
#include "feature.h"
#include "geoposition.h"
#include "ownerable.h"

struct gamei : public geoposition, public ownerable {
	unsigned		minutes;
	unsigned		restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	int				globals[128];
	void			clear();
	unsigned		getminutes() const { return minutes; }
	static bool		isvalid(point m) { return m.x > 0 && m.x < 256 && m.y > 0 && m.y < 256; }
};
extern gamei game;

void end_game();
void enter_area(point m, int level, const featurei* entry, direction_s appear_side);
void main_menu();
void next_phase(fnevent proc);
void new_game();
void pass_minute();
bool present_game(const char* name);
void play_game();
void save_game(const char* name);
void save_log();
void load_game(const char* name);

