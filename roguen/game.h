#include "direction.h"
#include "feature.h"
#include "geoposition.h"
#include "global.h"
#include "ownerable.h"

#pragma once

class gamei : public geoposition, public ownerable {
	unsigned		minutes;
	unsigned		restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	int				globals[128];
	void			setup_globals();
	static void		setup_rumors(int count);
public:
	static point	start_village;
	void			clear();
	static void		endgame();
	void			enter(point m, int level, const featurei* entry, direction_s appear_side);
	int				get(const globali& e) const { return globals[bsid(&e)]; }
	static int		getcount(variant v, int minimal = 1);
	unsigned		getminutes() const { return minutes; }
	static int		getpositivecount(variant v);
	static int		getrange(point m1, point m2);
	static bool		isvalid(point m) { return m.x > 0 && m.x < 256 && m.y > 0 && m.y < 256; }
	static void		next(fnevent proc);
	static void		newgame();
	static void		mainmenu();
	void			pass(unsigned minutes);
	void			passminute();
	static void		play();
	void			playminute();
	void			randomworld();
	void			read();
	void			set(const globali& e, int v);
	const char*		timeleft(unsigned end_stamp) const;
	void			write(const char* name);
	static void		writelog();
};
extern gamei game;
