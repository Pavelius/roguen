#include "main.h"

creature* player;
creature* opponent;
creature* enemy;

static void copy(statable& v1, const statable& v2) {
	v1 = v2;
}

static creature* findalive(point m) {
	for(auto& e : bsdata<creature>()) {
		if(e.isvalid() && e.getposition() == m)
			return &e;
	}
	return 0;
}

static ability_s damage_ability(wear_s v) {
	switch(v) {
	case RangedWeapon: return DamageRanged;
	default: return DamageMelee;
	}
}

void creature::clear() {
	memset(this, 0, sizeof(*this));
	worldpos = {-1000, -1000};
	setroom(0);
	setowner(0);
}

void creature::levelup() {
	basic.abilities[Level] += 1;
}

static bool isfreecr(point m) {
	if(findalive(m))
		return false;
	auto tile = area[m];
	if(tile == Water || tile == DarkWater || tile == DeepWater)
		return false;
	return area.isfree(m);
}

void creature::place(point m) {
	m = area.getfree(m, 10, isfreecr);
	setposition(m);
	update_room();
}

creature* creature::create(point m, variant kind, variant character) {
	if(!kind)
		return 0;
	if(!character)
		character = "Monster";
	if(!character.iskind<classi>())
		return 0;
	m = area.getfree(m, 10, isfreecr);
	if(!area.isvalid(m))
		return 0;
	auto p = bsdata<creature>::addz();
	p->clear();
	p->setposition(m);
	p->worldpos = game;
	p->setkind(kind);
	p->setnoname();
	p->class_id = character.value;
	monsteri* pm = kind;
	if(pm) {
		copy(p->basic, *pm);
		p->advance(pm->use);
	} else {
		adat<variant> conditions;
		conditions.add(kind);
		if(p->is(Female))
			conditions.add("Female");
		p->setname(charname::random(conditions));
	}
	p->basic.create();
	p->advance(kind, 0);
	if(character.value)
		p->advance(character, 0);
	p->levelup();
	p->finish();
	p->update_room();
	if(pm) {
		if(pm->friendly <= -10)
			p->set(Enemy);
	}
	if(p->is(PlaceOwner)) {
		auto pr = p->getroom();
		if(pr)
			pr->setowner(p);
	}
	return p;
}

bool creature::isplayer() const {
	return game.getowner() == this;
}

bool creature::isenemy(const creature& opponent) const {
	return opponent.is(is(Enemy) ? Ally : Enemy);
}

void creature::movestep(direction_s v) {
	if(area.is(getposition(), Iced)) {
		if(!roll(Dexterity)) {
			act(getnm("IcedSlice"));
			v = round(v, (d100() < 50) ? NorthWest : NorthEast);
			wait();
		}
	}
	setdirection(v);
	movestep(to(getposition(), v));
}

static void drop_item(point m, const char* id) {
	if(!area.isvalid(m) || !id)
		return;
	itemi* pi = single(id);
	if(!pi)
		return;
	item it; it.create(pi);
	it.drop(m);
}

void creature::interaction(point m) {
	if(!area.isvalid(m))
		return;
	auto f = area.features[m];
	if(f) {
		auto& ei = bsdata<featurei>::elements[f];
		if(ei.is(AllowActivate)) {
			if(!area.is(m, Activated))
				area.set(m, Activated);
			else
				area.remove(m, Activated);
		} else if(ei.is(Woods) && wears[MeleeWeapon].geti().is(CutWoods)) {
			auto& wei = wears[MeleeWeapon].geti();
			auto chance = get(Strenght) / 10 + wei.weapon.damage;
			if(chance < 1)
				chance = 1;
			if(wei.is(TwoHanded))
				chance *= 2;
			if(d100() < chance) {
				act(getnm("YouCutWood"), getnm(ei.id));
				area.features[m] = NoFeature;
				drop_item(m, "WoodenLagsTable");
			}
		}
	}
}

