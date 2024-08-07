#include "areapiece.h"
#include "boost.h"
#include "charname.h"
#include "creature.h"
#include "direction.h"
#include "indexa.h"
#include "itema.h"
#include "modifier.h"
#include "game.h"
#include "pushvalue.h"
#include "script.h"
#include "siteskill.h"
#include "speech.h"
#include "trigger.h"
#include "triggern.h"

extern collection<roomi> rooms;

void apply_spell(const spelli& ei, int level);
bool choose_targets(const variants& effects);
void damage_item(item& it);
int getfloor();

bool creature::fixaction(const char* id, const char* action, ...) const {
	const char* result = 0;
	if(canspeak()) {
		auto pm = getmonster();
		if(pm) {
			speech_get(result, id, action, pm->getid(), 0);
			if(pm->parent)
				speech_get(result, id, action, pm->parent->getid(), 0);
		} else
			speech_get(result, id, action, getkind().getid(), 0);
		speech_get(result, id, action, 0, 0);
		if(result) {
			sayv(console, result, xva_start(action));
			return true;
		}
	}
	auto pn = getdescription(str("%1%2", id, action));
	if(pn) {
		actv(console, pn, xva_start(action), getname(), is(Female), ' ');
		return true;
	}
	return false;
}

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static void fixdamage(const creature* p, int total, int damage_weapon, int damage_armor, int damage_bonus) {
	p->logs(getnm("ApplyDamage"), total, damage_weapon, damage_armor);
}

static creature* findalive(point m) {
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid() && e.getposition() == m)
			return &e;
	}
	return 0;
}

static void pay_movement(creature* player) {
	auto cost = 100;
	if(!player->is(Fly)) {
		auto& ei = area->getfeature(player->getposition());
		if(ei.movedifficult)
			cost = cost * ei.movedifficult / 100;
	}
	player->waitseconds(cost);
}

static void pay_attack(creature* player, const item& weapon) {
	auto cost = 120 - weapon.geti().weapon.speed * 4 - player->get(Dexterity) / 4;
	if(cost < 20)
		cost = 20;
	player->waitseconds(cost);
}

static ability_s damage_ability(ability_s v) {
	switch(v) {
	case BalisticSkill: return DamageRanged;
	default: return DamageMelee;
	}
}

static bool attack_effect(const creature* p, const item& w, feat_s v) {
	if(w.is(v))
		return true;
	return p->is(v);
}

bool creature::resist(feat_s resist, feat_s immunity) const {
	if(is(immunity))
		return true;
	if(is(resist)) {
		if(d100() < 50)
			return true;
	}
	return false;
}

bool creature::resist(feat_s feat) const {
	auto immunity = bsdata<feati>::elements[feat].immunity;
	if(!immunity)
		return false;
	return resist(feat, immunity);
}

static void poison_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->resist(PoisonResistance, PoisonImmunity))
		return;
	if(player->roll(Strenght, -value))
		return;
	auto v = player->get(Poison) + value;
	player->fixeffect("PoisonVisual");
	if(v >= player->basic.abilities[Hits])
		player->kill();
	else
		player->set(Poison, v);
}

static void poison_attack(const creature* player, creature* enemy, const item& weapon) {
	auto strenght = 0;
	if(attack_effect(player, weapon, WeakPoison))
		strenght += xrand(1, 2);
	if(attack_effect(player, weapon, StrongPoison))
		strenght += xrand(2, 4);
	if(attack_effect(player, weapon, DeathPoison))
		strenght += xrand(3, 6);
	poison_attack(enemy, strenght);
}

static void illness_attack(creature* player, int value) {
	if(value <= 0)
		return;
	if(player->resist(DiseaseResist, DiseaseImmunity))
		return;
	auto v = player->get(Illness) + value;
	if(v >= player->get(Strenght))
		player->kill();
	else
		player->set(Illness, v);
}

static void damage_backpack_item(wear_s type, int chance) {
	if(d100() >= chance)
		return;
	itema source;
	source.selectbackpack(player);
	source.matchf(Potion, true);
	auto pi = source.random();
	if(pi)
		pi->damage();
}

void damage_equipment(int bonus, bool allow_save) {
	static wear_s equipment_order[] = {Torso, Backward, Head, Elbows, Neck, Girdle, Gloves, Legs, FingerRight, FingerLeft};
	for(auto w : equipment_order) {
		auto& e = player->wears[w];
		if(bonus <= 0)
			break;
		if(!e)
			continue;
		if(d100() < 30)
			continue;
		bonus--;
		if(allow_save && player->resist(AcidResistance, AcidImmunity))
			continue;
		e.damage();
	}
}

static void special_spell_attack(creature* player, item& weapon, creature* enemy, const spelli& ei) {
	ei.apply(1, 1, false, true);
	weapon.damage();
}

static void attack_effect_stun(creature* enemy) {
	if(!enemy->resist(StunResistance, StunImmunity)) {
		enemy->set(Stun);
		enemy->fixeffect("SearchVisual");
	}
}

static void special_attack(creature* player, item& weapon, creature* enemy, int& pierce, int& damage) {
	if(attack_effect(player, weapon, Vorpal)) {
		if(!enemy->resist(DeathResistance, DeathImmunity)) {
			damage = 100;
			pierce = 100;
		}
	}
	if(attack_effect(player, weapon, BleedingHit))
		enemy->set(Blooding);
	if(attack_effect(player, weapon, StunningHit))
		attack_effect_stun(enemy);
	if(attack_effect(player, weapon, PierceHit))
		pierce += 4;
	if(attack_effect(player, weapon, MightyHit))
		damage += 2;
	if(attack_effect(player, weapon, ColdDamage)) {
		enemy->add(Freezing, 2);
		area->setflag(player->getposition(), Iced);
	}
	if(attack_effect(player, weapon, FireDamage))
		enemy->add(Burning, 2);
	if(attack_effect(player, weapon, AcidDamage))
		enemy->add(Corrosion, 2);
	auto power = weapon.getpower();
	if(power.iskind<spelli>() && weapon.ischarge())
		special_spell_attack(player, weapon, enemy, bsdata<spelli>::elements[power.value]);
	// Damage equipment sometime
	if(d100() < 30) {
		pushvalue push_player(player, enemy);
		damage_equipment(1, false);
	}
}

