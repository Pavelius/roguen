#include "main.h"

extern creaturea targets;
extern itema items;

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

static void choose_targets(unsigned flags) {
	targets.clear();
	if(FGT(flags, Allies)) {
		if(player->is(Ally))
			add_creatures(Ally);
		if(player->is(Enemy))
			add_creatures(Enemy);
	}
	if(FGT(flags, Enemies)) {
		if(player->is(Ally))
			add_creatures(Enemy);
		if(player->is(Enemy))
			add_creatures(Ally);
	}
	if(FGT(flags, You))
		targets.add(player);
	if(!FGT(flags, FarRange))
		targets.matchrange(player->getposition(), 1, true);
	targets.distinct();
	items.clear();
	if(FGT(flags, Item)) {
		for(auto p : targets)
			items.select(p);
	}
}