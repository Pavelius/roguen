#include "direction.h"
#include "feature.h"
#include "geoposition.h"
#include "global.h"
#include "ownerable.h"

#pragma once

struct gamei : public geoposition, public ownerable {
	unsigned		minutes;
	unsigned		restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	int				globals[128];
	static point	start_village;
	void			setup_globals();
	static void		setup_rumors(int count);
	void			clear();
	void			enter(point m, int level, const featurei* entry, direction_s appear_side);
	static int		getcount(variant v, int minimal = 1);
	unsigned		getminutes() const { return minutes; }
	static int		getpositivecount(variant v);
	static int		getrange(point m1, point m2);
	static bool		isvalid(point m) { return m.x > 0 && m.x < 256 && m.y > 0 && m.y < 256; }
	void			pass(unsigned minutes);
	void			playminute();
	void			randomworld();
	void			set(const globali& e, int v);
	const char*		timeleft(unsigned end_stamp) const;
};
extern gamei game;

void end_game();
void main_menu();
void next_phase(fnevent proc);
void new_game();
void pass_minute();
bool present_game(const char* name);
void play_game();
void save_game(const char* name);
void save_log();
void load_game(const char* name);

