#include "bsreq.h"
#include "main.h"
#include "widget.h"

NOBSDATA(color)
NOBSDATA(dice)
NOBSDATA(itemi::weaponi)
NOBSDATA(point)

BSDATAD(variant)
BSMETA(variant) = {{}};
BSMETA(varianti) = {BSREQ(id), {}};

BSDATAD(advancement);
BSDATAC(boosti, 256);
BSDATAC(classi, 16);
BSDATAC(creature, 256);
BSDATAC(dungeon, 256);
BSDATAC(itemi, 512);
BSDATAC(itemground, 1024);
BSDATAC(monsteri, 512);
BSDATAC(roomi, 64);
BSDATAC(sitei, 256);
BSDATAC(trigger, 256);

BSMETA(abilityi) = {
	BSREQ(id),
	{}};
BSMETA(areafi) = {
	BSREQ(id),
	{}};
BSMETA(classi) = {
	BSREQ(id),
	BSREQ(hd), BSREQ(cap), BSREQ(player),
	{}};
BSMETA(speech) = {
	BSREQ(id),
	{}};
BSMETA(color) = {
	BSREQ(r), BSREQ(g), BSREQ(b),
	{}};
BSMETA(conditioni) = {
	BSREQ(id),
	{}};
BSMETA(dice) = {
	BSREQ(min),
	BSREQ(max),
	{}};
BSMETA(dungeon) = {
	BSREQ(rumor),
	BSREQ(level),
	BSREQ(final_level),
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
	BSENM(ability, abilityi),
	BSFLG(feats, feati),
	BSENM(wear, weari),
	BSREQ(weapon),
	BSREQ(bonus),
	BSREQ(wear_index),
	BSREQ(use),
	{}};
BSMETA(itemi::weaponi) = {
	BSREQ(parry), BSREQ(enemy_parry),
	BSREQ(block), BSREQ(enemy_block), BSREQ(block_ranged),
	BSREQ(damage), BSREQ(pierce),
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
	BSREQ(use),
	BSREQ(avatar),
	BSREQ(friendly),
	BSREQ(parent),
	BSREQ(treasure),
	BSREQ(appear),
	BSREQ(minions),
	{}};
BSMETA(point) = {
	BSREQ(x), BSREQ(y),
	{}};
BSMETA(racei) = {
	BSREQ(id),
	{}};
BSMETA(shapei) = {
	BSREQ(id),
	{}};
BSMETA(sitei) = {
	BSREQ(id),
	BSREQ(global), BSREQ(global_finish),
	BSREQ(local),
	BSREQ(landscape),
	BSREQ(darkness),
	BSREQ(levels),
	BSFLG(feats, feati),
	BSREQ(sites),
	BSENM(walls, tilei),
	BSENM(floors, tilei),
	BSREQ(minimap),
	BSREQ(offset),
	{}};
BSMETA(sitegeni) = {
	BSREQ(id),
	{}};
BSMETA(spelli) = {
	BSREQ(id),
	BSENM(target, targeti),
	{}};
BSMETA(targeti) = {
	BSREQ(id),
	{}};
BSMETA(tilei) = {
	BSREQ(id),
	{}};
BSMETA(triggeri) = {
	BSREQ(id),
	{}};
BSMETA(trigger) = {
	BSENM(type, triggeri),
	BSREQ(p1),
	BSREQ(p2),
	BSREQ(effect),
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
	{"AreaFlag", VAR(areafi), 1},
	{"Class", VAR(classi), 1},
	{"Condition", VAR(conditioni), 1},
	{"Dungeon", VAR(dungeon), 0},
	{"Feat", VAR(feati), 1},
	{"Feature", VAR(featurei), 1},
	{"Hotkey", VAR(hotkey), 2},
	{"HotkeyList", VAR(hotkeylist), 1},
	{"Item", VAR(itemi), 1},
	{"List", VAR(listi), 1},
	{"Monster", VAR(monsteri), 1},
	{"Race", VAR(racei), 1},
	{"RandomTable", VAR(randomizeri), 1},
	{"Script", VAR(script), 1},
	{"Shape", VAR(shapei), 1},
	{"Site", VAR(sitei), 1},
	{"SiteGenerator", VAR(sitegeni), 1},
	{"Spell", VAR(spelli), 1},
	{"Speech", VAR(speech), 1},
	{"Tile", VAR(tilei), 1},
	{"Trigger", VAR(trigger), 3},
	{"VisualEffect", VAR(visualeffect), 1},
	{"Widget", VAR(widget), 1},
};
BSDATAF(varianti)