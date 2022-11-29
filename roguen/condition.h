#include "nameable.h"

#pragma once

enum condition_s : unsigned char {
	Identified, NPC,
	NoWounded, Wounded, HeavyWounded,
	Unaware, NoAnyFeature, Locked,
	NoInt, AnimalInt, LowInt, AveInt, HighInt,
	TargetCreatures, TargetFeatures, TargetRooms, TargetItems, Random,
	You, Allies, Enemies, Neutrals, Multitarget, Ranged,
};

struct conditioni : nameable {
};
