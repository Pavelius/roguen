#include "geoposition.h"
#include "feat.h"
#include "nameable.h"
#include "shape.h"
#include "variant.h"

#pragma once

struct sitegeni;
struct sitei : nameable {
	typedef void (sitei::*fnproc)() const;
	variants		landscape, loot;
	unsigned char	walls, floors;
	featable		feats;
	unsigned char	doors;
	char			chance_hidden_doors, chance_stuck_doors, chance_locked_doors, doors_count;
	const shapei*	shape;
	const sitegeni*	local;
	void			building() const;
	void			cityscape() const;
	void			corridors() const;
	void			dungeon() const;
	void			fillfloor() const;
	void			fillwalls() const;
	void			fillwallsall() const;
	void			nogenerate() const {}
	void			outdoor() const;
	void			room() const;
};
struct sitegeni : nameable {
	sitei::fnproc	proc;
};
struct locationi : sitei {
	variants		sites;
	const sitegeni *global, *global_finish;
	char			darkness, chance_finale, offset;
	color			minimap;
};
struct areaheadi : geoposition {
	struct totali {
		short		mysteries;
		short		traps;
		short		doors, doors_locked, doors_hidden, doors_stuck;
		short		rooms, rooms_hidden;
		short		monsters;
		short		boss;
		short		loots;
	};
	short unsigned	site_id;
	totali			total;
	char			darkness;
	void			clear();
	void			createarea(point start_village);
	locationi*		getloc() const { return bsdata<locationi>::ptr(site_id); }
	void			setloc(const locationi* v) { bsset(site_id, v); }
};