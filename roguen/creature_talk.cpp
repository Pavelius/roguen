#include "main.h"

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
	answers an;
	pushvalue push_an(answers::last, &an);
	for(auto pa = p->nextanswer(); pa; pa = pa->nextanswer()) {
		if(!pa->text)
			continue;
		if(pa->text[0] == '@') {
			auto ps = bsdata<script>::find(pa->text + 1);
			if(ps)
				ps->run(0);
		} else
			an.add(pa, pa->text);
	}
	if(!an)
		an.add(0, getnm("Continue"));
	auto pv = an.choose();
	if(bsdata<phrasei>::have(pv))
		return (phrasei*)pv;
	return 0;
}

static const phrasei* apply_answer(const phrasei* p) {
	if(!p)
		return 0;
	runscript(p->elements);
	auto pt = talki::owner(p);
	return pt->find(p->next);
}

static void talk_entry(const phrasei* p) {
	while(p) {
		console.clear();
		runscript(p->elements);
		opponent->say(p->text);
		p = ask_answer(p);
		p = apply_answer(p);
	}
}

bool creature::talk(const char* id) {
	auto current_talk = bsdata<talki>::find(id);
	if(!current_talk)
		return false;
	talk_entry(current_talk->find(1));
	return true;
}