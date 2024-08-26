#include "flagable.h"
#include "nameable.h"

#pragma once

enum feat_s : unsigned char {
	Darkvision, TwoHanded, CutWoods, CutMines, Retaliate, Thrown,
	BleedingHit, StunningHit, PierceHit, MightyHit, Vorpal,
	AcidDamage, FireDamage, ColdDamage,
	IgnoreWeb, Fly, LightSource, Regeneration, ManaRegeneration,
	FastMove, SlowMove, FastAttack, SlowAttack, FastAction, SlowAction,
	WeakPoison, StrongPoison, DeathPoison, PoisonResistance, PoisonImmunity,
	AcidResistance, AcidImmunity, ColdResistance, ColdImmunity, DeathResistance, DeathImmunity,
	DiseaseResist, DiseaseImmunity, FireResistance, FireImmunity, StunResistance, StunImmunity,
	Coins, Notable, Natural, KnowRumor, KnowLocation,
	Female, PlaceOwner, Undead, Summoned, Local, Ally, Enemy,
	Stun, Blooding,
};
struct featable : flagable<8> {};
struct feati : nameable {
	feat_s		immunity;
};

feat_s negative_feat(feat_s v);