static void restore(creature* player, ability_s a, ability_s test) {
	auto v = player->get(a);
	auto mv = player->basic.abilities[a];
	if(v < mv) {
		if(player->roll(test))
			player->add(a, 1);
	}
}

static void add(creature* player, ability_s id, int value, int minimal = 0) {
	auto i = player->get(id) + value;
	if(i < minimal)
		i = minimal;
	player->set(id, i);
}

static void posion_recovery(creature* player, ability_s v) {
	if(player->get(v) > 0) {
		add(player, v, -1);
		if(player->is(PoisonResistance))
			add(player, v, -1);
		if(!player->roll(Strenght)) {
			player->fixeffect("PoisonVisual");
			player->damage(1);
		}
	}
}

static direction_s movedirection(point m) {
	if(m.x < 0)
		return West;
	else if(m.y < 0)
		return North;
	else if(m.y >= area->mps)
		return South;
	else
		return East;
}

static void drop_wears(creature* player, int chance) {
	point pt = player->getposition();
	for(auto& e : player->wears) {
		if(!e || e.is(Natural))
			continue;
		if(d100() >= chance)
			continue;
		e.drop(pt);
	}
}

static void drop_treasure(creature* pe) {
	if(pe->is(Summoned))
		return;
	drop_wears(pe, 15);
	monsteri* p = pe->getkind();
	if(!p)
		return;
	pushvalue push_player(player, pe);
	pushvalue push_rect(last_rect, pe->getposition().rectangle());
	script_run(p->treasure);
}

static void nullify_elements(creature* player) {
	if(player->is(Burning) || player->is(Freezing)) {
		player->set(Burning, 0);
		player->set(Freezing, 0);
	}
}

static void check_burning(creature* player) {
	if(player->is(Burning)) {
		if(!player->resist(FireResistance, FireImmunity))
			player->damage(xrand(1, 3));
		player->add(Burning, -1);
	}
}

static void check_freezing(creature* player) {
	if(player->is(Freezing)) {
		if(!player->resist(ColdResistance, ColdImmunity)) {
			player->damage(1);
			player->slowdown(100 / 2);
			damage_backpack_item(Potion, 20);
		}
		player->add(Freezing, -1);
	}
}

static void check_corrosion(creature* p) {
	if(p->is(Corrosion)) {
		if(!player->resist(AcidResistance, AcidImmunity)) {
			player->damage(player->get(Corrosion));
			damage_equipment(1, true);
		}
		player->add(Corrosion, -1);
	}
}

static void check_blooding(creature* p) {
	if(p->is(Blooding)) {
		p->damage(1);
		area->setflag(p->getposition(), Blooded);
		if(p->roll(Strenght))
			p->remove(Blooding);
	}
}

static void check_stun(creature* p) {
	if(p->is(Stun)) {
		if(p->roll(Strenght))
			p->remove(Stun);
		if(p->is(StunResistance) && p->roll(Strenght))
			p->remove(Stun);
	}
}

static void check_satiation(creature* p) {
	if(p->satiation > 0)
		p->satiation--;
}

static void random_walk(creature* p) {
	if(d100() < 60)
		p->wait();
	else {
		static direction_s allaround[] = {North, South, East, West};
		p->movestep(maprnd(allaround));
	}
}

static bool check_stairs_movement(creature* p, point m) {
	auto& ei = area->getfeature(m);
	auto pf = ei.getlead();
	if(pf) {
		if(p->ishuman()) {
			if(draw::yesno(getnm(str("Move%1", ei.id)))) {
				game.enter(game.position, game.level + ei.lead, pf, Center);
				return false;
			}
		}
	}
	return true;
}

static bool check_dangerous_feature(creature* p, point m) {
	auto& ei = area->getfeature(m);
	if(ei.is(DangerousFeature)) {
		p->wait(2);
		if(!p->roll(Strenght)) {
			p->act(getnme(str("%1Entagled", ei.id)));
			p->damage(1);
			p->wait();
			p->fixactivity();
			return false;
		}
		p->act(getnme(str("%1Break", ei.id)));
		area->setfeature(m, 0);
	}
	return true;
}

static void check_trap(creature* player, point m) {
	if(player->is(Fly))
		return;
	auto& ei = area->getfeature(m);
	if(ei.is(TrappedFeature)) {
		auto bonus = area->is(m, Hidden) ? -30 : 20;
		if(!player->roll(Wits, bonus)) {
			auto pn = getdescription(ei.id);
			if(pn)
				player->act(pn, getnm(ei.id));
			player->apply(ei.effect);
		}
		if(!player->is(Local))
			area->remove(m, Hidden);
	}
}

static bool check_webbed_tile(creature* p, point m) {
	if(p->is(IgnoreWeb))
		return true;
	if(p->is(Webbed)) {
		p->wait(2);
		if(!p->roll(Strenght)) {
			p->act(getnm("WebEntagled"));
			p->wait();
			p->fixactivity();
			return false;
		}
		p->act(getnm("WebBreak"));
		area->remove(m, Webbed);
	}
	return true;
}

static bool check_leave_area(creature* p, point m) {
	if(!area->isvalid(m) && game.level == 0) {
		if(p->ishuman()) {
			auto direction = movedirection(m);
			auto np = to(game.position, direction);
			if(draw::yesno(getnm("LeaveArea"), getnm(bsdata<directioni>::elements[direction].id)))
				game.enter(np, 0, 0, direction);
		}
		p->wait();
		return false;
	}
	return true;
}

