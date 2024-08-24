#include "nameable.h"

#pragma once

enum wear_s : unsigned char {
	Backpack, Potion, Scroll, BackpackLast = Backpack + 15,
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, Ammunition,
	Torso, Head, Neck, Backward, Girdle, Gloves, FingerRight, FingerLeft, Elbows, Legs,
};
struct weari : nameable {
};
