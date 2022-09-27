#include "bsreq.h"
#include "main.h"
#include "widget.h"

NOBSDATA(dice)
NOBSDATA(itemi::weaponi)

BSDATAD(variant)
BSMETA(variant) = {{}};
BSMETA(varianti) = {BSREQ(id), {}};

BSDATAD(advancement);
BSDATAC(classi, 16);
BSDATAC(creature, 256);
BSDATAC(boosti, 256);
BSDATAC(itemi, 512);
BSDATAC(itemground, 1024);
BSDATAC(monsteri, 512);
BSDATAC(sitei, 64);

BSMETA(abilityi) = {
	BSREQ(id),
	{}};
BSMETA(classi) = {
	BSREQ(id),
	BSREQ(hd), BSREQ(cap), BSREQ(player),
	{}};
BSMETA(dice) = {
	BSREQ(min),
	BSREQ(max),
	{}};
BSMETA(feati) = {
	BSREQ(id),
	{}};
BSMETA(featurei) = {
	BSREQ(id),
	{}};
BSMETA(itemi) = {
	BSREQ(id),
	BSREQ(cost), BSREQ(weight), BSREQ(count),
	BSREQ(avatar),
	BSFLG(flags, feati),
	BSENM(wear, weari),
	BSREQ(weapon),
	BSREQ(bonus),
	BSREQ(wear_index),
	{}};
BSMETA(itemi::weaponi) = {
	BSREQ(damage),
	BSENM(ammunition, itemi),
	{}};
BSMETA(advancement) = {
	BSREQ(type),
	BSREQ(level),
	BSREQ(elements),
	{}};
BSMETA(monsteri) = {
	BSREQ(id),
	BSDST(abilities, abilityi),
	BSFLG(feats, feati),
	BSREQ(avatar),
	{}};
BSMETA(racei) = {
	BSREQ(id),
	{}};
BSMETA(sitei) = {
	BSREQ(id),
	BSREQ(landscape),
	BSREQ(overland),
	{}};
BSMETA(weari) = {
	BSREQ(id),
	{}};
BSMETA(visualeffect) = {
	BSREQ(id),
	{}};

BSDATA(varianti) = {
	{"NoVariant", VAR(script), 1},
	{"Ability", VAR(abilityi), 1},
	{"Advancement", VAR(advancement), 2},
	{"Class", VAR(classi), 1},
	{"Feat", VAR(feati), 1},
	{"Feature", VAR(featurei), 1},
	{"Hotkey", VAR(hotkey), 2},
	{"HotkeyList", VAR(hotkeylist), 1},
	{"Item", VAR(itemi), 1},
	{"List", VAR(listi), 1},
	{"Monster", VAR(monsteri), 1},
	{"Race", VAR(racei), 1},
	{"Script", VAR(script), 1},
	{"Site", VAR(sitei), 1},
	{"VisualEffect", VAR(visualeffect), 1},
	{"Widget", VAR(widget), 1},
};
BSDATAF(varianti)