static bool check_place_owner(creature* p, point m) {
	if(p->is(PlaceOwner)) {
		auto pr = roomi::find(m);
		if(p->getroom() != pr) {
			p->wait();
			return false;
		}
	}
	return true;
}

static void detect_hidden_objects(creature* player) {
	if(!last_location)
		return;
	auto floor = last_location->floors;
	indexa source;
	source.select(player->getposition(), imin(player->getlos(), 2));
	source.match(Visible, true);
	// Secrect doors
	for(auto m : source) {
		auto& ei = area->getfeature(m);
		if(ei.isvisible())
			continue;
		if(!player->roll(Alertness, -2000))
			continue;
		player->actp(getnm("YouFoundSecretDoor"));
		area->setreveal(m, floor);
		break;
	}
	// Traps
	for(auto m : source) {
		if(!area->is(m, Hidden))
			continue;
		auto& ei = area->getfeature(m);
		if(!ei.is(TrappedFeature))
			continue;
		if(!player->roll(Alertness, -2000))
			continue;
		area->remove(m, Hidden);
		player->actp(getdescription("YouDetectTrap"), area->getfeature(m).getname());
		break;
	}
}

static void drop_item(point m, const char* id) {
	if(!area->isvalid(m) || !id)
		return;
	itemi* pi = single(id);
	if(!pi)
		return;
	item it; it.create(pi);
	it.drop(m);
}

static bool check_stuck_doors(creature* p, point m, const featurei& ei) {
	if(!ei.is(StuckFeature))
		return false;
	if(p->roll(Strenght, -10)) {
		p->act(getnm("YouOpenStuckDoor"), getnm(ei.id));
		area->setactivate(m);
		area->setactivate(m);
	} else {
		auto random_table = bsdata<randomizeri>::find(str("%1%2", ei.id, "Fail"));
		if(random_table) {
			auto effect = random_table->random();
			if(effect.iskind<listi>()) {
				auto p = bsdata<listi>::elements + effect.value;
				auto pn = getdescription(p->id);
				if(pn)
					player->act(pn, getnm(ei.id));
				pushvalue push_rect(last_rect, {m.x, m.y, m.x, m.y});
				script_run(p->elements);
			}
		}
		movable::fixeffect(m2s(m), "SearchVisual");
		area->setfeature(m, 0);
	}
	return true;
}

bool check_activate(creature* player, point m, const featurei& ei) {
	auto pa = ei.getactivate();
	if(!pa)
		return false;
	auto activate_item = ei.activate_item;
	if(activate_item) {
		if(ei.random_count)
			activate_item.value += area->param[m] % ei.random_count;
		if(activate_item.iskind<itemi>()) {
			if(!player->useitem(bsdata<itemi>::elements + activate_item.value)) {
				player->actp(getnm(str("%1%2", ei.id, "NoActivateItem")), bsdata<itemi>::elements[activate_item.value].getname());
				return false;
			} else
				player->actp(getnm(str("%1%2", ei.id, "UseActivateItem")), bsdata<itemi>::elements[activate_item.value].getname());
		}
	}
	area->setactivate(m);
	return true;
}

static int chance_cut_wood(const item& weapon) {
	auto& ei = player->wears[MeleeWeapon].geti();
	auto chance = player->get(Strenght) / 10 + ei.weapon.damage;
	if(chance < 1)
		chance = 1;
	if(ei.is(TwoHanded))
		chance *= 2;
	return chance;
}

static bool check_cut_wood(creature* player, point m, const featurei& ei) {
	if(ei.is(Woods) && player->wears[MeleeWeapon].geti().is(CutWoods)) {
		if(player->roll(Woodcutting)) {
			player->act(getnm("YouCutWood"), getnm(ei.id));
			area->setfeature(m, 0);
			drop_item(m, "WoodenLagsTable");
			return true;
		}
	}
	return false;
}

static bool issame(point m, tilef f) {
	if(!area->isvalid(m))
		return false;
	return bsdata<tilei>::elements[area->tiles[m]].is(f);
}

static bool issame(point m, direction_s d, tilef f) {
	if(!issame(m, f))
		return false;
	auto m1 = to(m, round(d, East));
	auto m2 = to(m, round(d, West));
	if(!area->isvalid(m1) || !area->isvalid(m2))
		return false;
	if(bsdata<featurei>::elements[area->features[m1]].is(BetweenWalls))
		return false;
	if(bsdata<featurei>::elements[area->features[m2]].is(BetweenWalls))
		return false;
	return true;
}

static bool check_mining(creature* player, point m) {
	if(!player->wears[MeleeWeapon].geti().is(CutMines))
		return false;
	if(!issame(m, Mines))
		return false;
	auto direction = area->getdirection(player->getposition(), m);
	if(!issame(m, direction, Mines) || !m.in({2, 2, areamap::mps - 2, areamap::mps - 2})) {
		player->actp(getnm("YouCantMineHere"));
		return false;
	}
	if(player->roll(Mining)) {
		player->act(getnm("YouCrashWall"), getnm(bsdata<tilei>::elements[area->tiles[m]].id));
		area->setfeature(m, 0);
		area->settile(m, getfloor());
		if(d100() < 25) {
			drop_item(m, "MiningOre");
			if(player->roll(Gemcutting, -2000))
				drop_item(m, "MiningGem");
		}
		return true;
	} else if(d100() < 10)
		damage_item(player->wears[MeleeWeapon]);
	return false;
}

static void check_interaction(creature* player, point m) {
	auto& ei = area->getfeature(m);
	if(ei.isvisible()) {
		if(check_stuck_doors(player, m, ei))
			return;
		if(check_activate(player, m, ei))
			return;
		if(check_cut_wood(player, m, ei))
			return;
	}
}

