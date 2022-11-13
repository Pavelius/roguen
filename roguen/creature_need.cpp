#include "greatneed.h"
#include "main.h"

extern greatneed* last_need;
static collection<greatneedi> needs;
extern int last_coins;

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
		if(v.iskind<itemi>() && v.value == type) {
			if(!v.counter)
				return 1;
			return v.counter;
		}
	}
	return 0;
}

static int getrestcount(int score, int maximum) {
	auto n = maximum * score / 100;
	return maximum - n;
}

static int getprogress(int count, int maximum) {
	return count * 100 / maximum;
}

static void apply_answer(void* pv) {
	if(!last_need)
		return;
	if(player->iswear(pv)) {
		auto pi = (item*)pv;
		auto count = pi->getcount();
		auto need_count = getneedcount(*pi, last_need->geti().need);
		auto rest_count = getrestcount(last_need->score, need_count);
		if(count > rest_count)
			count = rest_count;
		if(count > 0) {
			auto last_percent = getprogress(count, need_count);
			last_need->score += last_percent;
			player->logs(getnm("YouGiveItemTo"), pi->getname(), opponent->getname(), count);
			pi->setcount(pi->getcount() - count);
			last_value = 10;
			auto total_coins = last_need->geti().coins;
			if(total_coins) {
				last_coins = last_percent * total_coins / 100;
				player->addcoins(last_coins);
				last_value = 11;
			}
		}
	}
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
	return talk("NeedTalk", apply_answer);
}