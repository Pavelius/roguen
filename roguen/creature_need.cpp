#include "greatneed.h"
#include "main.h"

extern greatneed* last_need;
static collection<greatneedi> needs;

static creature* random_target(const greatneedi* p) {
	collection<creature> source;
	source.select(creature::ispresent);
	source.match(creature::isnamed, true);
	source.match(creature::isneed, false);
	source.match(creature::ishuman, false);
	return source.random();
}

void add_need(int bonus) {
	auto p = needs.pick();
	auto pc = random_target(p);
	greatneed::add(p, pc, game.getminutes() + xrand(24 * 60 * 5, 24 * 60 * 8));
}

static int getneedcount(const item& e, const variants& source) {
	if(!e)
		return 0;
	auto type = e.getkind();
	for(auto v : source) {
		if(v.iskind<itemi>() && v.value == type)
			return v.counter;
	}
	return 0;
}

void add_need_answers(int bonus) {
	if(!last_need)
		return;
	for(auto& e : player->backpack()) {
		auto need_count = getneedcount(e, last_need->geti().need);
		if(!need_count)
			continue;
		answers::last->add(&e, getnm("IHaveItem"), e.getname());
	}
}

void prepare_need() {
	needs.select();
	needs.shuffle();
}

bool creature::isneed(const void* object) {
	auto p = (creature*)object;
	return greatneed::find(p) != 0;
}

bool creature::speechneed() {
	pushvalue push_need(last_need, greatneed::find(this));
	if(!last_need)
		return false;
	return talk("NeedTalk");
}