bool creature::matchspeech(variant v) const {
	if(v.iskind<monsteri>())
		return iskind(v);
	else if(v.iskind<conditioni>())
		return is((condition_s)v.value);
	return true;
}

bool creature::matchspeech(const variants& source) const {
	for(auto v : source) {
		if(!matchspeech(v))
			return false;
	}
	return true;
}

void creature::matchspeech(speecha& source) const {
	auto ps = source.data;
	for(auto p : source) {
		if(!matchspeech(p->condition))
			continue;
		*ps++ = p;
	}
	source.count = ps - source.data;
}

const speech* creature::matchfirst(const speecha& source) const {
	auto ps = source.data;
	for(auto p : source) {
		if(!matchspeech(p->condition))
			continue;
		return p;
	}
	return 0;
}

const char* creature::getspeech(const char* id) const {
	speecha source;
	source.select(id);
	if(!source)
		return getnm("NothingToSay");
	auto conditional = source[0]->condition.count > 0;
	if(conditional) {
		auto p = matchfirst(source);
		if(!p)
			return getnm("NothingToSay");
		return p->name;
	} else
		return source.getrandom();
}

void creature::interaction(creature& opponent) {
	if(opponent.isenemy(*this))
		attackmelee(opponent);
	else if(!isplayer())
		return;
	else if(opponent.is(PlaceOwner)) {
		fixaction();
		opponent.wait();
		if(d100() < 40)
			opponent.speech("DontPushMePlaceOwner");
		return;
	} else {
		auto pt = opponent.getposition();
		opponent.setposition(getposition());
		opponent.fixmovement();
		opponent.wait();
		setposition(pt);
		fixmovement();
		if(d100() < 40)
			opponent.speech("DontPushMe");
	}
}

int creature::getdamage(wear_s v) const {
	auto result = wears[v].getdamage();
	result += get(damage_ability(v));
	return result;
}

void creature::getdefence(int attacker_strenght, const item& attacker_weapon, defencet& result) const {
	result[0].skill = WeaponSkill;
	result[0].value = getparrying(attacker_weapon, wears[MeleeWeapon], get(WeaponSkill)) - getmightpenalty(attacker_strenght);
	result[1].skill = ShieldUse;
	result[1].value = getblocking(attacker_weapon, wears[MeleeWeaponOffhand], get(ShieldUse)) - getmightpenalty(attacker_strenght);
	result[2].skill = DodgeSkill;
	result[2].value = get(DodgeSkill);
	for(auto& e : result) {
		if(e.value < 0)
			e.value = 0;
	}
}

static ability_s getbestdefence(const defencet& defences, int parry_result, int& parry_skill, int& parry_effect) {
	auto parry = (ability_s)0;
	parry_skill = 0;
	parry_effect = 0;
	for(auto& e : defences) {
		if(parry_result <= e.value) {
			auto v = (e.value - parry_result) / 10;
			if(!parry_skill || parry_effect < v) {
				parry = e.skill;
				parry_skill = e.value;
				parry_effect = v;
			}
		}
	}
	return parry;
}

void creature::damage(const item& weapon, int effect) {
	auto weapon_damage = weapon.getdamage();
	auto damage_reduction = get(DamageReduciton) - weapon.geti().weapon.pierce;
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage - damage_reduction + effect;
	fixdamage(result_damage, weapon_damage, 0, -damage_reduction, effect, 0);
	damage(result_damage);
}

