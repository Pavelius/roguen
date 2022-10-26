#include "main.h"

BSDATA(targeti) = {
	{"You", true},
	{"YouOrAlly", true, false, true},
	{"AllyClose", false, false, true, 1},
	{"EnemyOrAllyClose", false, true, true, 1},
	{"EnemyOrAllyNear", false, true, true},
	{"EnemyClose", false, true, false, 1},
	{"EnemyNear", false, true, false},
};
assert_enum(targeti, EnemyNear)

extern creaturea targets;

static void keep_enemies() {
	if(player->is(Ally))
		targets.match(Enemy, true);
	else if(player->is(Enemy))
		targets.match(Ally, true);
}

static void keep_allies() {
	if(player->is(Ally))
		targets.match(Ally, true);
	else if(player->is(Enemy))
		targets.match(Enemy, true);
}

static void add_creatures(feat_s v) {
	for(auto p : creatures) {
		if(p->is(v))
			targets.add(p);
	}
}

static void choose_targets(target_s target) {
	auto& ei = bsdata<targeti>::elements[target];
	targets.clear();
	if(ei.allies) {
		if(player->is(Ally))
			add_creatures(Ally);
		if(player->is(Enemy))
			add_creatures(Enemy);
	}
	if(ei.enemies) {
		if(player->is(Ally))
			add_creatures(Enemy);
		if(player->is(Enemy))
			add_creatures(Ally);
	}
	if(ei.you)
		targets.add(player);
	if(ei.range < 1000)
		targets.matchrange(player->getposition(), ei.range, true);
	targets.distinct();
}