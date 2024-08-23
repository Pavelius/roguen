#include "areapiece.h"
#include "creature.h"
#include "crt.h"
#include "game.h"
#include "greatneed.h"
#include "race.h"
#include "textscript.h"

static const char* getnameshort(const char* id) {
	auto pn = getnme(str("%1Short", id));
	if(pn)
		return pn;
	return getnm(id);
}

static const char* getnameshort(ability_s i) {
	return getnameshort(bsdata<abilityi>::elements[i].id);
}

static void addv(stringbuilder& sb, const dice& value) {
	if(value.max != value.min)
		sb.adds("%1i-%2i", value.min, value.max);
	else
		sb.adds("%1i", value.min);
}

static void addv(stringbuilder& sb, const char* id, int value, const char* format = 0) {
	if(!value)
		return;
	if(!format) {
		if(value!=0)
			format = "[~%-1%+2i]";
		else
			format = "[~%-1]";
	}
	sb.adds(format, getnameshort(id), value);
}

static void addv(stringbuilder& sb, const char* id) {
	sb.adds(getnm(id));
}

static void addv(stringbuilder& sb, ability_s id, int value) {
	addv(sb, bsdata<abilityi>::elements[id].id, value, "%-1 [%2i]");
}

static void addv(stringbuilder& sb, const featable& feats) {
	for(auto v = 0; v < 32; v++) {
		if(feats.is(v))
			addv(sb, bsdata<feati>::elements[v].id);
	}
}

static void addf(stringbuilder& sb, ability_s i, int value, int value_maximum = 0) {
	switch(i) {
	case Armor:
		sb.addn("[~%1]\t%2i", getnameshort(i), value);
		if(value_maximum > 0)
			sb.add("-%1i", value + value_maximum);
		break;
	case Hits: case Mana: case DamageMelee:
		sb.addn("[~%1]\t%2i/%3i", getnameshort(i), value, value_maximum);
		break;
	default:
		if(i >= WeaponSkill && i <= Dodge)
			sb.addn("[~%1]\t%2i%%", getnameshort(i), value);
		else
			sb.addn("[~%1]\t%2i", getnameshort(i), value);
		break;
	}
}

static const char* time_left(unsigned end_stamp) {
	auto stamp = game.getminutes();
	if(end_stamp <= stamp)
		return getnm("FewTime");
	auto value = (end_stamp - stamp) / (24 * 60);
	if(value > 0)
		return str("%1i %-2", value, stringbuilder::getbycount("Day", value));
	value = (end_stamp - stamp) / 60;
	if(value > 0)
		return str("%1i %-2", value, stringbuilder::getbycount("Hour", value));
	value = end_stamp - stamp;
	return str("%1i %-2", value, stringbuilder::getbycount("Minute", value));
}

static void addf(stringbuilder& sb, const char* id, int value, int value_maximum = 0) {
	sb.addn("[~%1]\t%2i", getnm(id), value);
}

static void wearing(stringbuilder& sb, const variants source);

static void wearing(stringbuilder& sb, variant v) {
	if(v.iskind<abilityi>())
		addv(sb, bsdata<abilityi>::elements[v.value].id, v.counter, 0);
	if(v.iskind<listi>())
		wearing(sb, bsdata<listi>::elements[v.value].elements);
	//else if(v.iskind<feati>()) {
	//	addv(sb, bsdata<feati>::elements[v.value].id);
}

static void wearing(stringbuilder& sb, const variants source) {
	for(auto v : source)
		wearing(sb, v);
}

static const char* getrace(variant v, bool female) {
	if(v.iskind<racei>()) {
		if(female) {
			char temp[260]; stringbuilder sb(temp);
			sb.add("%1Female", bsdata<racei>::elements[v.value].id);
			return getnm(temp);
		} if(v.iskind<racei>())
			return getnm(bsdata<racei>::elements[v.value].id);
	} else if(v.iskind<monsteri>())
		return getnm(bsdata<monsteri>::elements[v.value].id);
	return "None";
}

