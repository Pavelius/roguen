#include "areamap.h"
#include "collection.h"
#include "color.h"
#include "geoposition.h"
#include "feat.h"
#include "nameable.h"
#include "ownerable.h"
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
class roomi : public geoposition, public ownerable {
	short unsigned	site_id;
public:
	rect			rc;
	static roomi*	add() { return bsdata<roomi>::add(); }
	point			center() const { return {(short)(rc.x1 + rc.width() / 2), (short)(rc.y1 + rc.height() / 2)}; }
	void			clear() { memset(this, 0, sizeof(*this)); setowner(0); }
	static roomi*	find(geoposition gp, point pt);
	const sitei&	geti() const { return bsdata<sitei>::elements[site_id]; }
	void			getrumor(stringbuilder& sb) const;
	const char*		getname() const { return geti().getname(); }
	static const char* getname(const void* p) { return ((roomi*)p)->getname(); }
	bool			is(feat_s v) const { return geti().feats.is(v); }
	bool			isexplored() const;
	bool			islocal() const;
	bool			ismarkable() const;
	bool			isnotable() const { return is(Notable); }
	void			set(const sitei* p) { bsset(site_id, p); }
};
typedef collection<roomi> rooma;

extern areamap area;
extern areaheadi areahead;
extern const sitei*	last_site;
extern const sitegeni* last_method;
extern locationi* last_location;

int getfloor();
int getwall();