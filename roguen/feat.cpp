#include "bsdata.h"
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
	{"Vorpal"},
	{"AcidDamage"},
	{"FireDamage"},
	{"ColdDamage"},
	{"IllnessDamage"},
	{"IgnoreWeb"},
	{"Fly"},
	{"LightSource"},
	{"Regeneration"},
	{"ManaRegeneration"},
	{"ElfBlood"},
	{"DwarfBlood"},
	{"OrkBlood"},
	{"FastMove"},
	{"SlowMove"},
	{"FastAttack"},
	{"SlowAttack"},
	{"FastAction"},
	{"SlowAction"},
	{"Encumbered"},
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
	{"Stun"},
	{"Blooding"},
};
assert_enum(feati, Blooding)

feat_s negative_feat(feat_s v) {
	switch(v) {
	case FastAction: return SlowAction;
	case FastAttack: return SlowAttack;
	case FastMove: return SlowMove;
	default: return Darkvision;
	}
}