static void look_items(creature* player, point m) {
	items.clear();
	items.select(m);
	if(!items)
		return;
	if(player->ishuman()) {
		auto index = 0;
		char temp[4096]; stringbuilder sb(temp);
		auto count = items.getcount();
		auto payment_cost = player->getpaymentcost();
		sb.add(getnm("ThereIsLying"));
		for(auto p : items) {
			if(index > 0) {
				if(index == count - 1)
					sb.adds(getnm("And"));
				else
					sb.add(",");
			}
			sb.adds("%-1", p->getfullname(payment_cost));
			index++;
		}
		sb.add(".");
		player->actp(temp);
	}
}

static void update_boost(featable& feats, variant parent) {
	feats.clear();
	for(auto& e : bsdata<boosti>()) {
		if(e.parent != parent)
			continue;
		if(e.effect.iskind<feati>())
			feats.set(e.effect.value);
	}
}

static void update_basic(char* dest, const char* source) {
	memcpy(dest, source, Hits * sizeof(statable::abilities[0]));
}

static bool spell_allowmana(const void* object) {
	auto p = (spelli*)object;
	return player->get(Mana) >= p->mana;
}

static bool spell_allowuse(const void* object) {
	auto p = (spelli*)object;
	if(p->summon.size() != 0)
		return true;
	return choose_targets(p->targets);
}

static bool spell_iscombat(const void* object) {
	auto p = (spelli*)object;
	if(p->summon)
		return true;
	if(p->ishostile())
		return true;
	return false;
}

static bool	spell_isnotcombat(const void* object) {
	return !spell_iscombat(object);
}

void creature::clear() {
	memset(this, 0, sizeof(*this));
	worldpos = {-1000, -1000};
	moveorder = {-1000, -1000};
	guardorder = {-1000, -1000};
	setroom(0);
	setowner(0);
	if(game.getowner() == this)
		game.setowner(0);
}

bool isfreecr(point m) {
	if(findalive(m))
		return false;
	auto tile = area->tiles[m];
	if(bsdata<tilei>::elements[tile].is(CanSwim))
		return false;
	return area->isfree(m);
}

bool isfreecrfly(point m) {
	if(findalive(m))
		return false;
	return area->isfree(m);
}

static void update_room(creature* player) {
	auto pn = roomi::find(player->getposition());
	if(pn) {
		auto pb = player->getroom();
		auto room_changed = false;
		if(pn != pb) {
			if(player->ishuman()) {
				auto pd = getdescription(pn->geti().id);
				if(pd)
					player->actp(pd);
			}
			room_changed = true;
		}
		player->setroom(pn);
		if(room_changed)
			fire_trigger(WhenCreatureP1EnterSiteP2, player->getkind(), &pn->geti());
	} else
		player->setroom(0);
}

static void update_skills() {
	for(auto& e : bsdata<abilityi>()) {
		// Pure skill get bonus if you learn it
		if(!e.base)
			continue;
		auto i = e.getindex();
		if(!player->basic.abilities[i])
			continue;
		player->abilities[i] += player->get(e.base) / 3;
	}
}

void creature::update_abilities() {
	abilities[DamageMelee] += get(Strenght) / 15;
	abilities[DamageRanged] += get(Dexterity) / 15;
	abilities[Armor] += get(Strenght) / 15;
	abilities[Speed] += 25 + get(Dexterity) / 5;
	if(is(Stun)) {
		abilities[WeaponSkill] /= 2;
		abilities[BalisticSkill] /= 2;
		abilities[Dodge] -= 40;
	}
	if(is(LightSource))
		abilities[LineOfSight] += 3;
	if(ispresent()) {
		if(!is(IgnoreWeb) && is(Webbed)) {
			abilities[WeaponSkill] -= 10;
			abilities[Dodge] -= 20;
		}
	}
}

void creature::place(point m) {
	if(!area)
		return;
	m = area->getfree(m, 10, isfreecr);
	setposition(m);
	update_room(this);
}

bool creature::ishuman() const {
	return game.getowner() == this;
}

bool creature::isenemy(const creature& opponent) const {
	return opponent.is(is(Enemy) ? Ally : Enemy);
}

bool creature::is(areaf v) const {
	if(!area)
		return false;
	return ispresent() && area->is(getposition(), v);
}

void creature::movestep(direction_s v) {
	if(is(Iced)) {
		if(!roll(Dexterity, 30)) {
			act(getnm("IcedSlice"));
			v = round(v, (d100() < 50) ? NorthWest : NorthEast);
			wait();
		}
	}
	setdirection(v);
	movestep(to(getposition(), v));
}

void creature::interaction(creature& opponent) {
	if(opponent.isenemy(*this))
		attackmelee(opponent);
	else if(!ishuman())
		pay_movement(this);
	else if(opponent.is(PlaceOwner)) {
		fixactivity();
		opponent.wait();
		if(d100() < 40 && opponent.canspeak())
			opponent.fixaction("DontPushMePlaceOwner", 0);
		pay_movement(this);
	} else {
		auto pt = opponent.getposition();
		opponent.setposition(getposition());
		opponent.fixmovement();
		opponent.wait();
		update_room(&opponent);
		setposition(pt);
		update_room(this);
		fixmovement();
		if(d100() < 40 && opponent.canspeak())
			opponent.fixaction("DontPushMe", 0);
		pay_movement(this);
	}
}

static void apply_damage(creature* player, const item& weapon, int effect) {
	auto weapon_damage = weapon.geti().weapon.damage;
	auto damage_reduction = player->get(Armor) - weapon.geti().weapon.pierce;
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage - damage_reduction + effect;
	fixdamage(player, result_damage, weapon_damage, -damage_reduction, effect);
	player->damage(result_damage);
}

static ability_s weapon_skill(const item& weapon) {
	auto& ei = weapon.geti();
	switch(ei.wear) {
	case RangedWeapon: return BalisticSkill;
	default: return WeaponSkill;
	}
}

