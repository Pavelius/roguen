#include "collection.h"
#include "color.h"
#include "geoposition.h"
#include "feat.h"
#include "nameable.h"
#include "ownerable.h"
#include "shape.h"
#include "variant.h"

#pragma once

struct script;
struct sitei : nameable {
	variants		landscape, loot;
	unsigned char	walls, floors;
	featable		feats;
	unsigned char	doors;
	char			chance_hidden_doors, chance_stuck_doors, chance_locked_doors, doors_count;
	const shapei*	shape;
	const script*	local;
	variants		getloot() const;
};
extern const sitei*	last_site;
struct locationi : sitei {
	variants		sites;
	const script	*global, *global_finish;
	char			darkness, chance_finale, offset, traps_count;
	color			minimap;
};
extern locationi* last_location;
struct areaheadi : geoposition {
	struct totali {
		short		mysteries;
		short		traps;
		short		doors, doors_locked, doors_hidden, doors_stuck;
		short		rooms, rooms_hidden;
		short		monsters;
		short		boss;
		short		loots, artifacts, cursed, blessed;
	};
	short unsigned	site_id;
	totali			total;
	char			darkness;
	void			clear();
	locationi*		getloc() const { return bsdata<locationi>::ptr(site_id); }
	void			setloc(const locationi* v) { bsset(site_id, v); }
};
class roomi : public ownerable {
	short unsigned	site_id;
public:
	unsigned		param;
	rect			rc;
	point			center() const { return {(short)(rc.x1 + rc.width() / 2), (short)(rc.y1 + rc.height() / 2)}; }
	void			clear() { memset(this, 0, sizeof(*this)); setowner(0); }
	const sitei&	geti() const { return bsdata<sitei>::elements[site_id]; }
	point			getsellitems() const;
	point			getspecialsellitems() const;
	void			getrumor(stringbuilder& sb) const;
	const char*		getname() const;
	int				getseed() const;
	static const char* getname(const void* p) { return ((roomi*)p)->getname(); }
	bool			is(feat_s v) const { return geti().feats.is(v); }
	bool			isexplored() const;
	bool			ismarkable() const;
	bool			isnotable() const { return is(Notable); }
	void			set(const sitei* p) { bsset(site_id, p); }
};
extern roomi* last_room;
typedef collection<roomi> rooma;
extern rooma rooms;

roomi* add_room();
roomi* find_room(point pt);
int room_index(const roomi* p);