#include "bsreq.h"
#include "draw.h"
#include "draw_object.h"
#include "visualeffect.h"
#include "resid.h"

using namespace draw;

BSDATA(visualeffect) = {
	{"AcidSplash", res::Splash, 2},
	{"BloodingVisual", res::Conditions, 1, 0, 15, -32},
	{"FireSplash", res::Splash, 0},
	{"HitVisual", res::Conditions, 0, 0, 15, -8},
	{"IceSplash", res::Splash, 1},
	{"LightingSplash", res::Splash, 1},
	{"PoisonVisual", res::Conditions, 2, 0, 15, -26},
	{"SearchVisual", res::Splash, 3},
	{"MissileNorth", res::Missile, 0, 0, 15, -32},
	{"MissileSouth", res::Missile, 0, ImageMirrorV, 15, -32},
	{"MissileEast", res::Missile, 1, ImageMirrorH, 15, -32},
	{"MissileWest", res::Missile, 1, 0, 15, -32},
	{"MissileNorthWest", res::Missile, 2, ImageMirrorH, 15, -32},
	{"MissileNorthEast", res::Missile, 2, 0, 15, -32},
	{"MissileSouthWest", res::Missile, 2, ImageMirrorH | ImageMirrorV, 15, -32},
	{"MissileSouthEast", res::Missile, 2, ImageMirrorV, 15, -32},
	{"FlyingItem", res::Items, 0, 0, 15, -32},
};
BSDATAF(visualeffect)