static int add_bonus_damage(creature* player, creature* enemy, item& weapon, feat_s feat, int value, feat_s resistance, feat_s immunity) {
	if(!attack_effect(player, weapon, feat))
		return 0;
	auto bonus_damage = value;
	if(immunity && enemy->is(immunity))
		bonus_damage = 0;
	else if(resistance && enemy->is(resistance))
		bonus_damage /= 2;
	return bonus_damage;
}

static void apply_pierce(int& armor, int pierce) {
	if(armor > 0) {
		if(pierce > armor)
			armor = 0;
		else
			armor -= pierce;
	} else
		armor = 0;
}

static void apply_damage(creature* player, int damage, int pierce, bool use_block) {
	auto armor = player->get(Armor);
	apply_pierce(armor, pierce);
	auto damage_result = damage - armor;
	if(damage_result <= 0)
		return;
	if(use_block) {
		auto block_damage = player->get(Block);
		if(block_damage > 0) {
			damage_result -= xrand(0, block_damage);
			if(damage_result <= 0) {
				player->fixvalue(getnm("Block"), ColorGreen);
				return;
			}
		}
	}
	player->damage(damage_result);
}

static void make_attack(creature* player, creature* enemy, item& weapon, int attack_skill, int damage_percent) {
	auto roll_result = d100();
	auto enemy_name = enemy->getname();
	auto attacker_name = player->getname();
	auto weapon_ability = weapon_skill(weapon);
	auto damage = (int)weapon.geti().weapon.damage;
	damage += player->get(damage_ability(weapon_ability));
	damage += add_bonus_damage(player, enemy, weapon, FireDamage, 2, FireResistance, FireImmunity);
	damage += add_bonus_damage(player, enemy, weapon, ColdDamage, 2, ColdResistance, ColdImmunity);
	attack_skill += player->get(weapon_ability);
	if(damage_percent)
		damage = damage * damage_percent / 100;
	damage += (attack_skill - roll_result) / 10;
	if(roll_result > attack_skill) // Miss is hurt
		damage -= 1;
	if(damage < 0)
		damage = 0;
	auto pierce = (int)weapon.geti().weapon.pierce;
	if(roll_result < attack_skill / 3)
		special_attack(player, weapon, enemy, pierce, damage);
	auto armor = enemy->get(Armor);
	apply_pierce(armor, pierce);
	auto damage_result = damage - armor;
	if(damage_result > 0 && weapon.is(MissHalfTime) && (d100() < 50))
		damage_result = 0;
	if(damage_result <= 0) {
		player->logs(getnm("AttackMiss"), damage_result, enemy->getname(), roll_result, damage, -armor);
		return;
	}
	auto block_damage = enemy->get(Block);
	if(block_damage > 0) {
		damage_result -= xrand(0, block_damage);
		if(damage_result <= 0) {
			enemy->fixvalue(getnm("Block"), ColorGreen);
			return;
		}
	}
	if(enemy->roll(Dodge)) {
		player->logs(getnm("AttackHitButEnemyDodge"), enemy->getname());
		enemy->fixvalue(getnm("Dodge"), ColorGreen);
	} else {
		player->logs(getnm("AttackHit"), damage_result, enemy->getname(), roll_result, damage, -armor);
		enemy->damage(damage_result);
		poison_attack(player, enemy, weapon);
		if(attack_effect(enemy, enemy->wears[MeleeWeapon], Retaliate)) {
			if(!player->roll(Dodge))
				player->damage(1);
		}
	}
	if(roll_result >= 95 && d100() < 30)
		weapon.damage();
}

int	creature::getexpreward() const {
	static ability_s skills[] = {Strenght, Dexterity, Wits, WeaponSkill, BalisticSkill};
	auto result = get(Strenght) / 10;
	result += get(Dexterity) / 15;
	result += get(WeaponSkill) / 10;
	result += get(BalisticSkill) / 10;
	result += getmaximum(Hits) / 2;
	return result;
}

void creature::kill() {
	if(d100() < 40)
		area->setflag(getposition(), Blooded);
	logs(getnm("ApplyKill"));
	auto human_killed = ishuman();
	fixeffect("HitVisual");
	fixremove();
	drop_treasure(this);
	if(enemy == this && player)
		player->experience += getexpreward();
	fire_trigger(WhenCreatureP1Dead, getkind());
	clear();
	if(human_killed)
		game.endgame();
}

void creature::damage(int v) {
	if(v <= 0)
		return;
	fixvalue(-v);
	abilities[Hits] -= v;
	if(abilities[Hits] <= 0)
		kill();
}

void creature::attackmelee(creature& enemy) {
	fixactivity();
	auto number_attackers = enemy.get(EnemyAttacks);
	if(number_attackers > 3)
		number_attackers = 3;
	make_attack(this, &enemy, wears[MeleeWeapon], number_attackers * 10, 100);
	pay_attack(this, wears[MeleeWeapon]);
	enemy.add(EnemyAttacks, 1);
}

bool creature::canshoot(bool interactive) const {
	if(!wears[RangedWeapon]) {
		if(interactive)
			actp(getnm("YouNeedRangeWeapon"));
		return false;
	}
	auto ammo = wears[RangedWeapon].geti().weapon.ammunition;
	if(ammo && !wears[Ammunition].is(*ammo)) {
		if(interactive)
			actp(getnm("YouNeedAmmunition"), ammo->getname());
		return false;
	}
	return true;
}

bool creature::canthrown(bool interactive) const {
	if(!wears[MeleeWeapon].is(Thrown)) {
		if(interactive)
			actp(getnm("YouNeedThrownWeapon"));
		return false;
	}
	return true;
}

void creature::attackrange(creature& enemy) {
	if(!canshoot(false))
		return;
	auto pa = wears[RangedWeapon].geti().weapon.ammunition;
	if(pa)
		fixshoot(enemy.getposition(), pa->wear_index);
	else
		fixactivity();
	make_attack(this, &enemy, wears[RangedWeapon], 0, 100);
	pay_attack(this, wears[RangedWeapon]);
	if(pa) {
		wears[Ammunition].use();
		if(d100() < 50) {
			item it;
			it.create(pa, 1);
			it.setcount(1);
			it.drop(enemy.getposition());
		}
	}
}

