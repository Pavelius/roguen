#include "main.h"

BSDATA(conditioni) = {
	{"Identified"},
	{"NPC"},
	{"Random"},
	{"Healthy"},
	{"Wounded"},
	{"HeavyWounded"},
	{"Unaware"},
	{"NoAnyFeature"},
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
	case Unaware: return isunaware();
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
	case NoAnyFeature:
		return area.features[getposition()] == 0;
	default:
		return true;
	}
}