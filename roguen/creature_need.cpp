#include "areapiece.h"
#include "creature.h"
#include "functor.h"
#include "modifier.h"
#include "game.h"
#include "greatneed.h"
#include "pushvalue.h"
#include "rand.h"
#include "script.h"
#include "speech.h"
#include "talk.h"
#include "textscript.h"

extern greatneed* last_need;
static collection<greatneedi> needs;
extern int last_coins;

static bool isneed(const void* object) {
	auto p = (creature*)object;
	return find_need(p) != 0;
}

static creature* random_target(const greatneedi* p) {
	collection<creature> source;
	source.select(fntis<creature, &creature::ispresent>);
	source.match(fntis<actable, &actable::isnamed>, true);
	source.match(isneed, false);
	source.match(fntis<creature, &creature::ishuman>, false);
	return source.random();
}

void add_need(int bonus) {
	auto p = needs.pick();
	auto pc = random_target(p);
	auto days = p->days;
	if(!days) {
		days.min = 4;
		days.max = 8;
	}
	days.correct();
	add_greatneed(p, pc, game.getminutes() + xrand(24 * 60 * days.min, 24 * 60 * days.max));
}

static void add_need(variant owner) {
	auto p = needs.pick();
	add_greatneed(p, owner, game.getminutes() + xrand(24 * 60 * 5, 24 * 60 * 8));
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

void update_need() {
	auto stamp = game.getminutes();
	for(auto& e : bsdata<greatneed>()) {
		if(e.deadline > stamp)
			continue;
		auto& ei = e.geti();
		if(e.score < 40)
			e.set(NeedFail);
		else if(e.score >= 100)
			e.set(NeedSuccess);
		e.set(NeedFinished);
		// Add new need
		needs.add(const_cast<greatneedi*>(&ei));
		add_need(e.owner);
	}
}

static int getrestcount(int score, int maximum) {
	auto n = maximum * score / 100;
	return maximum - n;
}

static int getprogress(int count, int maximum) {
	return count * 100 / maximum;
}

static void say_thank_you(const char* item_name, int count, int coins) {
	char temp[512]; stringbuilder sb(temp);
	actvf(sb, 0, speech_get("ThankYouForService"), item_name, count);
	if(coins)
		actvf(sb, ' ', speech_get("HereYourMoney"), coins);
	opponent->say(temp);
	pause();
}

static void say_need(const char* suffix, ...) {
	if(!opponent->ishuman() && !opponent->is(Visible))
		return;
	pushvalue push(player, opponent);
	sayv(console, getnm(str("%1%2", last_need->geti().getid(), suffix)), xva_start(suffix));
	pause();
}

bool payment(creature* player, creature* keeper, const char* object, int coins);

static void talk_apply_answer(void* pv) {
	if(player->iswear(pv)) {
		auto pi = (item*)pv;
		auto count = pi->getcount();
		if(last_need) {
			auto need_count = getneedcount(*pi, last_need->geti().need);
			auto rest_count = getrestcount(last_need->score, need_count);
			if(count > rest_count)
				count = rest_count;
			if(count > 0) {
				auto last_percent = getprogress(count, need_count);
				last_need->score += last_percent;
				if(last_need->score >= 100)
					last_need->set(NeedCompleted);
				const char* item_name = pi->getname();
				player->logs(getnm("YouGiveItemTo"), item_name, opponent->getname(), count);
				pi->setcount(pi->getcount() - count);
				last_coins = last_need->geti().coins * last_percent / 100;
				player->addcoins(last_coins);
				say_thank_you(item_name, count, last_coins);
				if(last_need->is(NeedCompleted))
					say_need("Completed");
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
		auto count = e.getcount();
		auto rest_count = getrestcount(last_need->score, need_count);
		if(count > rest_count)
			count = rest_count;
		an.add(&e, getnm("IHaveItem"), e.getname(), count);
	}
}

void prepare_need() {
	needs.select();
	needs.shuffle();
}

bool speech_need() {
	pushvalue push_need(last_need);
	last_need = find_need(opponent, NeedFinished);
	if(last_need) {
		if(last_need->is(NeedSuccess)) {
			say_need("Success");
			script_run(last_need->geti().success);
		} else if(last_need->is(NeedFail)) {
			say_need("Fail");
			script_run(last_need->geti().fail);
		} else
			say_need("Partial");
		last_need->clear();
		shrink_greatneed();
		return true;
	}
	last_need = find_need(opponent, NeedCompleted);
	if(last_need) {
		if(d100() < 30)
			return false;
		opponent->speak("VisitMeLater");
		return true;
	}
	last_need = find_need(opponent);
	if(!last_need)
		return false;
	return talk_opponent("NeedTalk", talk_apply_answer);
}