void creature::attack(creature& enemy, wear_s v, int attack_skill, int damage_multiplier) {
	skilli defences[3] = {};
	int parry_skill = 0;
	last_hit = 1 + d100(); last_hit_result = 0;
	last_parry = 1 + d100(); last_parry_result = 0;
	attack_skill += get(wears[v].geti().ability);
	auto attack_delta = attack_skill - last_hit;
	last_hit_result = attack_delta / 10;
	if(last_hit > 5 && last_hit > attack_skill) {
		logs(getnm("AttackMiss"), last_hit, attack_skill);
		return;
	}
	auto enemy_name = enemy.getname();
	auto attacker_name = getname();
	enemy.getdefence(get(Strenght), wears[v], defences);
	auto parry = getbestdefence(defences, last_parry, parry_skill, last_parry_result);
	auto parry_delta = parry_skill - last_parry;
	if(parry) {
		if(parry_delta > attack_delta) {
			logs(getnm("AttackHitButParryCritical"), last_hit, attack_skill, last_parry, parry_skill, enemy.getname(), getnm(bsdata<abilityi>::elements[parry].id));
			if(parry == MeleeWeapon && enemy.wears[MeleeWeapon].is(Retaliate)) {
				auto range = area.getrange(enemy.getposition(), getposition());
				if(range <= 1)
					damage(enemy.wears[MeleeWeapon], last_parry_result - last_hit_result);
			}
			return;
		}
	}
	if(parry)
		logs(getnm("AttackHitButParrySuccess"), last_hit, attack_skill, last_parry, parry_skill, enemy.getname(), getnm(bsdata<abilityi>::elements[parry].id));
	else
		logs(getnm("AttackHitButParryFail"), last_hit, attack_skill, last_parry, enemy.getname(), defences[0].value, defences[1].value, defences[2].value);
	auto ability_damage = get(damage_ability(v));
	auto weapon_damage = wears[v].getdamage();
	auto damage_reduction = enemy.get(DamageReduciton) - wears[v].geti().weapon.pierce;
	if(damage_reduction < 0)
		damage_reduction = 0;
	auto result_damage = weapon_damage + ability_damage - damage_reduction + last_hit_result - last_parry_result;
	enemy.fixdamage(result_damage, weapon_damage, ability_damage, -damage_reduction, last_hit_result, -last_parry_result);
	result_damage = result_damage * damage_multiplier / 100;
	enemy.damage(result_damage);
}

void creature::fixdamage(int total, int damage_weapon, int damage_strenght, int damage_armor, int damage_skill, int damage_parry) const {
	logs(getnm("ApplyDamage"), total, damage_weapon, damage_strenght, damage_armor,
		damage_skill, damage_parry);
}

void creature::damage(int v) {
	if(v <= 0) {
		logs(getnm("ArmorNegateDamage"));
		return;
	}
	fixvalue(-v);
	abilities[Hits] -= v;
	if(abilities[Hits] <= 0) {
		if(d100() < 40)
			area.set(getposition(), Blooded);
		auto player_killed = isplayer();
		logs(getnm("ApplyKill"), v);
		fixeffect("HitVisual");
		fixremove();
		clear();
		if(player_killed)
			game.endgame();
	}
}

void creature::attackmelee(creature& enemy) {
	fixaction();
	auto number_attackers = enemy.get(EnemyAttacks);
	if(number_attackers > 4)
		number_attackers = 4;
	attack(enemy, MeleeWeapon, number_attackers * 10, 100);
	enemy.add(EnemyAttacks, 1);
}

bool creature::canshoot(bool interactive) const {
	if(!wears[RangedWeapon]) {
		if(interactive)
			actp(getnm("YouNeedRangeWeapon"));
		return false;
	}
	auto ammo = wears[RangedWeapon].geti().getammunition();
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
	auto pa = wears[RangedWeapon].geti().getammunition();
	if(pa)
		fixshoot(enemy.getposition(), pa->wear_index);
	else
		fixaction();
	attack(enemy, RangedWeapon, 0, 100);
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
	attack(enemy, MeleeWeapon, 0, 100);
	item it;
	it.create(&wears[MeleeWeapon].geti(), 1);
	it.setcount(1);
	it.drop(enemy.getposition());
	wears[MeleeWeapon].use();
}

void creature::fixcantgo() const {
	fixaction();
	act(getnm("CantGoThisWay"));
}

static bool isfreelt(point m) {
	return area.isfree(m);
}

