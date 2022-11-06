#include "main.h"

BSDATA(conditioni) = {
	{"Identified"},
	{"NPC"},
	{"Random"},
	{"ShowMinimapBullet"},
	{"Healthy"},
	{"Wounded"},
	{"HeavyWounded"},
	{"Busy"},
	{"NoWebbed"},
	{"NoInt"},
	{"AnimalInt"},
	{"LowInt"},
	{"AveInt"},
	{"HighInt"},
	{"Item"},
	{"Feature"},
	{"You"},
	{"Allies"},
	{"Enemies"},
	{"Neutrals"},
	{"Multitarget"},
	{"Ranged"},
};
assert_enum(conditioni, Ranged);

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
	case Wounded:
		n = get(Hits);
		m = get(HitsMaximum);
		return n > 0 && n < m;
	case HeavyWounded:
		n = get(Hits);
		m = get(HitsMaximum);
		return n > 0 && n < m / 2;
	case NoWounded: return get(Hits) == get(HitsMaximum);
	case Allies:
		if(player->is(Ally))
			return is(Ally);
		else if(player->is(Enemy))
			return is(Enemy);
		return false;
	case Enemies:
		if(player->is(Enemy))
			return is(Ally);
		else if(player->is(Ally))
			return is(Enemy);
		return false;
	case NoWebbed:
		return !area.is(getposition(), Webbed);
	default:
		return true;
	}
}

bool item::is(condition_s v) const {
	switch(v) {
	case NoWounded: return broken == 0;
	case Wounded: return broken != 0;
	case HeavyWounded: return broken >= 2;
	case Identified: return identified != 0;
	default: return false;
	}
}