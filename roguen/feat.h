#include "flagable.h"
#include "nameable.h"

#pragma once

enum feat_s : unsigned char {
	Darkvision, TwoHanded, CutWoods, Retaliate, Thrown, Cursed, Blessed,
	BleedingHit, StunningHit, PierceHit, MightyHit, NoDamageUndead, MissHalfTime,
	FireDamage,
	IgnoreWeb, Fly, LightSource, Regeneration, ManaRegeneration,
	WeakPoison, StrongPoison, DeathPoison, PoisonResistance, PoisonImmunity,
	AcidResistance, AcidImmunity, ColdResistance, ColdImmunity, DiseaseResist, DiseaseImmunity,
	FireResistance, FireImmunity, StunResistance, StunImmunity,
	Coins, Notable, Natural, KnowRumor, KnowLocation,
	Female, PlaceOwner, Undead, Summoned, Local, Ally, Enemy,
	Stun, Blooding,
};
struct featable : flagable<8> {};
struct feati : nameable {};