static direction_s movedirection(point m) {
	if(m.x < 0)
		return West;
	else if(m.y < 0)
		return North;
	else if(m.y >= area.mps)
		return South;
	else
		return East;
}

void creature::lookitems() const {
	items.select(getposition());
	if(isplayer() && items) {
		char temp[4096]; stringbuilder sb(temp);
		auto count = items.getcount();
		auto index = 0;
		sb.add(getnm("ThereIsLying"));
		for(auto p : items) {
			if(index > 0) {
				if(index == count - 1)
					sb.adds(getnm("And"));
				else
					sb.add(",");
			}
			sb.adds("%-1", p->getfullname());
			index++;
		}
		sb.add(".");
		actp(temp);
	}
}

void creature::movestep(point ni) {
	if(!area.isvalid(ni)) {
		if(isplayer()) {
			auto direction = movedirection(ni);
			auto np = to(game.position, direction);
			if(confirm(getnm("LeaveArea"), getnm(bsdata<directioni>::elements[direction].id)))
				game.enter(np, 0, NoFeature, direction);
		}
		wait();
		return;
	}
	auto m = getposition();
	// Place owner can't leave it room
	if(is(PlaceOwner)) {
		auto pr = roomi::find(worldpos, m);
		if(getroom() != pr) {
			wait();
			return;
		}
	}
	auto f = area.getfeature(ni);
	switch(f) {
	case StairsUp:
		if(isplayer()) {
			if(confirm(getnm("MoveStairsUp"))) {
				game.enter(game.position, game.level - 1, StairsDown, Center);
				return;
			}
		}
		break;
	case StairsDown:
		if(isplayer()) {
			if(confirm(getnm("MoveStairsDown"))) {
				game.enter(game.position, game.level + 1, StairsUp, Center);
				return;
			}
		}
		break;
	}
	if(area.is(m, Webbed) && !is(IgnoreWeb)) {
		wait(2);
		if(!roll(Strenght)) {
			act(getnm("WebEntagled"));
			wait();
			fixaction();
			return;
		}
		act(getnm("WebBreak"));
		area.remove(m, Webbed);
	}
	auto opponent = findalive(ni);
	if(opponent)
		interaction(*opponent);
	else if(isfreecr(ni)) {
		setposition(ni);
		fixmovement();
		lookitems();
	} else {
		interaction(ni);
		fixaction();
	}
	update_room();
	wait();
}

void creature::finish() {
	update();
	abilities[Hits] = get(HitsMaximum);
	abilities[Mana] = get(ManaMaximum);
	fixappear();
}

void creature::aimove() {
	// When stay on ground you do some work
	if(d100() < 60) {
		wait();
		return;
	}
	static direction_s allaround[] = {North, South, East, West};
	movestep(maprnd(allaround));
}

void creature::advance(variant kind, int level) {
	for(auto& e : bsdata<advancement>()) {
		if(e.type == kind && e.level == level)
			advance(e.elements);
	}
}

void creature::advance(variants elements) {
	for(auto v : elements)
		advance(v);
}

void creature::advance(variant v) {
	if(v.iskind<abilityi>())
		basic.abilities[v.value] += v.counter;
	else if(v.iskind<itemi>()) {
		item it;
		it.create(bsdata<itemi>::elements + v.value, 1);
		equip(it);
	} else if(v.iskind<feati>()) {
		if(v.counter < 0)
			feats.remove(v.value);
		else
			feats.set(v.value);
	}
}

bool creature::roll(ability_s v, int bonus) const {
	auto value = get(v);
	auto result = d100();
	if(result <= 5)
		return true;
	else if(result >= 95)
		return false;
	return result <= (value + bonus);
}

void adventure_mode();

void creature::lookcreatures() const {
	creatures.select(getposition(), getlos(), isplayer());
	if(is(Ally)) {
		enemies = creatures;
		enemies.match(Enemy, true);
	} else if(is(Enemy)) {
		enemies = creatures;
		enemies.match(Ally, true);
	} else
		enemies.clear();
}

