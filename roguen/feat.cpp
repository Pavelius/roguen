#include "crt.h"
#include "feat.h"

BSDATA(feati) = {
	{"Darkvision"},
	{"TwoHanded"},
	{"CutWoods"},
	{"CutMines"},
	{"Retaliate"},
	{"Thrown"},
	{"BleedingHit"},
	{"StunningHit"},
	{"PierceHit"},
	{"MightyHit"},
	{"MissHalfTime"},
	{"Vorpal"},
	{"AcidDamage"},
	{"FireDamage"},
	{"ColdDamage"},
	{"IgnoreWeb"},
	{"Fly"},
	{"LightSource"},
	{"Regeneration"},
	{"ManaRegeneration"},
	{"WeakPoison"},
	{"StrongPoison"},
	{"DeathPoison"},
	{"PoisonResistance", PoisonImmunity},
	{"PoisonImmunity"},
	{"AcidResistance", AcidImmunity},
	{"AcidImmunity"},
	{"ColdResistance", ColdImmunity},
	{"ColdImmunity"},
	{"DeathResistance", DeathImmunity},
	{"DeathImmunity"},
	{"DiseaseResist", DiseaseImmunity},
	{"DiseaseImmunity"},
	{"FireResistance", FireImmunity},
	{"FireImmunity"},
	{"StunResistance", StunImmunity},
	{"StunImmunity"},
	{"Coins"},
	{"Notable"},
	{"Natural"},
	{"KnowRumor"},
	{"KnowLocation"},
	{"Female"},
	{"PlaceOwner"},
	{"Undead"},
	{"Summoned"},
	{"Local"},
	{"Ally"},
	{"Enemy"},
	{"Stun"},
	{"Blooding"},
};
assert_enum(feati, Blooding)