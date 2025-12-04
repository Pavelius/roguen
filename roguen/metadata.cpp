#include "areapiece.h"
#include "bsreq.h"
#include "craft.h"
#include "filter.h"
#include "hotkey.h"
#include "global.h"
#include "greatneed.h"
#include "charname.h"
#include "creature.h"
#include "modifier.h"
#include "querry.h"
#include "race.h"
#include "script.h"
#include "siteskill.h"
#include "speech.h"
#include "talk.h"
#include "trigger.h"
#include "visualeffect.h"
#include "widget.h"

NOBSDATA(color)
NOBSDATA(dice)
NOBSDATA(framerange)
NOBSDATA(point)
NOBSDATA(weaponi)

BSDATAD(variant)
BSMETA(variant) = {{}};
BSMETA(varianti) = {BSREQ(id), {}};

BSDATAD(advancement);
BSDATAD(areapiece);
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
	BSREQ(type), BSREQ(id),
	BSREQ(elements),
	{}};
BSMETA(areapiece) = {
	BSREQ(position), BSREQ(level),
	{}};
BSMETA(areafi) = {
	BSREQ(id),
	{}};
BSMETA(crafti) = {
	BSREQ(id),
	BSREQ(elements),
	{}};
BSMETA(creature) = {
	BSREQ(abilities),
	{}};
BSMETA(color) = {
	BSREQ(r), BSREQ(g), BSREQ(b), BSREQ(a),
	{}};
BSMETA(dice) = {
	BSREQ(min),
	BSREQ(max),
	{}};
BSMETA(feati) = {
	BSREQ(id),
	{}};
BSMETA(filteri) = {
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
	BSREQ(chance_consume), BSREQ(chance_ill),
	BSREQ(rotting),
	BSENM(wear, weari),
	BSREQ(wear_index),
	BSREQ(weapon),
	BSREQ(use),
	BSREQ(powers), BSREQ(chance_power),
	BSREQ(cursed),
	BSREQ(required),
	{}};
BSMETA(locationi) = {
	BSREQ(id),
	BSREQ(global), BSREQ(global_finish),
	BSREQ(local),
	BSREQ(landscape), BSREQ(sites), BSREQ(loot), BSREQ(monsters),
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
BSMETA(monsteri) = {
	BSREQ(id),
	BSDST(abilities, abilityi),
	BSREQ(appear),
	BSREQ(use),
	BSREQ(avatar),
	BSREQ(parent),
	BSREQ(minions),
	BSREQ(rest),
	{}};
BSMETA(point) = {
	BSREQ(x), BSREQ(y),
	{}};
BSMETA(querryi) = {
	BSREQ(id),
	{}};
BSMETA(shapei) = {
	BSREQ(id),
	{}};
BSMETA(sitei) = {
	BSREQ(id),
	BSREQ(local),
	BSREQ(landscape), BSREQ(loot), BSREQ(wearing),
	BSREQ(chance_hidden_doors), BSREQ(chance_locked_doors), BSREQ(chance_stuck_doors),
	BSREQ(doors_count),
	BSREQ(shape),
	BSFLG(feats, feati),
	BSENM(walls, tilei),
	BSENM(floors, tilei),
	BSENM(doors, featurei),
	{}};
BSMETA(siteskilli) = {
	BSREQ(id),
	BSENM(skill, abilityi),
	BSREQ(bonus),
	BSREQ(effect),
	BSREQ(keyid),
	BSREQ(tool),
	{}};
BSMETA(spelli) = {
	BSREQ(id),
	BSREQ(mana), BSREQ(adventure), BSREQ(counterspell),
	BSREQ(summon), BSREQ(use),
	{}};
BSMETA(talki) = {
	BSREQ(id),
	{}};
BSMETA(weaponi) = {
	BSREQ(damage), BSREQ(pierce), BSREQ(ammunition),
	{}};
BSMETA(visualeffect) = {
	BSREQ(id),
	{}};
BSMETA(weari) = {
	BSREQ(id),
	{}};
BSMETA(widget) = {
	BSREQ(id),
	{}};
BSDATA(varianti) = {
	{"NoVariant"},
	{"Ability", VAR(abilityi), 1, 0, fiscript<abilityi>, fitest<abilityi>},
	{"Advancement", VAR(advancement), 2},
	{"AreaFlag", VAR(areafi), 1, 0, fiscript<areafi>},
	{"Craft", VAR(crafti), 1, 0, fiscript<crafti>},
	{"Creature", VAR(creature), 0},
	{"Charname", VAR(charname), 2, 0, 0, 0, read_charname},
	{"Feat", VAR(feati), 1, 0, fiscript<feati>, fitest<feati>},
	{"Feature", VAR(featurei), 1, 0, fiscript<featurei>, 0, 0, 0, fnfilter<featurei>},
	{"Filter", VAR(filteri), 1},
	{"Global", VAR(globali), 1, 0, fiscript<globali>},
	{"Hotkey", VAR(hotkey), 2, 0, 0, 0, 0, check_hotkeys},
	{"Item", VAR(itemi), 1, 0, fiscript<itemi>},
	{"List", VAR(listi), 1, 0, fiscript<listi>, fitest<listi>},
	{"Location", VAR(locationi), 1, 0, fiscript<locationi>},
	{"Modifier", VAR(modifieri), 1, 0, fiscript<modifieri>, fitest<modifieri>},
	{"Monster", VAR(monsteri), 1, 0, fiscript<monsteri>, fitest<monsteri>, 0, check_monsters},
	{"Need", VAR(greatneedi), 1, 0, 0, 0, 0, check_need_objects},
	{"NeedFlag", VAR(needni), 1, 0, fiscript<needni>, fitest<needni>},
	{"Querry", VAR(querryi), 1, 0, fiscript<querryi>, fitest<querryi>},
	{"Quest", VAR(quest), 0},
	{"Race", VAR(racei), 1, 0, fiscript<racei>},
	{"RandomTable", VAR(randomizeri), 1, 0, fiscript<randomizeri>},
	{"Script", VAR(script), 1, 0, fiscript<script>},
	{"Shape", VAR(shapei), 1, 0, fiscript<shapei>, 0, shapei::read},
	{"Site", VAR(sitei), 1, 0, fiscript<sitei>},
	{"SiteSkill", VAR(siteskilli), 1, 0, 0, 0, 0, check_site_skills},
	{"Spell", VAR(spelli), 1, 0, fiscript<spelli>},
	{"Speech", VAR(speechi), 1, 0, fiscript<speechi>, 0, speech_read},
	{"Talk", VAR(talki), 1, 0, 0, 0, read_talk},
	{"Tile", VAR(tilei), 1, 0, fiscript<tilei>},
	{"TileFlag", VAR(tilefi), 1},
	{"Trigger", VAR(trigger), 3},
	{"Variant", VAR(varianti), 1},
	{"VisualEffect", VAR(visualeffect), 1},
	{"Widget", VAR(widget), 1, 0, fiscript<widget>},
};
BSDATAF(varianti)