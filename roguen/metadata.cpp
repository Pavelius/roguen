#include "bsreq.h"
#include "main.h"
#include "widget.h"

BSDATAD(variant)
BSMETA(variant) = {{}};
BSMETA(varianti) = {BSREQ(id), {}};

BSDATAC(classi, 16);
BSDATAC(creature, 256);
BSDATAC(monsteri, 512);

BSMETA(abilityi) = {
	BSREQ(id),
	{}};
BSMETA(genderi) = {
	BSREQ(id),
	{}};
BSMETA(monsteri) = {
	BSREQ(id),
	BSDST(abilities, abilityi),
	BSENM(gender, genderi),
	BSREQ(avatar),
	{}};
BSMETA(racei) = {
	BSREQ(id),
	{}};

BSDATA(varianti) = {
	{"NoVariant", VAR(script), 1},
	{"Monster", VAR(monsteri), 1},
	{"Race", VAR(racei), 1},
	{"Script", VAR(script), 1},
	{"Widget", VAR(widget), 1},
};
BSDATAF(varianti)