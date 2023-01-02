#include "nameable.h"

#pragma once

enum wear_s : unsigned char {
	Backpack, Potion, BackpackLast = Backpack + 15,
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, Ammunition,
	Torso, Head, Backward, Legs, Gloves, FingerRight, FingerLeft, Elbows,
};
struct weari : nameable {
};