void creature::attackthrown(creature& enemy) {
	if(!canthrown(false))
		return;
	fixthrown(enemy.getposition(), "FlyingItem", wears[MeleeWeapon].geti().getindex());
	make_attack(this, &enemy, wears[MeleeWeapon], 0, 100);
	pay_attack(this, wears[MeleeWeapon]);
	item it;
	it.create(&wears[MeleeWeapon].geti(), 1);
	it.setcount(1);
	it.drop(enemy.getposition());
	wears[MeleeWeapon].use();
}

void creature::fixcantgo() const {
	fixactivity();
	act(getnm("CantGoThisWay"));
}

static bool isfreeplace(creature* player, point ni) {
	if(player->is(Fly))
		return isfreecrfly(ni);
	return isfreecr(ni);
}

void creature::movestep(point ni) {
	if(!check_leave_area(this, ni))
		return;
	if(!check_place_owner(this, ni))
		return;
	auto m = getposition();
	if(!check_dangerous_feature(this, m))
		return;
	if(!check_stairs_movement(this, ni))
		return;
	if(!check_webbed_tile(this, m))
		return;
	auto opponent = findalive(ni);
	if(opponent)
		interaction(*opponent);
	else if(isfreeplace(this, ni)) {
		setposition(ni);
		update_room(this);
		fixmovement();
		look_items(this, getposition());
		pay_movement(this);
	} else {
		check_mining(this, ni);
		check_interaction(this, ni);
		fixactivity();
		pay_movement(this);
	}
	check_trap(this, getposition());
}

void creature::finish() {
	satiation += 2000;
	update();
	abilities[Hits] = getmaximum(Hits);
	abilities[Mana] = getmaximum(Mana);
	fixappear();
}

static int add_ability(ability_s i, int value, int current_value, int minimum, int maximum, bool interactive) {
	value += current_value;
	if(value < minimum)
		value = minimum;
	else if(value > maximum)
		value = maximum;
	auto delta = value - current_value;
	if(interactive && delta != 0) {
		auto color_positive = get_positive_color(i);
		auto color_negative = get_negative_color(i);
		if(color_negative != ColorNone)
			player->fixvalue(delta, color_positive, color_negative);
	}
	return value;
}

static void add_ability(ability_s v, int counter, bool interactive, bool basic) {
	if(v < sizeof(player->basic.abilities) / sizeof(player->basic.abilities[0])) {
		if(basic)
			player->basic.abilities[v] = add_ability(v, counter, player->basic.abilities[v], 0, 100, interactive);
		else if(v == Hits || v == Mana)
			player->abilities[v] = add_ability(v, counter, player->abilities[v], 0, player->basic.abilities[v], interactive);
		else
			player->abilities[v] = add_ability(v, counter, player->abilities[v], 0, 100, interactive);
	}
}

static int get_counter(int counter) {
	if(counter > 100)
		return xrand(1, counter - 100);
	return counter;
}

bool creature::roll(ability_s v, int bonus) const {
	auto value = get(v);
	if(value <= 0)
		return false;
	auto result = d100();
	if(bonus == -2000) {
		value = value / 2;
		bonus = 0;
	} else if(bonus == -3000) {
		value = value / 3;
		bonus = 0;
	}
	last_value = (value - result) / 10;
	if(result <= 5)
		return true;
	else if(result >= 95)
		return false;
	return result <= (value + bonus);
}

void adventure_mode();

static void look_creatures() {
	creatures.select(player->getposition(), player->getlos(), player->ishuman(), player);
	if(player->is(Ally)) {
		enemies = creatures;
		enemies.match(Enemy, true);
	} else if(player->is(Enemy)) {
		enemies = creatures;
		enemies.match(Ally, true);
	} else
		enemies.clear();
}

static void ready_enemy() {
	enemy = 0;
	if(enemies) {
		enemies.sort(player->getposition());
		enemy = enemies[0];
	}
}

static void ready_actions() {
	look_creatures();
	ready_enemy();
	allowed_spells.select(player);
	last_actions.clear();
}

static void ready_skills() {
	last_actions.select(fntis<siteskilli, &siteskilli::isusable>);
}

int creature::getloh() const {
	return 2 + get(Wits) / 10;
}

bool creature::canhear(point i) const {
	return area->getrange(getposition(), i) <= getloh();
}

bool creature::isfollowmaster() const {
	auto master = getowner();
	if(!master)
		return false;
	const int bound_range = 2;
	if(area->getrange(master->getposition(), getposition()) <= bound_range)
		return false;
	return true;
}

static const sitei* get_site(creature* p) {
	return p->getroom() ? &p->getroom()->geti() : 0;
}

void creature::makemovelong() {
	if(wait_seconds < 100 * 6)
		return;
	wait_seconds -= 100 * 6;
}

static void use_skills() {
	ready_skills();
	if(last_actions) {
		last_action = last_actions.random();
		script_execute("ApplyAction");
	} else
		player->wait();
}

