#include "areapiece.h"
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
BSDATAD(areapiece);
BSDATAC(classi, 16);
BSDATAC(creature, 256);
BSDATAC(itemi, 512);
BSDATAC(locationi, 128);
BSDATAC(monsteri, 512);
BSDATAC(sitei, 256);
BSDATAC(siteskilli, 256);
BSDATAC(tilei, 64);
BSDATAC(trigger, 256);

BSMETA(abilityi) = {
	BSREQ(id),
	{}};
BSMETA(advancement) = {
	BSREQ(type), BSREQ(level), BSREQ(id),
	BSREQ(elements),
	{}};
BSMETA(areapiece) = {
	BSREQ(position), BSREQ(level),
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
	BSREQ(id), BSREQ(unidentified),
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
	BSREQ(use),
	BSREQ(powers), BSREQ(chance_power),
	BSDST(required, abilityi),
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
	BSREQ(chance_finale), BSREQ(traps_count),
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
	BSREQ(unique),
	BSREQ(use),
	BSREQ(avatar),
	BSREQ(friendly),
	BSREQ(parent),
	BSREQ(treasure),
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
BSMETA(siteskilli) = {
	BSREQ(id),
	BSENM(skill, abilityi),
	BSREQ(bonus),
	BSENM(retry, durationi),
	BSFLG(target, conditioni),
	BSREQ(effect),
	{}};
BSMETA(spelli) = {
	BSREQ(id),
	BSREQ(mana), BSREQ(count),
	BSENM(duration, durationi),
	BSFLG(target, conditioni),
	BSREQ(effect), BSREQ(summon),
	{}};
BSMETA(weaponi) = {
	BSREQ(damage), BSREQ(pierce), BSREQ(speed), BSREQ(ammunition),
	{}};
BSMETA(visualeffect) = {
	BSREQ(id),
	{}};
BSMETA(weari) = {
	BSREQ(id),
	{}};
BSDATA(varianti) = {
	{"NoVariant"},
	{"Ability", VAR(abilityi), 1, 0, ftscript<abilityi>, fttest<abilityi>},
	{"Advancement", VAR(advancement), 3},
	{"AreaFlag", VAR(areafi), 1, 0, ftscript<areafi>},
	{"Class", VAR(classi), 1, 0, ftscript<classi>},
	{"Condition", VAR(conditioni), 1, 0, 0, fttest<conditioni>},
	{"Creature", VAR(creature), 0},
	{"Dialog", VAR(dialogi), 1},
	{"Feat", VAR(feati), 1, 0, ftscript<feati>, fttest<feati>},
	{"Feature", VAR(featurei), 1, 0, ftscript<featurei>},
	{"Global", VAR(globali), 1, 0, ftscript<globali>},
	{"Hotkey", VAR(hotkey), 2},
	{"Item", VAR(itemi), 1, 0, ftscript<itemi>},
	{"List", VAR(listi), 1, 0, ftscript<listi>, fttest<listi>},
	{"Location", VAR(locationi), 1, 0, ftscript<locationi>},
	{"Modifier", VAR(modifieri), 1, 0, ftscript<modifieri>},
	{"Monster", VAR(monsteri), 1, 0, ftscript<monsteri>, fttest<monsteri>},
	{"Need", VAR(greatneedi), 1, 0, ftscript<needni>, fttest<needni>},
	{"NeedFlag", VAR(needni), 1},
	{"Quest", VAR(quest), 0},
	{"Race", VAR(racei), 1, 0, ftscript<racei>},
	{"RandomTable", VAR(randomizeri), 1, 0, ftscript<randomizeri>},
	{"Script", VAR(script), 1, 0, ftscript<script>, fttest<script>},
	{"Shape", VAR(shapei), 1, 0, ftscript<shapei>},
	{"Site", VAR(sitei), 1, 0, ftscript<sitei>},
	{"SiteSkill", VAR(siteskilli), 1},
	{"Spell", VAR(spelli), 1, 0, ftscript<spelli>},
	{"Speech", VAR(speech), 1, 0, ftscript<speech>},
	{"Tile", VAR(tilei), 1, 0, ftscript<tilei>},
	{"TileFlag", VAR(tilefi), 1},
	{"Trigger", VAR(trigger), 3},
	{"Variant", VAR(varianti), 1},
	{"VisualEffect", VAR(visualeffect), 1},
};
BSDATAF(varianti)