#include "flagable.h"
#include "nameable.h"

#pragma once

enum featn : unsigned char {
	Darkvision, TwoHanded, CutWoods, CutMines, Retaliate, Thrown,
	BleedingHit, StunningHit, PierceHit, MightyHit, Vorpal,
	AcidDamage, FireDamage, ColdDamage, IllnessDamage,
	IgnoreWeb, Fly, LightSource, Regeneration, ManaRegeneration,
	ElfBlood, DwarfBlood, OrkBlood,
	FastMove, SlowMove, FastAttack, SlowAttack, FastAction, SlowAction, Encumbered,
	WeakPoison, StrongPoison, DeathPoison, PoisonResistance, PoisonImmunity,
	AcidResistance, AcidImmunity, ColdResistance, ColdImmunity, DeathResistance, DeathImmunity,
	DiseaseResist, DiseaseImmunity, FireResistance, FireImmunity, StunResistance, StunImmunity,
	Coins, Notable, Natural, KnowRumor, KnowLocation,
	Female, PlaceOwner, Undead, Summoned, Local,
	Stun, Blooding,
};
struct featable : flagable<8> {};
struct feati : nameable {
	featn		immunity;
};

featn negative_feat(featn v);

