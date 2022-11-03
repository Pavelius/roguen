#include "main.h"

extern creaturea targets;
extern itema items;

static void add_creatures(feat_s v) {
	for(auto p : creatures) {
		if(p->is(v))
			targets.add(p);
	}
}

void choose_targets(unsigned flags) {
	targets.clear();
	if(FGT(flags, Allies)) {
		if(player->is(Ally))
			add_creatures(Ally);
		if(player->is(Enemy))
			add_creatures(Enemy);
	}
	if(FGT(flags, Enemies)) {
		for(auto p : enemies)
			targets.add(p);
	}
	if((flags & (FG(Allies) & FG(Enemies))) == 0)
		targets = creatures;
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