#include "bsreq.h"
#include "condition.h"
#include "dialog.h"
#include "hotkey.h"
#include "global.h"
#include "greatneed.h"
#include "creature.h"
#include "modifier.h"
#include "race.h"
#include "script.h"
#include "siteskill.h"
#include "trigger.h"
#include "visualeffect.h"
#include "widget.h"

NOBSDATA(color)
NOBSDATA(dice)
NOBSDATA(diceprogress)
NOBSDATA(framerange)
NOBSDATA(point)
NOBSDATA(weaponi)

BSDATAD(variant)
BSMETA(variant) = {{}};
BSMETA(varianti) = {BSREQ(id), {}};

BSDATAD(advancement);
BSDATAC(classi, 16);
BSDATAC(creature, 256);
BSDATAC(globali, 128);
BSDATAC(itemi, 512);
BSDATAC(itemground, 1024);
BSDATAC(locationi, 128);
BSDATAC(monsteri, 512);
BSDATAC(roomi, 64);
BSDATAC(sitei, 256);
BSDATAC(siteskilli, 256);
BSDATAC(tilei, 64);
BSDATAC(trigger, 256);

BSMETA(abilityi) = {
	BSREQ(id),
	{}};
BSMETA(advancement) = {
	BSREQ(type),
	BSREQ(level),
	BSREQ(elements),
	{}};
BSMETA(globali) = {
	BSREQ(id),
	BSREQ(minimum), BSREQ(maximum), BSREQ(current),
	BSREQ(effect), BSREQ(fail),
	{}};
BSMETA(areafi) = {
	BSREQ(id),
	{}};
BSMETA(classi) = {
	BSREQ(id),
	BSREQ(player),
	{}};
BSMETA(creature) = {
	BSREQ(abilities),
	{}};
BSMETA(speech) = {
	BSREQ(id),
	{}};
BSMETA(color) = {
	BSREQ(r), BSREQ(g), BSREQ(b), BSREQ(a),
	{}};
BSMETA(conditioni) = {
	BSREQ(id),
	{}};
BSMETA(dialogi) = {
	BSREQ(id),
	{}};
BSMETA(dice) = {
	BSREQ(min),
	BSREQ(max),
	{}};
BSMETA(feati) = {
	BSREQ(id),
	{}};
BSMETA(framerange) = {
	BSREQ(start), BSREQ(count),
	{}};
BSMETA(itemi) = {
	BSREQ(id), BSREQ(no_identifier),
	BSREQ(weight), BSREQ(cost),
	BSFLG(feats, feati),
	BSREQ(wearing),
	BSREQ(parent),
	BSREQ(count),
	BSREQ(avatar),
	BSREQ(rotting),
	BSENM(wear, weari),
	BSREQ(wear_index),
	BSREQ(weapon),
	BSREQ(use), BSREQ(use_blessed), BSREQ(use_cursed),
	BSREQ(powers), BSREQ(chance_power),
	{}};
BSMETA(locationi) = {
	BSREQ(id),
	BSREQ(global), BSREQ(global_finish),
	BSREQ(local),
	BSREQ(landscape), BSREQ(sites), BSREQ(loot),
	BSREQ(darkness), BSREQ(offset),
	BSREQ(chance_hidden_doors), BSREQ(chance_locked_doors), BSREQ(chance_stuck_doors),
	BSREQ(doors_count),
	BSFLG(feats, feati),
	BSREQ(chance_finale),
	BSENM(walls, tilei),
	BSENM(floors, tilei),
	BSENM(doors, featurei),
	BSREQ(minimap),
	{}};
BSMETA(modifieri) = {
	BSREQ(id),
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
BSMETA(shapei) = {
	BSREQ(id),
	{}};
BSMETA(sitei) = {
	BSREQ(id),
	BSREQ(local),
	BSREQ(landscape), BSREQ(loot),
	BSREQ(chance_hidden_doors), BSREQ(chance_locked_doors), BSREQ(chance_stuck_doors),
	BSREQ(doors_count),
	BSFLG(feats, feati),
	BSENM(walls, tilei),
	BSENM(floors, tilei),
	BSENM(doors, featurei),
	{}};
BSMETA(sitegeni) = {
	BSREQ(id),
	{}};
BSMETA(siteskilli) = {
	BSREQ(id),
	BSENM(skill, abilityi),
	BSREQ(bonus),
	BSENM(retry, durationi),
	BSFLG(target, conditioni),
	BSREQ(effect), BSREQ(fail),
	{}};
BSMETA(spelli) = {
	BSREQ(id),
	BSREQ(mana), BSREQ(count),
	BSENM(duration, durationi),
	BSFLG(target, conditioni),
	BSREQ(effect), BSREQ(summon),
	{}};
BSMETA(weaponi) = {
	BSREQ(damage), BSREQ(skill), BSREQ(pierce), BSREQ(speed), BSREQ(ammunition),
	{}};
BSMETA(weari) = {
	BSREQ(id),
	{}};
BSMETA(visualeffect) = {
	BSREQ(id),
	{}};
BSDATA(varianti) = {
	{"NoVariant"},
	{"Ability", VAR(abilityi), 1},
	{"Advancement", VAR(advancement), 2},
	{"AreaFlag", VAR(areafi), 1},
	{"Class", VAR(classi), 1},
	{"Condition", VAR(conditioni), 1},
	{"Creature", VAR(creature), 0},
	{"Dialog", VAR(dialogi), 1},
	{"Feat", VAR(feati), 1},
	{"Feature", VAR(featurei), 1},
	{"Global", VAR(globali), 1},
	{"Hotkey", VAR(hotkey), 2},
	{"Item", VAR(itemi), 1},
	{"List", VAR(listi), 1},
	{"Location", VAR(locationi), 1},
	{"Modifier", VAR(modifieri), 1},
	{"Monster", VAR(monsteri), 1},
	{"Need", VAR(greatneedi), 1},
	{"NeedFlag", VAR(needni), 1},
	{"Quest", VAR(quest), 0},
	{"Race", VAR(racei), 1},
	{"RandomTable", VAR(randomizeri), 1},
	{"Script", VAR(script), 1},
	{"Shape", VAR(shapei), 1},
	{"Site", VAR(sitei), 1},
	{"SiteGenerator", VAR(sitegeni), 1},
	{"SiteSkill", VAR(siteskilli), 1},
	{"Spell", VAR(spelli), 1},
	{"Speech", VAR(speech), 1},
	{"Tile", VAR(tilei), 1},
	{"TileFlag", VAR(tilefi), 1},
	{"Trigger", VAR(trigger), 3},
	{"Variant", VAR(varianti), 1},
	{"VisualEffect", VAR(visualeffect), 1},
	{"Widget", VAR(widget), 1},
};
BSDATAF(varianti)