void creature::makemove() {
	// Recoil form action
	if(wait_seconds > 0) {
		wait_seconds -= get(Speed);
		return;
	}
	pushvalue push_player(player, this);
	pushvalue push_rect(last_rect, getposition().rectangle());
	pushvalue push_site(last_site, get_site(this));
	pushvalue push_action(last_action);
	set(EnemyAttacks, 0);
	check_satiation(this);
	update();
	nullify_elements(this);
	check_blooding(this);
	if(!is(Local))
		detect_hidden_objects(this);
	check_burning(this);
	check_freezing(this);
	check_corrosion(this);
	ready_actions();
	if(ishuman()) {
		ready_skills();
		adventure_mode();
	} else if(enemy) {
		allowed_spells.match(spell_iscombat, true);
		allowed_spells.match(spell_allowmana, true);
		allowed_spells.match(spell_allowuse, true);
		if(allowed_spells && d100() < 40)
			cast(*((spelli*)allowed_spells.random()));
		else if(canshoot(false))
			attackrange(*enemy);
		else
			moveto(enemy->getposition());
	} else {
		allowed_spells.match(spell_isnotcombat, true);
		allowed_spells.match(spell_allowmana, true);
		allowed_spells.match(spell_allowuse, true);
		if(isfollowmaster())
			moveto(getowner()->getposition());
		else if(d100() < 20)
			use_skills();
		else if(allowed_spells && d100() < 20)
			cast(*((spelli*)allowed_spells.random()));
		else if(area->isvalid(moveorder)) {
			if(moveorder == getposition())
				moveorder = {-1000, -1000};
			else if(!moveto(moveorder))
				moveorder = {-1000, -1000};
		} else if(area->isvalid(guardorder)) {
			if(guardorder != getposition())
				moveorder = guardorder;
		} else
			random_walk(this);
	}
}

static void wearing(variants source, int multiplier);

static void wearing(variant v, int multiplier) {
	if(v.iskind<abilityi>())
		player->abilities[v.value] += v.counter * multiplier;
	else if(v.iskind<feati>()) {
		multiplier = v.counter * multiplier;
		if(multiplier >= 0)
			player->feats_active.set(v.value);
		else
			player->feats_active.remove(v.value);
	} else if(v.iskind<listi>())
		wearing(bsdata<listi>::elements[v.value].elements, multiplier);
}

static void wearing(variants source, int multiplier) {
	for(auto v : source)
		wearing(v, multiplier);
}

static void update_wears() {
	for(auto& e : player->equipment()) {
		if(!e)
			continue;
		wearing(e.geti().wearing, 1);
		auto power = e.getpower();
		if(power)
			wearing(power, bsdata<magici>::elements[e.getmagic()].multiplier);
	}
}

void creature::update_room_abilities() {
	auto p = getroom();
	if(!p)
		return;
	fire_trigger(WhenCreatureP1InSiteP2UpdateAbilities, getkind(), &p->geti());
}

static void update_negative_skills() {
	for(auto i = (ability_s)0; i < Hits; i = (ability_s)(i + 1)) {
		if(player->abilities[i] < 0)
			player->abilities[i] = 0;
	}
}

void creature::update() {
	auto push_player = player;
	player = this;
	update_basic(abilities, basic.abilities);
	update_boost(feats_active, this);
	update_wears();
	update_room_abilities();
	update_skills();
	update_abilities();
	update_negative_skills();
	player = push_player;
}

static void block_creatures(creature* exclude) {
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos != game)
			continue;
		if(&e == exclude)
			continue;
		area->setblock(e.getposition(), 0xFFFF);
	}
}

static void block_swimable() {
	for(auto& e : bsdata<tilei>()) {
		if(e.is(CanSwim))
			area->blocktiles(bsid(&e));
	}
}

bool creature::moveto(point ni) {
	area->clearpath();
	if(!is(Fly))
		block_swimable();
	area->blockwalls();
	area->blockfeatures();
	block_creatures(this);
	area->makewave(getposition());
	area->blockzero();
	auto m0 = getposition();
	auto m1 = area->getnext(m0, ni);
	if(!area->isvalid(m1))
		return false;
	movestep(area->getdirection(m0, m1));
	return true;
}

void creature::unlink() {
	boosti::remove(this);
	for(auto& e : bsdata<creature>()) {
		if(e.getowner() == this)
			e.setowner(0);
	}
}

void creature::act(const char* format, ...) const {
	if(ishuman() || is(Visible))
		actv(console, format, xva_start(format), getname(), is(Female), '\n');
}

void creature::actp(const char* format, ...) const {
	if(ishuman())
		actv(console, format, xva_start(format), getname(), is(Female), '\n');
}

void creature::sayv(stringbuilder& sb, const char* format, const char* format_param) const {
	if(ishuman() || is(Visible))
		actable::sayv(sb, format, format_param, getname(), is(Female));
}

int	creature::getlos() const {
	auto r = get(LineOfSight) - area->darkness;
	auto m = 1;
	if(is(Darkvision))
		m++;
	// Local folk knows his place better that you
	if(is(Local))
		m++;
	if(r < m)
		r = m;
	if(r > 4)
		r = 4;
	return r;
}

bool creature::isvalid() const {
	return worldpos == game;
}

bool creature::speechrumor() const {
	collection<quest> source;
	source.select();
	if(!source)
		return false;
	auto seed = game.getminutes() / (60 * 12);
	auto p = (quest*)source.data[seed % source.getcount()];
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	getrumor(*p, sb);
	say(temp);
	return true;
}

bool creature::speechlocation() const {
	rooma source;
	source.collectiona::select(area->rooms, fntis<roomi, &roomi::isnotable>);
	source.match(fntis<roomi, &roomi::isexplored>, false);
	auto p = source.random();
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	p->getrumor(sb);
	say(temp);
	return true;
}

void advance_value(variant v) {
	if(v.iskind<abilityi>())
		player->basic.abilities[v.value] += get_counter(v.counter);
	else if(v.iskind<itemi>()) {
		if(v.counter >= 0)
			player->wearable::equipi(v.value, v.counter ? v.counter : 1);
	} else if(v.iskind<feati>())
		ftscript<feati>(v.value, v.counter);
	else if(v.iskind<spelli>())
		player->spells[v.value] += v.counter;
	else if(v.iskind<modifieri>())
		modifier = (modifiers)v.value;
	else if(v.iskind<script>())
		bsdata<script>::elements[v.value].proc(v.counter);
}

static void advance_value(variants elements) {
	for(auto v : elements)
		advance_value(v);
}

