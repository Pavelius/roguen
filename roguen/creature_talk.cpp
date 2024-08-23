#include "creature.h"
#include "pushvalue.h"
#include "script.h"
#include "talk.h"

static fncommand talk_proc;

static int maximum_count(const variants& source) {
	auto result = 0;
	for(auto v : source) {
		if(v.counter > result)
			result = v.counter;
	}
	return result;
}

static void remove_item(creature* p, const itemi& ei, int count) {
	for(auto i = Backpack; i <= BackpackLast; i = (wear_s)(i + 1)) {
		auto& e = p->wears[i];
		if(!e)
			continue;
		if(e.is(ei)) {
			auto allow_count = e.getcount();
			if(count < allow_count) {
				e.setcount(allow_count - count);
				count = 0;
			} else {
				e.setcount(0);
				count -= allow_count;
			}
			if(count <= 0)
				break;
		}
	}
}

static const phrasei* ask_answer(const phrasei* p) {
	an.clear();
	for(auto pa = p->nextanswer(); pa; pa = pa->nextanswer()) {
		if(!pa->text)
			continue;
		if(pa->text[0] == '@') {
			auto ps = bsdata<script>::find(pa->text + 1);
			if(ps)
				ps->proc(0);
		} else
			an.add(pa, pa->text);
	}
	if(!an)
		an.add(0, getnm("Continue"));
	return (phrasei*)an.choose();
}

static const phrasei* apply_answer(const phrasei* p, const talki* owner) {
	if(!p)
		return 0;
	if(bsdata<phrasei>::have(p)) {
		script_run(p->elements);
		return owner->find(p->next);
	}
	if(talk_proc) {
		last_value = 0;
		talk_proc((void*)p);
		if(last_value)
			return owner->find(last_value);
	}
	return 0;
}

static void talk_entry(const phrasei* p) {
	while(p) {
		console.clear();
		script_run(p->elements);
		opponent->say(p->text);
		auto owner = find_talk(p);
		p = ask_answer(p);
		p = apply_answer(p, owner);
	}
}

bool creature::talk(const char* id, fncommand proc) {
	auto current_talk = bsdata<talki>::find(id);
	if(!current_talk)
		return false;
	pushvalue push_proc(talk_proc, proc);
	talk_entry(current_talk->find(1));
	return true;
}