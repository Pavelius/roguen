#include "crt.h"
#include "condition.h"

BSDATA(conditioni) = {
	{"Identified"},
	{"NPC"},
	{"Healthy"},
	{"Wounded"},
	{"HeavyWounded"},
	{"Unaware"},
	{"NoAnyFeature"},
	{"Locked"},
	{"NoInt"},
	{"AnimalInt"},
	{"LowInt"},
	{"AveInt"},
	{"HighInt"},
	{"TargetCreatures"},
	{"TargetFeatures"},
	{"TargetRooms"},
	{"TargetItems"},
	{"Random"},
	{"You"},
	{"Allies"},
	{"Enemies"},
	{"Neutrals"},
	{"Multitarget"},
	{"Ranged"},
};
assert_enum(conditioni, Ranged)