void creature::lookenemies() {
	lookcreatures();
	enemy = 0;
	if(enemies) {
		// Combat situation - need eliminate enemy
		enemies.sort(getposition());
		enemy = enemies[0];
	}
}

void creature::makemove() {
	pushvalue push_player(player);
	player = this;
	// Recoil form action
	if(wait_seconds > 0) {
		wait_seconds -= get(Speed);
		return;
	}
	set(EnemyAttacks, 0);
	update();
	// Sleeped creature don't move
	if(is(Sleep))
		return;
	// Unaware attack or others
	if(is(Unaware))
		remove(Unaware);
	lookenemies();
	if(isplayer()) {
		area.setlos(getposition(), getlos(), isfreelt);
		adventure_mode();
	} else if(enemy) {
		if(canshoot(false))
			attackrange(*enemy);
		else
			moveto(enemy->getposition());
	} else
		aimove();
	// Stun creature may remove this state at end of it turn
	if(is(Stun)) {
		if(roll(Strenght))
			remove(Stun);
	}
}

static int getmultiplier(magic_s v) {
	switch(v) {
	case Cursed: return -2;
	case Blessed: return 2;
	case Artifact: return 4;
	default: return 1;
	}
}

void creature::dress(variant v, int multiplier) {
	if(v.iskind<abilityi>())
		abilities[v.value] += v.counter * multiplier;
	else if(v.iskind<listi>())
		dress(bsdata<listi>::elements[v.value].elements, multiplier);
	else if(v.iskind<feati>()) {
		if(v.counter > 0)
			feats.set(v.value);
		else if(v.counter < 0)
			feats.remove(v.value);
	}
}

void creature::dress(variants source, int multiplier) {
	for(auto v : source)
		dress(v, multiplier);
}

void creature::update_wears() {
	for(auto i = MeleeWeapon; i <= Elbows; i = (wear_s)(i + 1)) {
		if(!wears[i])
			continue;
		auto& ei = wears[i].geti();
		auto magic = wears[i].getmagic();
		feats.add(ei.feats);
		if(wears[i].isidentified()) {
			if(ei.dress)
				dress(ei.dress, getmultiplier(magic));
		}
		if(ei.ability) {
			abilities[ei.ability] += ei.bonus;
			switch(magic) {
			case Cursed: abilities[ei.ability] -= 2; break;
			case Blessed: case Artifact: abilities[ei.ability] += 1; break;
			default: break;
			}
		}
	}
}

void creature::update_room() {
	auto pn = roomi::find(worldpos, getposition());
	if(pn) {
		auto pb = getroom();
		if(pn != pb) {
			auto ps = pn->getsite();
			auto pd = getdescription(ps->id);
			if(pd)
				actp(pd);
		}
		room_id = bsdata<roomi>::source.indexof(pn);
	} else
		room_id = 0xFFFF;
}

void creature::update_boost() {
	variant v = this;
	active_spells.clear();
	for(auto& e : bsdata<boosti>()) {
		if(e.parent==v)
			active_spells.set(e.effect);
	}
}

void creature::update_basic() {
	memcpy(abilities, basic.abilities, Hits * sizeof(abilities[0]));
}

void creature::update_abilities() {
	abilities[DamageMelee] += get(Strenght) / 10;
	abilities[DamageThrown] += get(Strenght) / 10;
	abilities[Speed] += get(Dexterity);
	if(is(Light))
		abilities[LineOfSight] += 3;
}

void creature::update() {
	update_basic();
	update_boost();
	update_wears();
	update_abilities();
}

static void blockcreatures(creature* exclude) {
	for(auto& e : bsdata<creature>()) {
		if(e.worldpos != game)
			continue;
		if(&e == exclude)
			continue;
		area.setblock(e.getposition(), 0xFFFF);
	}
}

