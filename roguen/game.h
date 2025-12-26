#pragma once

#include "direction.h"
#include "feature.h"
#include "geoposition.h"
#include "ownerable.h"
#include "timemanage.h"

struct gamei : geoposition, ownerable, timemanage {
	int			globals[128];
	void		clear();
	static bool	isvalid(point m) { return m.x > 0 && m.x < 256 && m.y > 0 && m.y < 256; }
};
extern gamei game;

void check_time_passed();
void end_game();
void enter_area(point m, int level, const featurei* entry, directionn appear_side);
void main_menu();
bool isnext();
void next_phase(fnevent proc);
void new_game();
bool present_game(const char* name);
void play_game();
void save_game(const char* name);
void save_log();
void load_game(const char* name);

