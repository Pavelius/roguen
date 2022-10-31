#include "main.h"

BSDATA(conditioni) = {
	{"NoModifier"},
	{"Identified"},
	{"NPC"},
	{"Random"},
	{"ShowMinimapBullet"},
	{"Healthy"},
	{"LightWounded"},
	{"HeavyWounded"},
	{"Busy"},
	{"NoInt"},
	{"AnimalInt"},
	{"LowInt"},
	{"AveInt"},
	{"HighInt"},
};
assert_enum(conditioni, HighInt);

bool creature::is(condition_s v) const {
	int n, m;
	switch(v) {
	case Busy: return wait_seconds > 1000;
	case NPC: return ischaracter();
	case Random: return d100() < 40;
	case NoInt: return get(Wits) == 10;
	case AnimalInt: return get(Wits) < 10;
	case LowInt: return get(Wits) < 20;
	case AveInt: return get(Wits) < 35;
	case HighInt: return get(Wits) < 50;
	case LightWounded:
		n = get(Hits);
		m = get(HitsMaximum);
		return n > 0 && n < m && n >= m / 2;
	case HeavyWounded:
		n = get(Hits);
		m = get(HitsMaximum);
		return n > 0 && n < m / 2;
	case NoWounded: return get(Hits) == get(HitsMaximum);
	default:
		return true;
	}
}