#include "creature.h"
#include "game.h"
#include "race.h"

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
	if(value.max!=value.min)
		sb.adds("%1i-%2i", value.min, value.max);
	else
		sb.adds("%1i", value.min);
}

static void addf(stringbuilder& sb, ability_s i, int value, int value_maximum = 0) {
	switch(i) {
	case Hits: case Mana:
		sb.addn("[~%1]\t%2i/%3i", getnameshort(i), value, value_maximum);
		break;
	default:
		if(i>=WeaponSkill && i<= ShieldUse)
			sb.addn("[~%1]\t%2i%%", getnameshort(i), value);
		else
			sb.addn("[~%1]\t%2i", getnameshort(i), value);
		break;
	}
}

static const char* getrace(variant v, bool female) {
	if(female) {
		char temp[260]; stringbuilder sb(temp);
		sb.add("%1Female", bsdata<racei>::elements[v.value].id);
		return getnm(temp);
	} else
		return getnm(bsdata<racei>::elements[v.value].id);
}

void creature::getinfo(stringbuilder& sb) const {
	sb.addn(getname());
	sb.addn(getrace(getkind(), is(Female)));
	sb.addn("%1 %2i [~%-Level]", getnm(getclass().id), get(Level));
	sb.addn("$Tab -40");
	sb.addn("---");
	for(auto i = Strenght; i <= Charisma; i = (ability_s)(i + 1))
		addf(sb, i, abilities[i]);
	sb.addn("---");
	for(auto i = WeaponSkill; i <= ShieldUse; i = (ability_s)(i + 1))
		addf(sb, i, abilities[i]);
	sb.addn("---");
	addf(sb, Armor, abilities[Armor]);
	addf(sb, Hits, abilities[Hits], abilities[HitsMaximum]);
	addf(sb, Mana, abilities[Mana], abilities[ManaMaximum]);
	addf(sb, Money, getmoney());
	sb.addn("---");
	sb.addn("[~%1]\t%2i", getnm("Rounds"), game.getminutes());
}

static void addv(stringbuilder& sb, const char* id, int value, const char* format = 0) {
	if(!value)
		return;
	if(!format)
		format = "[~%-1%+2i]";
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

void item::getinfo(stringbuilder& sb) const {
	auto& ei = geti();
	sb.adds(getfullname());
	addv(sb, Damage, get(FO(itemstat, damage)));
	addv(sb, "Pierce", get(FO(itemstat, pierce)));
	addv(sb, "Parry", get(FO(itemstat, parry)));
	addv(sb, "EnemyParry", get(FO(itemstat, enemy_parry)));
	addv(sb, "Speed", get(FO(itemstat, speed)));
	switch(ei.wear) {
	case Torso:
		addv(sb, Armor, get(FO(itemstat, armor)));
		break;
	}
	//addv(sb, ei.feats);
}

void creature::getrumor(quest& e, stringbuilder& sb) const {
	char temp[64]; stringbuilder sba(temp);
	auto direction = area.getdirection(game.position, e.position);
	auto range = game.getrange(game.position, e.position);
	auto site_name = e.level.getname();
	sba.adjective(e.modifier.getname(), stringbuilder::getgender(site_name));
	auto part_one = "RumorDungeon";
	if(!range)
		part_one = "RumorDungeonHere";
	actvf(sb, getname(), is(Female), 0,
		getnm(part_one),
		getnm(bsdata<directioni>::elements[direction].id),
		site_name,
		temp,
		range);
	monsteri* guardian = e.problem;
	if(guardian) {
		actvf(sb, getname(), is(Female), ' ',
			getnm("RumorDungeonMore"),
			e.reward.getname(),
			guardian->minions->getname());
		actvf(sb, getname(), is(Female), ' ',
			getnm("RumorDungeonGuardian"),
			guardian->getname());
	}
}

void roomi::getrumor(stringbuilder& sb) const {
	char temp[64]; stringbuilder sba(temp);
	auto direction = area.getdirection(player->getposition(), center());
	auto site_name = getname();
	sb.add(getnm("RumorLocation"),
		getnm(bsdata<directioni>::elements[direction].id),
		site_name);
	area.set(rc, &areamap::setflag, Explored);
}