void creature::moveto(point ni) {
	area.clearpath();
	area.blocktiles(Water);
	area.blocktiles(DeepWater);
	area.blocktiles(DarkWater);
	area.blockwalls();
	area.blockfeatures();
	blockcreatures(this);
	area.makewave(getposition());
	area.blockzero();
	auto m0 = getposition();
	auto m1 = area.getnext(m0, ni);
	if(!area.isvalid(m1))
		return;
	movestep(area.getdirection(m0, m1));
}

void creature::unlink() {
	boosti::remove(this);
	for(auto& e : bsdata<creature>()) {
		if(e.getowner() == this)
			e.setowner(0);
	}
}

void creature::act(const char* format, ...) const {
	if(game.getowner() == this || area.is(getposition(), Visible))
		actv(console, format, xva_start(format), getname(), is(Female));
}

void creature::actp(const char* format, ...) const {
	if(game.getowner() == this)
		actv(console, format, xva_start(format), getname(), is(Female));
}

void creature::actps(const char* format, ...) const {
	if(game.getowner() == this)
		actv(console, format, xva_start(format), getname(), is(Female), ' ');
}

int creature::getmightpenalty(int enemy_strenght) const {
	auto strenght = get(Strenght);
	if(strenght < enemy_strenght)
		return enemy_strenght - strenght;
	return 0;
}

int creature::getblocking(const item& enemy_weapon, const item& weapon, int value) const {
	if(!weapon || !weapon.is(ShieldUse))
		return 0;
	value += enemy_weapon.geti().weapon.enemy_block;
	if(enemy_weapon.is(RangedWeapon))
		value += weapon.geti().weapon.block_ranged;
	else
		value += weapon.geti().weapon.block;
	return value;
}

int creature::getparrying(const item& enemy_weapon, const item& weapon, int value) const {
	if(!weapon)
		return 0;
	if(enemy_weapon.is(RangedWeapon))
		return 0;
	value += enemy_weapon.geti().weapon.enemy_parry;
	value += weapon.geti().weapon.parry;
	return value;
}

void creature::sayv(stringbuilder& sb, const char* format, const char* format_param, const char* name, bool female) const {
	if(isplayer() || area.is(getposition(), Visible))
		actable::sayv(sb, format, format_param, name, female);
}

bool creature::is(condition_s v) const {
	switch(v) {
	case Busy: return wait_seconds > 1000;
	case NPC: return ischaracter();
	case Random: return d100() < 40;
	default: return true;
	}
}

int	creature::getlos() const {
	auto r = get(LineOfSight) - loc.darkness;
	auto m = 1;
	if(r < 1)
		r = 1;
	if(is(Darkvision)) {
		if(r < 2)
			r = 2;
	}
	if(r > 4)
		r = 4;
	return r;
}

bool creature::isvalid() const {
	return worldpos == game;
}

bool creature::speechrumor() const {
	geomarka source;
	source.select(geomark::notknown);
	auto p = source.random();
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	p->getrumor(sb);
	say(temp);
	return true;
}

bool creature::speechlocation() const {
	collection<roomi> source;
	source.select(roomi::notknown);
	auto p = source.random();
	if(!p)
		return false;
	char temp[1024]; stringbuilder sb(temp);
	p->getrumor(sb);
	say(temp);
	return true;
}

void creature::apply(variant v) {
	if(v.iskind<spelli>())
		apply((spell_s)v.value, v.counter, true);
}

void creature::apply(spell_s v, unsigned minutes) {
	auto p = boosti::add(this, v);
	auto n = game.getminutes() + minutes;
	if(p->stamp < n)
		p->stamp = n;
}

void creature::use(variants source) {
	for(auto v : source)
		apply(v);
}

void creature::use(item& v) {
	if(!v)
		return;
	auto& ei = v.geti();
	if(!ei.use)
		return;
	use(ei.use);
	act(getnm("YouUseItem"), v.getname());
	v.use();
	wait();
}