static void advance_value(variant kind, int level) {
	for(auto& e : bsdata<advancement>()) {
		if(e.type == kind && e.level == level) {
			advance_value(e.elements);
			player->update();
		}
	}
}

static void apply_value(variant v) {
	if(v.iskind<spelli>())
		ftscript<spelli>(v.value, v.counter);
	else if(v.iskind<areafi>()) {
		if(v.counter < 0)
			area->remove(player->getposition(), v.value);
		else
			area->setflag(player->getposition(), v.value);
	} else if(v.iskind<featurei>()) {
		if(v.counter < 0)
			area->setfeature(player->getposition(), 0);
		else
			area->setfeature(player->getposition(), v.value);
	} else
		advance_value(v);
}

static void apply_value(variant v, creature* target) {
	pushvalue push_modifier(modifier);
	pushvalue push_player(player, target);
	apply_value(v);
}

void creature::use(item& v) {
	if(!v)
		return;
	auto script = v.getuse();
	if(!script) {
		actp(getnm("ItemNotUsable"), v.getname());
		return;
	}
	auto push_item = last_item;
	last_item = &v;
	act(getnm("YouUseItem"), v.getname());
	apply(script);
	auto power = v.getpower();
	if(power)
		apply_value(power, this);
	v.use();
	last_item = push_item;
	update();
	wait();
}

void creature::everyminute() {
	if(is(Regeneration))
		restore(this, Hits, Strenght);
	if(is(ManaRegeneration))
		restore(this, Mana, Wits);
	check_stun(this);
	posion_recovery(this, Poison);
}

void creature::every10minutes() {
	restore(this, Mana, Wits);
}

void creature::every30minutes() {
}

void creature::every4hour() {
	restore(this, Hits, Strenght);
}

void apply_ability(ability_s v, int counter) {
	last_ability = v;
	if(!counter)
		return;
	add_ability(v, counter, true, false);
	if(player->abilities[Hits] <= 0)
		player->kill();
}

void creature::apply(const variants& source) {
	pushvalue push_modifier(modifier);
	pushvalue push_player(player, this);
	for(auto v : source)
		apply_value(v);
}

void creature::cast(const spelli& e) {
	cast(e, get(e), e.mana);
}

void creature::summon(point m, const variants& elements, int count, int level) {
	auto isenemy = is(Enemy);
	auto isally = is(Ally);
	for(auto i = 0; i < count; i++) {
		auto v = randomizeri::random(elements);
		auto p = player_create(m, v, false);
		if(isenemy)
			p->set(Enemy);
		else
			p->remove(Enemy);
		if(isally)
			p->set(Ally);
		else
			p->remove(Ally);
		p->set(Summoned);
		p->setowner(this);
	}
}

bool creature::ispresent() const {
	return worldpos == game;
}

int	creature::getpaymentcost() const {
	auto room = getroom();
	if(!room)
		return 0;
	auto keeper = room->getowner();
	if(!keeper)
		return 0;
	auto skill_delta = keeper->get(Wits) - get(Wits);
	return 200 + skill_delta;
}

int	creature::getsellingcost() const {
	auto room = getroom();
	if(!room)
		return 0;
	auto keeper = room->getowner();
	if(!keeper)
		return 0;
	auto skill_delta = get(Wits) - keeper->get(Wits);
	auto result = 40 + skill_delta;
	if(result < 10)
		result = 10;
	return result;
}

void player_levelup() {
	player->basic.abilities[Level] += 1;
	advance_value(player->getkind(), player->basic.abilities[Level]);
}

creature* player_create(point m, variant kind, bool female) {
	if(!kind)
		return 0;
	pushvalue push_player(player);
	player = bsdata<creature>::addz();
	player->clear();
	player->worldpos = game;
	player->setkind(kind);
	player->setnoname();
	if(female)
		player->set(Female);
	monsteri* pm = kind;
	if(pm) {
		copy(player->basic, *pm);
		advance_value(pm->use);
		adat<variant> conditions;
		conditions.add(kind);
		player->setname(charname::param(conditions));
		player->basic.abilities[LineOfSight] += 4;
	} else {
		adat<variant> conditions;
		conditions.add(kind);
		if(player->is(Female))
			conditions.add("Female");
		player->setname(charname::param(conditions));
		player->basic.abilities[LineOfSight] += 4;
		advance_value(kind, 0);
		player_levelup();
	}
	player->place(m);
	player->finish();
	if(pm) {
		if(pm->friendly <= -10)
			player->set(Enemy);
	}
	if(player->is(PlaceOwner)) {
		auto pr = player->getroom();
		if(pr)
			pr->setowner(player);
	}
	return player;
}

bool creature::isallow(const item& it) const {
	auto& ei = it.geti();
	for(auto i = Strenght; i <= sizeof(ei.required) / sizeof(ei.required[0]); i = (ability_s)(i + 1)) {
		auto v = ei.required[i];
		if(!v)
			continue;
		if(get(i) < v)
			return false;
	}
	return true;
}

void creature::equipi(short unsigned type, int count) {
	item it; it.create(bsdata<itemi>::elements + type, count);
	it.createpower(0);
	if(isallow(it))
		equip(it);
	else
		wearable::additem(it);
}

bool creature::canremove(item& it) const {
	if(it.iscursed()) {
		it.setidentified(true);
		fixaction("ThisIsMyCursedItem", 0, it.getname());
		return false;
	}
	return true;
}

roomi* creature::getroom() const {
	if(!area)
		return 0;
	return area->rooms.ptrs(room_id);
}

void creature::setroom(const roomi* v) {
	room_id = (v == 0) ? 0xFFFF : area->rooms.indexof(v);
}

int	creature::getmaximum(ability_s v) const {
	switch(v) {
	case Hits: case Mana:
		return basic.abilities[v];
	default:
		return 0;
	}
}

bool ispresent(const void* p) {
	return ((creature*)p)->ispresent();
}

int get_maximum_faith(creature* p) {
	return p->get(Religion) / 4;
}