void creature::getinfo(stringbuilder& sb) const {
	sb.addn(getname());
	sb.addn(getrace(getkind(), is(Female)));
	sb.addn("%1i %-Level", get(Level));
	sb.addn("$tab -50");
	sb.addn("---");
	for(auto i = Strenght; i <= Wits; i = (ability_s)(i + 1))
		addf(sb, i, abilities[i]);
	sb.addn("---");
	for(auto i = WeaponSkill; i <= Dodge; i = (ability_s)(i + 1))
		addf(sb, i, abilities[i]);
	sb.addn("---");
	addf(sb, Armor, abilities[Armor], abilities[Block]);
	addf(sb, DamageMelee, abilities[DamageMelee] + player->wears[MeleeWeapon].geti().weapon.damage, abilities[DamageRanged] + player->wears[RangedWeapon].geti().weapon.damage);
	addf(sb, Hits, abilities[Hits], basic.abilities[Hits]);
	addf(sb, Mana, abilities[Mana], basic.abilities[Mana]);
	addf(sb, "Coins", getmoney());
	addf(sb, "Experience", experience);
	sb.addn("---");
	sb.addn("[~%1]\t%2i", getnm("Rounds"), game.getminutes());
}

void item::getinfo(stringbuilder& sb) const {
	auto& ei = geti();
	sb.adds(getfullname());
	addv(sb, "Damage", ei.weapon.damage);
	addv(sb, "Pierce", ei.weapon.pierce);
	wearing(sb, ei.wearing);
}

void creature::getrumor(quest& e, stringbuilder& sb) const {
	char temp[64]; stringbuilder sba(temp);
	auto direction = area->getdirection(game.position, e.position);
	auto range = game.position.range(e.position);
	auto site_name = e.level.getname();
	sba.adjective(e.modifier.getname(), stringbuilder::getgender(site_name));
	auto part_one = "RumorDungeon";
	if(!range)
		part_one = "RumorDungeonHere";
	actvf(sb, 0,
		getnm(part_one),
		getnm(bsdata<directioni>::elements[direction].id),
		site_name,
		temp,
		range);
	monsteri* guardian = e.problem;
	if(guardian) {
		actvf(sb, ' ',
			getnm("RumorDungeonMore"),
			e.reward.getname(),
			guardian->minions->getname());
		actvf(sb, ' ',
			getnm("RumorDungeonGuardian"),
			guardian->getname());
	}
}

void roomi::getrumor(stringbuilder& sb) const {
	char temp[64]; stringbuilder sba(temp);
	auto direction = area->getdirection(player->getposition(), center());
	auto site_name = getname();
	sb.add(getnm("RumorLocation"),
		getnm(bsdata<directioni>::elements[direction].id),
		site_name);
	area->set(rc, &areamap::setflag, Explored);
}

static const char* visualize_progress(int score) {
	if(score == 0)
		return "NoAnyProgress";
	else if(score < 40)
		return "LittleProgress";
	else if(score < 70)
		return "HalfwayProgress";
	else
		return "AlmostFinishProgress";
}

static void actual_need_state(stringbuilder& sb) {
	if(!last_need)
		return;
	sb.add(getnm("VisualizeProgress"), getnm(visualize_progress(last_need->score)), time_left(last_need->deadline));
}

static void print_reputation(stringbuilder& sb) {
	if(!player)
		return;
	auto v = player->get(Reputation) / 20;
	if(v < 0)
		sb.add(getnm(str("ReputationM%1i", -v)));
	else
		sb.add(getnm(str("Reputation%1i", v)));
}

static void list_of_feats(stringbuilder& sb) {
	for(auto i = (feat_s)0; i <= Blooding; i = (feat_s)(i + 1)) {
		if(i == Female || i == Ally)
			continue;
		if(player->is(i))
			sb.addn(bsdata<feati>::elements[i].getname());
	}
}

static void list_of_skills(stringbuilder& sb) {
	for(auto i = FirstSkill; i <= LastSkill; i = (ability_s)(i + 1)) {
		auto v = player->get(i);
		if(v)
			sb.addn("%1\t%2i%%", bsdata<abilityi>::elements[i].getname(), v);
	}
}

static void need_help_info(stringbuilder& sb) {
	if(!last_need)
		return;
	auto pn = getdescription(last_need->geti().getid());
	if(!pn)
		return;
	sb.add(pn, time_left(last_need->deadline));
}

BSDATA(textscript) = {
	{"ActualNeedState", actual_need_state},
	{"ListOfFeats", list_of_feats},
	{"ListOfSkills", list_of_skills},
	{"NeedHelpIntro", need_help_info},
	{"Reputation", print_reputation},
};
BSDATAF(textscript)