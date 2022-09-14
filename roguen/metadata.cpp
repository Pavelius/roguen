#include "bsreq.h"
#include "main.h"
#include "widget.h"

NOBSDATA(dice)
NOBSDATA(itemi::weaponi)

BSDATAD(variant)
BSMETA(variant) = {{}};
BSMETA(varianti) = {BSREQ(id), {}};

BSDATAC(classi, 16);
BSDATAC(creature, 256);
BSDATAC(itemi, 512);
BSDATAC(itemground, 1024);
BSDATAD(advancement);
BSDATAC(monsteri, 512);

BSMETA(abilityi) = {
	BSREQ(id),
	{}};
BSMETA(dice) = {
	BSREQ(min),
	BSREQ(max),
	{}};
BSMETA(feati) = {
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
BSMETA(weari) = {
	BSREQ(id),
	{}};

BSDATA(varianti) = {
	{"NoVariant", VAR(script), 1},
	{"Item", VAR(itemi), 1},
	{"Advancement", VAR(advancement), 2},
	{"List", VAR(listi), 1},
	{"Monster", VAR(monsteri), 1},
	{"Race", VAR(racei), 1},
	{"Script", VAR(script), 1},
	{"Widget", VAR(widget), 1},
};
BSDATAF(varianti)